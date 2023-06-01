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
    const std::string m_inputFile, m_cacheFilename;

    std::string getCachePath(const std::string& cacheDir, std::string filepath);

    // Helper function for loadFromFile
    void loadLine(std::string trimmedLine, int &keyOffset, int &valueOffset);
    void loadFromFile(const std::string& inputFile, int &keyOffset, int &valueOffset);

    std::string trim(const std::string& str);

private: 
    // Temporary data structures.  
    //  They should probably not be here, but rather only exist when needed
    std::unordered_map<std::string, std::streampos> m_keyIndexMap;

    std::vector<std::string> m_keys;
    std::vector<std::string> m_values;

};

