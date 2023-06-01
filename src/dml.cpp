#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "base64.h"
#include "file_parser.h"
#include "options.h"
#include "dml.h"

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

    std::vector<char>& keyBuffer = m_indexTree.getKeyBuffer();

    size_t keyBufferSize = keyBuffer.size();
    outFile.write(reinterpret_cast<const char*>(&keyBufferSize), sizeof(keyBufferSize));
    outFile.write(reinterpret_cast<const char*>(&keyBuffer[0]), sizeof(char) * keyBufferSize);

    size_t valueBufferSize = m_valueBuffer.size();
    outFile.write(reinterpret_cast<const char*>(&valueBufferSize), sizeof(valueBufferSize));
    outFile.write(reinterpret_cast<const char*>(&m_valueBuffer[0]), sizeof(char) * valueBufferSize);

    m_indexTree.saveTreeToFile(outFile);
}

void DML::loadCacheFiles() {
    std::ifstream file(m_cacheFilepath, std::ios::binary);
    if (file.is_open()) {
        std::vector<char>& keyBuffer = m_indexTree.getKeyBuffer();

        size_t keysBufferSize;
        file.read(reinterpret_cast<char*>(&keysBufferSize), sizeof(keysBufferSize));
        keyBuffer.resize(keysBufferSize);
        file.read(reinterpret_cast<char*>(&keyBuffer[0]), keysBufferSize);

        size_t valuesBufferSize;
        file.read(reinterpret_cast<char*>(&valuesBufferSize), sizeof(valuesBufferSize));
        m_valueBuffer.resize(valuesBufferSize);
        file.read(reinterpret_cast<char*>(&m_valueBuffer[0]), valuesBufferSize);

        m_indexTree.loadTreeFromFile(file);
        // m_indexTree = new RedBlackTree(m_keysBuffer, file);
    } else {
        std::cout << "Unable to the cache open file." << std::endl;
    }
}

