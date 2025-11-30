// Config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include "DataGenerator.h"  

struct RunConfig {
    GeneratorConfig gen;  
    int timeLimit  = 150; 
    int batchSize  = 4;   
    int numSeeds   = 1; 
};

// 从文本文件读取配置，成功返回 true
bool loadConfigFromFile(const std::string& path, RunConfig& cfg);

#endif // CONFIG_H
