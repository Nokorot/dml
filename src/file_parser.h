#pragma once

#include <unordered_map>
#include <vector>
#include <openssl/evp.h>
#include <string>

#include "options.h"
#include "red_black_tree.h"

class FileParser
{
public:
    FileParser(const std::string &inputFile);

    void parseFile(RedBlackTree &indexTree, std::vector<char> &valueBuffer);

private: 
    bool skipingToEndif = false;

    const std::string m_inputFile, m_cacheFilename;

    std::string getCachePath(const std::string& cacheDir, std::string filepath);

    // Helper function for parseFile
    void loadLine(std::string trimmedLine, RedBlackTree &indexTree, std::vector<char> &valueBuffer);
    void loadFromFile(const std::string& inputFile, RedBlackTree &indexTree, std::vector<char> &valueBuffer); 
    void loadFromBuffer(const char *buffer, RedBlackTree &indexTree, std::vector<char> &valueBuffer);
};

