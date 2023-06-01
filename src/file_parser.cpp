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
    loadFromFile(m_inputFile, indexTree, valueBuffer); 
}

void FileParser::loadLine(std::string trimmedLine, RedBlackTree &indexTree, std::vector<char> &valueBuffer) {

    std::istringstream iss(trimmedLine);
    std::string directive;
    iss >> directive;

    // TODO IF first character is %, hashTable

    // Check for the '%include' directive
    if (directive.compare("%include") == 0)
    {
        std::string includeFile;
        iss >> includeFile;

        // TODO: Error if there is more on the line?
  
        // Load the included file recursively
        loadFromFile(includeFile, indexTree, valueBuffer);
    }
    else if (directive.compare("%check") == 0)
    {
        std::string program;
        iss >> program;

        if (isProgramInPath(program))
        {
            std::string line;
            std::getline(iss >> std::ws, line);
            loadLine(program + " " + line, indexTree, valueBuffer);
        }
    }
    else if (directive.compare("%check*") == 0)
    {
        std::string program;
        iss >> program;

        if (isProgramInPath(program))
        {
            std::string line;
            std::getline(iss >> std::ws, line);
            loadLine(line, indexTree, valueBuffer);
        }
    }
    else  
    {
        std::string key = directive;
        std::string value;
                    
        std::getline(iss >> std::ws, value);  // Extract the entire rest of the line as the value
        int valueOffset = valueBuffer.size();
        {
            valueBuffer.resize(valueOffset + value.size() + 1);

            memcpy(&valueBuffer[valueOffset], value.c_str(), value.size());
            valueBuffer[valueOffset + value.size()] = '\0';
      
            indexTree.insert(key, valueOffset);
        }
    }


}

void FileParser::loadFromFile(const std::string& filepath, RedBlackTree &indexTree, std::vector<char> &valueBuffer)
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
        // Check if the line continuation is pending
        if (!pendingLine.empty())
        {
            line = pendingLine + line;
            pendingLine = "";
        }

        // Check if the line ends with a backslash
        if (line.back() == '\\')
        {
            line.pop_back();
            pendingLine = line;
            continue;
        }

        // NOTE: This comes after the backslash check
        std::string trimmedLine = trim(line);

        if (trimmedLine.empty() || trimmedLine[0] == '#')
            continue;

        // Handle comments
        size_t commentPos = trimmedLine.find('#');
        if (commentPos != std::string::npos)
        {
            // Check if the '#' is escaped
            if (commentPos > 0 && trimmedLine[commentPos - 1] == '\\')
                trimmedLine.erase(commentPos - 1, 1);
            else
                trimmedLine.erase(commentPos);
        }

        loadLine(trimmedLine, indexTree, valueBuffer);
    }

    if (!pendingLine.empty())
        loadLine(pendingLine, indexTree, valueBuffer);
}

bool FileParser::isProgramInPath(const std::string& programName) {
    std::string command = "which " + programName + " > /dev/null 2>&1";
    int result = std::system(command.c_str());
    return (result == 0);
}

std::string FileParser::trim(const std::string& str)
{
    const auto first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos)
        return "";

    const auto last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, (last - first + 1));
}
