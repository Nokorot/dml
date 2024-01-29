#pragma once

#include <vector>

#include "red_black_tree.h"
#include "options.h"

std::string getCachePath(const std::string& cacheDir, std::string filepath);

class DML 
{
public:
    DML(Options &opt, const std::string &filepath);
    ~DML();

    const std::vector<char>& getKeyBuffer();
    const char* search(const std::string key);

    void listKeys(FILE *sink);
    std::string browse(char *browseProgram);

    void pushRecent(const std::string key);

private:
    Options m_opt;

    RedBlackTree m_indexTree; 
    std::vector<char> m_valueBuffer;

    std::vector<std::string> m_history;

    const std::string m_filepath, m_cacheFilepath;

    void saveHistory();
    void loadHistory();

    void saveCacheFile();
    void loadCacheFiles();
};

