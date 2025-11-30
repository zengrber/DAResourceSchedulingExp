#include "Config.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>

namespace {

// 去掉前后空白
inline std::string trim(const std::string& s) {
    std::size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
        ++start;
    }
    std::size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        --end;
    }
    return s.substr(start, end - start);
}

} // anonymous namespace

bool loadConfigFromFile(const std::string& path, RunConfig& cfg) {
    std::ifstream fin(path);
    if (!fin) {
        std::cerr << "Failed to open config file: " << path << "\n";
        return false;
    }

    std::string line;
    while (std::getline(fin, line)) {
        line = trim(line);
        if (line.empty()) continue;
        if (line[0] == '#') continue;

        auto pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key   = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));

        // int helper
        auto toInt = [](const std::string& v, int defaultVal) -> int {
            try {
                return std::stoi(v);
            } catch (...) {
                return defaultVal;
            }
        };

        // double helper
        auto toDouble = [](const std::string& v, double defaultVal) -> double {
            try {
                return std::stod(v);
            } catch (...) {
                return defaultVal;
            }
        };

        // GeneratorConfig 部分
        if (key == "numJobs") {
            cfg.gen.numJobs = toInt(value, cfg.gen.numJobs);
        } else if (key == "numServers") {
            cfg.gen.numServers = toInt(value, cfg.gen.numServers);
        } else if (key == "serverCapMin") {
            cfg.gen.serverCapMin = toInt(value, cfg.gen.serverCapMin);
        } else if (key == "serverCapMax") {
            cfg.gen.serverCapMax = toInt(value, cfg.gen.serverCapMax);
        } else if (key == "demandMin") {
            cfg.gen.demandMin = toInt(value, cfg.gen.demandMin);
        } else if (key == "demandMax") {
            cfg.gen.demandMax = toInt(value, cfg.gen.demandMax);
        } else if (key == "durationMin") {
            cfg.gen.durationMin = toInt(value, cfg.gen.durationMin);
        } else if (key == "durationMax") {
            cfg.gen.durationMax = toInt(value, cfg.gen.durationMax);
        } else if (key == "maxArrivalTime") {
            cfg.gen.maxArrivalTime = toInt(value, cfg.gen.maxArrivalTime);
        } else if (key == "misreportProb") {
            cfg.gen.misreportProb = toDouble(value, cfg.gen.misreportProb);
        } else if (key == "misreportAlpha") {
            cfg.gen.misreportAlpha = toDouble(value, cfg.gen.misreportAlpha);
        } else if (key == "seed") {
            cfg.gen.seed = static_cast<unsigned int>(
                toInt(value, static_cast<int>(cfg.gen.seed))
            );
        }
        // Simulation 部分
        else if (key == "timeLimit") {
            cfg.timeLimit = toInt(value, cfg.timeLimit);
        } else if (key == "batchSize") {
            cfg.batchSize = toInt(value, cfg.batchSize);
            if (cfg.batchSize <= 0) cfg.batchSize = 1;
        } else if (key == "numSeeds") {
            cfg.numSeeds = toInt(value, cfg.numSeeds);
            if (cfg.numSeeds <= 0) cfg.numSeeds = 1;
        }
    }

    return true;
}