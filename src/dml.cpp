#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "base64.h"
#include "file_parser.h"
#include "options.h"
#include "dml.h"


#include <unistd.h>

DML::DML(Options &opt, const std::string &filepath)
    : m_filepath(filepath), m_cacheFilepath(getCachePath(opt.cacheDir, filepath))
{
    if (!opt.ignoreCache && std::filesystem::exists(m_cacheFilepath))
        loadCacheFiles();
    else 
    {
        FileParser parser(m_filepath);
        parser.parseFile(m_indexTree, m_valueBuffer);

        saveCacheFile();
    }
}

const std::vector<char>& DML::getKeyBuffer() 
{
    return m_indexTree.getKeyBuffer();
}

const char* DML::search(const std::string &key) 
{
    int index = m_indexTree.search(key);

    return &m_valueBuffer[index];
}


std::string DML::getCachePath(const std::string& cacheDir, std::string filepath)
{
    std::filesystem::path absolutePath = std::filesystem::absolute(m_filepath);
    std::string absolutePathString = absolutePath.string();

    unsigned char m_filePathHash[EVP_MAX_MD_SIZE];

    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    const EVP_MD* md = EVP_sha1();
    EVP_DigestInit_ex(mdctx, md, nullptr);
    EVP_DigestUpdate(mdctx, absolutePathString.c_str(), absolutePathString.length());
    EVP_DigestFinal_ex(mdctx, m_filePathHash, nullptr);
    EVP_MD_CTX_free(mdctx);

    std::string hashBase64 = base64_encode(m_filePathHash, EVP_MAX_MD_SIZE);

    return cacheDir + hashBase64.erase(hashBase64.length() - 2);
}

void DML::saveCacheFile()
{
    std::ofstream outFile(m_cacheFilepath, std::ios::binary);
    if (!outFile)
    {
        std::cerr << "Failed to create the cache file." << std::endl;
        return;
    }

    m_indexTree.saveTreeToFile(outFile);


    size_t valueBufferSize = m_valueBuffer.size();
    outFile.write(reinterpret_cast<const char*>(&valueBufferSize), sizeof(valueBufferSize));
    outFile.write(reinterpret_cast<const char*>(&m_valueBuffer[0]), sizeof(char) * valueBufferSize);
}

void DML::loadCacheFiles() {
    std::ifstream file(m_cacheFilepath, std::ios::binary);
    if (file.is_open()) {
        m_indexTree.loadTreeFromFile(file);

        size_t valuesBufferSize;
        file.read(reinterpret_cast<char*>(&valuesBufferSize), sizeof(valuesBufferSize));
        m_valueBuffer.resize(valuesBufferSize);
        file.read(reinterpret_cast<char*>(&m_valueBuffer[0]), valuesBufferSize);
    } else {
        std::cout << "Unable to the cache open file." << std::endl;
    }
}

std::string DML::browse(char *browseProgram) 
{
    std::string result;
    // struct popen2 child;
    int read_pipe[2], write_pipe[2];

    if(pipe(read_pipe) || pipe(write_pipe)) {
        printf("ERROR: Failed to open pipe!");
        exit(1);
    }

    pid_t p = fork();
    if(p < 0) {
        printf("ERROR: Failed to fork process!");
        exit(1);
    }
    if(p == 0) { /* child */
        char cmd[1024];

        close(read_pipe[1]); // Close the write end of the pipe
        close(write_pipe[0]); // Close the read end of the path

        char *ch = browseProgram, *dst = cmd;
        int input_file = 0;
        int output_file = 0;
        while (*ch) {
            if (*ch == '%') {
                switch (*(++ch)) {
                case 'i':
                    dst += sprintf(dst, "/proc/self/fd/%u", read_pipe[0]);
                    input_file = 1;
                break;
                case 'o':
                    dst += sprintf(dst, "/proc/self/fd/%u", write_pipe[1]);
                    output_file = 1;
                break;
                default:
                    fprintf(stderr, "ERROR: Unknown identifier '%%%c'!\n", *ch);
                    exit(1);
                };
    
                ++ch;
                continue;
            }
            *(dst++) = *(ch++);
        }
        *dst = 0;
  
        if (!input_file) 
            dup2(read_pipe[0], 0); // Replace stdin with the read end of the pipe
        if (!output_file) 
            dup2(write_pipe[1], 1); // Replace stdout with the write end of the pipe

        execl("/bin/sh", "sh", "-c", cmd, NULL);
        perror("execl"); exit(99);
    } 
    
    // Parent process
    close(read_pipe[0]); // Close read end of pipe
    close(write_pipe[1]);  // Close write end of pipe
    
    FILE *to_child_fd = fdopen(read_pipe[1], "w");
    
    const std::vector<char>& keyBuffer = getKeyBuffer();
    fprintf(to_child_fd, "%.*s", (int) keyBuffer.size(), &keyBuffer[0]);
    fflush(to_child_fd);
    close(read_pipe[1]);


    char buff[1024];
    size_t len = read(write_pipe[0], buff, 1024);

    if (len) {
        buff[len-1] = 0;
        return search(buff);
    }

    return "";
}
