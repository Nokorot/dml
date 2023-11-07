#include <bits/types/FILE.h>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "base64.h"
#include "file_parser.h"
#include "options.h"
#include "dml.h"

#include "shell_command.h"

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


std::string getCachePath(const std::string& cacheDir, std::string filepath)
{
    std::filesystem::path absolutePath = std::filesystem::absolute(filepath);
    std::string absolutePathString = absolutePath.string();

    unsigned char m_filePathHash[EVP_MAX_MD_SIZE];

    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    const EVP_MD* md = EVP_sha1();
    EVP_DigestInit_ex(mdctx, md, nullptr);
    EVP_DigestUpdate(mdctx, absolutePathString.c_str(), absolutePathString.length());
    EVP_DigestFinal_ex(mdctx, m_filePathHash, nullptr);
    EVP_MD_CTX_free(mdctx);

    std::string hashBase64 = base64_encode(m_filePathHash, EVP_MAX_MD_SIZE);

    return cacheDir + "/" + hashBase64.erase(20);
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

    int flags = OPEN_TO_CHILD_PIPE | OPEN_FROM_CHILD_PIPE | USE_IO_ARGUMENT_FLAGS;
    ShellCommand shcmd(browseProgram, flags);
    shcmd.execute();

    int to_child = shcmd.getToChildPID();
    FILE *to_child_fd = fdopen(to_child, "w");

    const std::vector<char>& keyBuffer = getKeyBuffer();
    fprintf(to_child_fd, "%.*s", (int) keyBuffer.size(), &keyBuffer[0]);

    fclose(to_child_fd);
    // close(to_child);

    int status = shcmd.waitforChildExit();
    // printf("Exit Status: %u\n", status);

    std::string output = shcmd.readOutput();

    // Remove trailing newline character
    if (!output.empty() && output.back() == '\n')
        output.pop_back();

    // printf("Output: %s\n", output.c_str());

    if (output.length() != 0) {
        // printf("Result: %s\n", search(output.c_str()));
        return search(output.c_str());
    }

    return "";
}
