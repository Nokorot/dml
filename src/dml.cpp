#include <bits/types/FILE.h>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>

#include "base64.h"
#include "file_parser.h"
#include "options.h"
#include "dml.h"

#include "shell_command.h"

DML::DML(Options &opt, const std::string &filepath)
    : m_opt(opt), m_filepath(filepath), 
    m_cacheFilepath(getCachePath(opt.cacheDir, filepath))
{
    if (std::filesystem::exists(m_cacheFilepath + ".hist"))
        loadHistory();

    if (!opt.ignoreCache && std::filesystem::exists(m_cacheFilepath + ".dat"))
        loadCacheFiles();
    else
    {
        FileParser parser(m_filepath);
        parser.parseFile(m_indexTree, m_valueBuffer);

        saveCacheFile();
    }
}

DML::~DML()
{
    saveHistory();
}

const std::vector<char>& DML::getKeyBuffer()
{
    return m_indexTree.getKeyBuffer();
}

const char* DML::search(const std::string key)
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
    std::ofstream outFile(m_cacheFilepath + ".dat", std::ios::binary);
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

void DML::saveHistory()
{
    if (!m_opt.historyLength)
        return;

    std::unordered_set<std::string> saved;
    std::ofstream outFile(m_cacheFilepath + ".hist", std::ios::binary);
    if (!outFile)
    {
        std::cerr << "Failed to create the history file." << std::endl;
        return;
    }

    int count = 0;
    for (auto key=m_history.begin(); key != m_history.end(); ++key) {
        // Skip duplicate lines.
        if (saved.find(*key) != saved.end())
            continue;

        if (count++ == m_opt.historyLength)
            break;

        saved.insert(*key);
        outFile << *key << '\n';
    }

    outFile.close();
}

void DML::loadHistory()
{
    std::ifstream inputFile(m_cacheFilepath + ".hist");

    // Check if the file is open
    if (!inputFile.is_open()) {
        std::cerr << "Failed to open the history file." << std::endl;
        return;
    }

    std::string line;
    while (std::getline(inputFile, line))
        m_history.push_back(line);

    // Close the file
    inputFile.close();
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

void DML::listKeys(FILE *sink)
{
    const std::vector<char>& keyBuffer = getKeyBuffer();

    if (!m_opt.recent) {
        fprintf(sink, "%.*s", (int) keyBuffer.size(), &keyBuffer[0]);
        return;
    }

    std::unordered_set<std::string> printed;
    for (auto it = m_history.begin(); it != m_history.end(); ++it) {
        if (printed.find(*it) != printed.end())
            continue;

        if (m_indexTree.search(it->c_str()) < 0)
            continue;

        fprintf(sink, "%s\n", it->c_str());

        printed.insert(*it);
    }

    // const char *ch_ptr = &keyBuffer[0];

    // printf("B len %s\n", &keyBuffer[0]);

    // const char* key = std::strchr(&keyBuffer[0], '\n');

    std::istringstream keystream(&keyBuffer[0]);
    std::string key;
    while (std::getline(keystream, key))
    {
        if (printed.find(key) != printed.end())
            continue;

        // fprintf(sink, "%.*s", (int) keyBuffer.size(), &keyBuffer[0]);
        fprintf(sink, "%s\n", key.c_str());
        printed.insert(key);
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

    listKeys(to_child_fd);
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
        if (m_opt.recent)
            pushRecent(output);

        // printf("Result: %s\n", search(output.c_str()));
        return search(output.c_str());
    }

    return "";
}

void DML::pushRecent(const std::string key) 
{
    m_history.insert(m_history.begin(), key);
}
