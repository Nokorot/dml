#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <openssl/evp.h>
#include <filesystem>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>

#include <unistd.h>

#include "base64.h"
#include "popen2.h"
// #include "execute.h"

#include "file_parser.h"

extern char** environ;

char* LoadFileIntoBuffer(const std::string& filename);

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
    
        struct popen2 info;
        popen2_env(shell_command.c_str(), &info, environ);
        close(info.to_child);

        
        // char *buff = (char *) malloc(sizeof(char)*1024);
        // memset(buff, 0, 1024);
        // read(info.from_child, buff, 1024);

    
        // Read data from the pipe
        static const int bufferSize = 1024;
        char buffer[bufferSize];
        std::ostringstream output;
        ssize_t bytes_read;
        while (0 < (bytes_read = read(info.from_child, buffer, bufferSize))) {
            output << buffer;
        }

        // Check if we reached the end of the data
        if (bytes_read < 0) {
            std::cout << "Error occurred while reading the pipe!" << std::endl;
        }

        // Close the pipe
        close(info.from_child);

        // Get the data as a string
        std::string data = output.str();

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

        const char* args[] = { "/bin/bash", "-c", (char * const) condition.c_str(), nullptr };
        pid_t p = fork();
        if(p < 0) {
            printf("ERROR: Failed to fork process!");
            exit(1);
        }
        if(p == 0) 
        { /* child */
            execve(args[0], const_cast<char**>(args), environ);
        }

        int status;
        waitpid(p, &status, 0);

        int exitStatus = -1;
        if (WIFEXITED(status)) {
            // Child process exited normally
            exitStatus = WEXITSTATUS(status);
            
            // if VERBOSE
            // printf("If command  '%s' exited with %u\n", condition.c_str(), exitStatus);
        } else if (WIFSIGNALED(status)) {
            // Child process terminated by a signal
            int signalNumber = WTERMSIG(status);
            std::cout << "Child process terminated by signal: " << signalNumber << std::endl;
            exit(1);
        } else {
            // Other termination conditions
            std::cout << "Child process terminated abnormally." << std::endl;
            exit(1);
        }
        
        if (exitStatus != 0)
        {
            skipingToEndif = true;
        }
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
    // std::ifstream inFile(filepath);
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
            line = pendingLine + line;
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
