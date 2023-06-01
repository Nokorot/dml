#include <cstddef>
#include <cstring>
#include <iostream>
#include <fstream>
#include <openssl/evp.h>
#include <filesystem>
#include <string>
#include <vector>

#include "base64.h"

#include "file_parser.h"

FileParser::FileParser(const std::string &inputFile)
    : m_inputFile(inputFile)
{}

void FileParser::parseFile(RedBlackTree &indexTree, std::vector<char> &valueBuffer) 
{ 
    int keyOffset = 0;
    int valueOffset = 0;
     
    loadFromFile(m_inputFile, keyOffset, valueOffset); 

    std::vector<char>& keyBuffer = indexTree.getKeyBuffer();
    keyBuffer.resize(keyOffset);

    keyOffset = 0;
    for (size_t i=0; i < m_keys.size(); ++i) { 
        // \n terminated
        memcpy(&keyBuffer[keyOffset], m_keys[i].c_str(), m_keys[i].size());
        keyBuffer[keyOffset + m_keys[i].size()] = '\n';
        
        // NOTE: This is the only place m_keyIndexMap is used, and as it is it is not necessary.
        // Since there are the same number of keys and values. So we can just use the index.
        // It may be useful, if we remove the empty vales for example.
        indexTree.insert(keyOffset, m_keys[i].size(), m_keyIndexMap[m_keys[i]]);

        keyOffset += m_keys[i].size() + 1;
    }

    valueBuffer.resize(valueOffset);
    valueOffset = 0;
    for (size_t i=0; i < m_values.size(); ++i) { 
        // \0 terminated
        memcpy(&valueBuffer[valueOffset], m_values[i].c_str(), m_values[i].size() + 1);
        valueOffset += m_values[i].size() + 1;
    }
}

void FileParser::loadLine(std::string trimmedLine, int &keyOffset, int &valueOffset) {
    std::istringstream iss(trimmedLine);
    std::string key, value;
    iss >> key;
                
    std::getline(iss >> std::ws, value);  // Extract the entire rest of the line as the value
 
    m_keys.push_back(key);
    keyOffset += key.size() + 1;

    m_keyIndexMap[key] = valueOffset; // Initialize index as 0
    m_values.push_back(value);

    valueOffset += value.size() + 1;
}

void FileParser::loadFromFile(const std::string& filepath, int &keyOffset, int &valueOffset)
{
    std::ifstream inFile(filepath);
    if (!inFile)
    {
        std::cerr << "Failed to open the input file." << std::endl;
        return;
    }

    std::string line;
    std::string pendingLine; // Store the pending line continuation
    while (std::getline(inFile, line))
    {
        std::string trimmedLine = trim(line);
    
        // Check if the line continuation is pending
        if (!pendingLine.empty())
        {
            trimmedLine = pendingLine + trimmedLine;
            pendingLine = "";
        }

        // Check for the '%include' directive
        if (trimmedLine.compare(0, 8, "%include") == 0)
        {
            std::istringstream iss(trimmedLine);
            std::string directive, includeFile;
            iss >> directive >> includeFile;

            // Remove the '%include' keyword and leading white-space
            includeFile = includeFile.substr(includeFile.find_first_not_of(" \t"));
            // Load the included file recursively
            loadFromFile(includeFile, keyOffset, valueOffset);
        } 
        else  
        {
            if (trimmedLine.empty() || trimmedLine[0] == '#')
                continue;

            size_t commentPos = trimmedLine.find('#');
            if (commentPos != std::string::npos)
            {
                // Check if the '#' is escaped
                if (commentPos > 0 && trimmedLine[commentPos - 1] == '\\')
                    trimmedLine.erase(commentPos - 1, 1);
                else
                    trimmedLine.erase(commentPos);
            }
            
            // Check if the line ends with a backslash
            if (trimmedLine.back() == '\\')
            {
                trimmedLine.pop_back();
                pendingLine = trimmedLine;
                continue;
            }

            loadLine(trimmedLine, keyOffset, valueOffset);
        }
    }

    if (!pendingLine.empty())
        loadLine(pendingLine, keyOffset, valueOffset);

}


std::string FileParser::trim(const std::string& str)
{
    const auto first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos)
        return "";

    const auto last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, (last - first + 1));
}
