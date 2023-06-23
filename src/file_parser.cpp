#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstring>
#include <vector>

#include <unistd.h>

#include "base64.h"
#include "shell_command.h"

#include "file_parser.h"

// Helper function
bool isProgramInPath(const std::string& programName);
std::string trim(const std::string& str);

std::streampos GetFileSize(std::ifstream& file);
char* LoadFileIntoBuffer(const std::string& filename);


// FileParser

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

    if (directive.empty())
        return;

    // TODO IF first character is %, hashTable

    // Check for the '%include' directive
    if (directive.compare("%endif") == 0)
    {
        skipingToEndif = false;
        return;
    } 
    else if (skipingToEndif) { return; }
    else if (directive.compare("%include") == 0)
    {
        std::string includeFile;
        iss >> includeFile;

        // TODO: Error if there is more on the line?
  
        // Load the included file recursively
        loadFromFile(includeFile, indexTree, valueBuffer);
    }
    else if (directive.compare("%pregen") == 0)
    {
        // TODO: This is a bit absurd .
        std::string shell_command;
        std::getline(iss >> std::ws, shell_command);
    
        ShellCommand shcmd = ShellCommand(shell_command, OPEN_FROM_CHILD_PIPE);
        shcmd.execute();

        std::string data = shcmd.readOutput();

        loadFromBuffer( data.c_str(), indexTree, valueBuffer);
    
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
    else if (directive.compare("%if") == 0)
    {
        // TODO: This is a bit absurd .
        std::string condition;
        std::getline(iss >> std::ws, condition);

        ShellCommand shcmd = ShellCommand(condition, 0);

        int exitStatus = shcmd.waitforChildExit();
        if (exitStatus != 0)
            skipingToEndif = true;
    }
    else  
    {
        std::string key = directive;
        std::string value;
                    
        std::getline(iss >> std::ws, value);  // Extract the entire rest of the line as the value
        if(value.empty())
            value = key;
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
    char* buffer = LoadFileIntoBuffer(filepath);
    if (!buffer)
    {
        std::cerr << "Failed to open the input file." << std::endl;
        return;
    }
 
    loadFromBuffer(buffer, indexTree, valueBuffer);

    delete[] buffer; 
}

void FileParser::loadFromBuffer(const char *buffer, RedBlackTree &indexTree, std::vector<char> &valueBuffer)
{
    std::istringstream iss(buffer);

    std::string line;
    std::string pendingLine; // Store the pending line continuation
    while (std::getline(iss, line))
    {
        // Check if the line continuation is pending
        if (!pendingLine.empty())
        {
            line = pendingLine + trim(line);
            pendingLine = "";
        }

        // NOTE: This comes after the backslash check

        // Check if the line ends with a backslash
        if (line.back() == '\\')
        {
            line.pop_back();
            pendingLine = trim(line);
            continue;
        }

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

    if (!pendingLine.empty()) {
        std::string trimmedLine = trim(pendingLine);
        loadLine(trimmedLine, indexTree, valueBuffer);
    }
}

bool isProgramInPath(const std::string& programName) {
    std::string command = "which " + programName + " > /dev/null 2>&1";
    int result = std::system(command.c_str());
    return (result == 0);
}

std::string trim(const std::string& str)
{
    const auto first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos)
        return "";

    const auto last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, (last - first + 1));
}



std::streampos GetFileSize(std::ifstream& file) {
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    return fileSize;
}

char* LoadFileIntoBuffer(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cout << "Failed to open file: " << filename << std::endl;
        return nullptr;
    }

    std::streampos fileSize = GetFileSize(file);
    

    char* buffer = new char[int(fileSize) + 1];

    file.seekg(0, std::ios::beg);
    file.read(buffer, fileSize);
    file.close();

    buffer[fileSize] = 0;

    return buffer;
}
