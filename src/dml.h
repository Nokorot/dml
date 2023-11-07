#pragma once

#include <vector>

#include "red_black_tree.h"
#include "options.h"

std::string getCachePath(const std::string& cacheDir, std::string filepath);

class DML 
{
public:
    DML(Options &opt, const std::string &filepath);

    const std::vector<char>& getKeyBuffer();
    const char* search(const std::string &key);

    std::string browse(char *browseProgram);


private:
    RedBlackTree m_indexTree; 
    std::vector<char> m_valueBuffer;

    const std::string m_filepath, m_cacheFilepath;

    void saveCacheFile();
    void loadCacheFiles();
};

