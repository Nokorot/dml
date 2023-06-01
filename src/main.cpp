#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#include "options.h"
#include "dml.h"

#define FLAG_IMPLEMENTATION
#include "flag.h"

void usage(FILE *sink, const char *program)
{
    fprintf(sink, "Usage: %s [OPTIONS] [--] +[key-flags] <key-string> ... \n\n", program);
    fprintf(sink, "OPTIONS:\n");
    flag_print_options(sink);
}

// NODE: Linux specific
std::string getDefaultCacheDirectory() 
{
    const char* homeDir = std::getenv("HOME");
    if (homeDir) {
        std::filesystem::path cacheDir = std::filesystem::path(homeDir) / ".cache";
        if (std::filesystem::exists(cacheDir))
            return cacheDir.string();
    }
    return "";
}

void createDirectory(const std::string& directoryPath) 
{
    if (!std::filesystem::exists(directoryPath)) {
        if (!std::filesystem::create_directory(directoryPath)) {
            std::cerr << "Failed to create directory: " << directoryPath << std::endl;
        }
    } 
}

int main(int argc, char **argv) 
{
    const char *program_name = *argv;

    // Define command line flags
    bool *help = flag_bool("help", false, "Print this help to stdout and exit with 0");
    bool *list = flag_bool("l", false, "List all the keys");

    bool *ignoreCache = flag_bool("i", false, "Ignore cache and reload the file");
    
    if (!flag_parse(&argc, argv)) {
        usage(stderr, program_name);
        flag_print_error(stderr);
        exit(1);
    }
    
    char** args = flag_rest_argv();

    if (*help) {
        usage(stdout, program_name);
        exit(0);
    }

    Options opt;
    opt.cacheDir = getDefaultCacheDirectory() + "/dml"; 
    createDirectory(opt.cacheDir);
    opt.ignoreCache = *ignoreCache;

    DML dml(opt, argv[0]);

    if (*list) {
        const std::vector<char>& keyBuffer = dml.getKeyBuffer();
        fprintf(stdout, "%.*s", (int) keyBuffer.size(), &keyBuffer[0]);
        exit(0);
    }
    
    if (args[1]) {
        const char* command = dml.search(args[1]);
        fprintf(stdout, "%s\n", command);
    }
}
