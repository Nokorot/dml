#include <bits/types/FILE.h>
#include <cstddef>
#include <cstdio>
// #include <cstdlib>
#include <iostream>
#include <signal.h>
#include <string>
#include <sys/types.h>
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

std::string getDefaultCacheDirectory();
void createDirectory(const std::string& directoryPath) ;
int main(int argc, char **argv);

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
    bool *help = flag_bool("help", 'h', "Print this help to stdout and exit with 0");
    bool *list = flag_bool("list", 'l', "List all the keys");

    bool *ignoreCache = flag_bool("ignore-cache", 'i', "Ignore cache and reload the file");

    char **browseProgram = flag_str("browse-prg", nullptr, "Specify a program with witch to browse the list");
    // TODO: Example _dml_composer --browse-prg "(){ < \"$1\" dmenu > \"$2\"; }" ???
    
    std::string defaultCacheDir = getDefaultCacheDirectory() + "/dml";
    char **cacheDir = flag_str("cache-dir", defaultCacheDir.c_str(), "Set cache directory");

    if (!flag_parse(argc, argv)) {
        usage(stderr, program_name);
        flag_print_error(stderr);
        exit(1);
    }
    
    // argc = flag_rest_argc();
    argv = flag_rest_argv();

    if (*help) {
        usage(stdout, program_name);
        exit(0);
    }

    Options opt;
    opt.cacheDir = *cacheDir; 
    createDirectory(opt.cacheDir);
    opt.ignoreCache = *ignoreCache;
    
    if (!argv[0]) {
        fprintf(stderr, "ERROR: Not enough arguments provided! (see -h)\n");
        exit(1);
    }

    DML dml(opt, argv[0]);

    if (argv[1])
    {
        const char* command = dml.search(argv[1]);
        fprintf(stdout, "%s\n", command);
    }
    else if (*browseProgram) 
    {
        std::string value = dml.browse(*browseProgram);
        if (value.length())
            fprintf(stdout, "%s\n", value.c_str());

        return 0;
    }
    else /* if (*list) */ {
        const std::vector<char>& keyBuffer = dml.getKeyBuffer();
        fprintf(stdout, "%.*s", (int) keyBuffer.size(), &keyBuffer[0]);
        exit(0);
    } 
}

