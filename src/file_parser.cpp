#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstring>
#include <system_error>
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

std::string parseString(const char *ch, const char **end)
{
    const char *head = ch;
    if (*ch == '"') {
      ++head, ++ch;
      while (*ch && *ch!='"') ++ch;
      if (end) *end = ch + (ch ? 1 : 0);
      return std::string(head, ch-head);
    }

    while (*ch && *ch!=' ' && *ch!='\t') ++ch;
    if (end) *end = ch;
    std::string restult(head, ch-head);

    return restult;
}

std::string replaceEnvVars(const std::string &input) {
    std::string output = input;

    size_t start = 0;
    while ((start = output.find('$', start)) != std::string::npos) {
        size_t end = start + 1;
        while (end < output.length() &&
                (isalnum(output[end]) || output[end] == '_')) {
            end++;
        }

        size_t varLength = end - start;
        if (varLength > 1) {
            std::string varName = output.substr(start + 1, varLength - 1);

            const char *envValue = std::getenv(varName.c_str());
            if (envValue != nullptr) {
                output.replace(start, varLength, envValue);
            } else {
                output.erase(start, 1);
            }
        } else {
            output.erase(start, 1);
        }
    }

    return output;
}

void FileParser::loadLine(std::string trimmedLine, RedBlackTree &indexTree, std::vector<char> &valueBuffer) {

    const char *ch = trimmedLine.c_str();
    std::string directive = parseString(ch, &ch);

    if (directive.empty())
        return;

    if (skipIfBlock == 1) {
        if (directive.compare("%else") == 0) {
            skipIfBlock = 0;
            return;
        }
        else if (directive.compare("%elseif") == 0) {
            std::string condition(ch);

            ShellCommand shcmd = ShellCommand(condition, 0);
            shcmd.execute();

            int exitStatus = shcmd.waitforChildExit();

            skipIfBlock = (exitStatus == 0) ? 0 : 1;
            return;
        }
    }

    if (skipIfBlock > 0) {
        if (directive.compare("%if") == 0) {
            skipIfBlock++;
            ifBlockDepth++;
        }
        else if (directive.compare("%endif") == 0) {
            skipIfBlock--;
            ifBlockDepth--;
        }
        return;
    }

    if (directive.compare("%endif") == 0) {
        ifBlockDepth--;
        return;
    }


    // Ship spaces
    while (*ch && (*ch==' ' || *ch=='\t')) ++ch;

    // TODO IF first character is %, hashTable
    if (directive.compare("%include") == 0)
    {
        std::string includeFile = parseString(ch, &ch);
        includeFile = replaceEnvVars(includeFile.c_str());

        // Load the included file recursively
        loadFromFile(includeFile, indexTree, valueBuffer);
    }
    else if (directive.compare("%pregen") == 0)
    {
        // TODO: This is a bit absurd .
        std::string shell_command(ch);

        ShellCommand shcmd = ShellCommand(shell_command, OPEN_FROM_CHILD_PIPE);
        shcmd.execute();

        std::string data = shcmd.readOutput();

        loadFromBuffer( data.c_str(), indexTree, valueBuffer);
    }
    else if (directive.compare("%check") == 0)
    {
        std::string program = parseString(ch, &ch);

        if (isProgramInPath(program))
        {
            std::string line(ch);
            loadLine(program + " " + line, indexTree, valueBuffer);
        }
    }
    else if (directive.compare("%check*") == 0)
    {
        std::string program = parseString(ch, &ch);

        if (isProgramInPath(program))
        {
            std::string line(ch);
            loadLine(line, indexTree, valueBuffer);
        }
    }
    else if (directive.compare("%if") == 0)
    {
        std::string condition(ch);

        ShellCommand shcmd = ShellCommand(condition, 0);
        shcmd.execute();

        int exitStatus = shcmd.waitforChildExit();

        ifBlockDepth++;
        if (exitStatus != 0)
            skipIfBlock = 1;
    }
    else if (directive.compare("%else") == 0
            || directive.compare("%elseif") == 0 )
    {

        if (!ifBlockDepth) {
            fprintf(stderr, "ERROR: Unexpected '%s' directive!\n");
            // FILE and Line for logging would be useful
            return;
        }
        
        skipIfBlock = 1;
    }
    /* else if (directive.compare("%def") == 0)
    {
        // TODO: This is a bit absurd .
    }*/
    else
    {
        std::string key = directive;
        std::string value(ch);

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

    std::string line = "";
    std::string pendingLine; // Store the pending line continuation
    while (std::getline(iss, line))
    {
        line = pendingLine + trim(line);

        // Check if the line ends with a backslash
        if (line.back() == '\\')
        {
            line.pop_back();
            pendingLine = line;
            continue;
        }
        pendingLine = "";

        if (line.empty() || line[0] == '#') 
            continue;

        // Handle comments
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos)
        {
            // Check if the '#' is escaped
            if (commentPos > 0 && line[commentPos - 1] == '\\')
                line.erase(commentPos - 1, 1);
            else
                line.erase(commentPos);
        }

        loadLine(line, indexTree, valueBuffer);
        line = "";
    }

    // If the last line ends with '\', include it anyway
    if (!line.empty()) {
        loadLine(line, indexTree, valueBuffer);
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
