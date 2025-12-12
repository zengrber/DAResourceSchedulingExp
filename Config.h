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

bool loadConfigFromFile(const std::string& path, RunConfig& cfg);

#endif // CONFIG_H
