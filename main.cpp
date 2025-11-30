#include <iostream>
#include <vector>
#include <random>
#include <chrono>

#include "Config.h"
#include "DataGenerator.h"
#include "Simulation.h"
#include "BaseScheduler.h"
#include "DAScheduler.h"
#include "Metrics.h"
#include "Job.h"
#include "Server.h"
#include "ResultWriter.h"

unsigned int generateRandomSeed() {
    unsigned int timePart = 
        static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());

    std::random_device rd;
    unsigned int rdPart = rd();  

    return timePart ^ (rdPart + 0x9e3779b9);
}

int main(int argc, char** argv) {
    std::string configPath = "config.txt";
    if (argc >= 2) {
        configPath = argv[1];
    }

    RunConfig rc;
    double prob = loadConfigFromFile(configPath, rc);
    if (prob >= 0) {
        std::cerr << "Using default config values.\n";
    }

    std::cout << "Loaded config from " << configPath << "\n";

    unsigned int baseSeed;
    if (rc.gen.seed == (unsigned int)(-1)) {  
        baseSeed = generateRandomSeed();
        std::cout << "[INFO] Auto-generated seed = " << baseSeed << "\n";
    } else {
        baseSeed = rc.gen.seed;
        std::cout << "[INFO] Using configured seed = " << baseSeed << "\n";
    }

    for (int s = 0; s < rc.numSeeds; ++s) {
        unsigned int seed = baseSeed + s;
        rc.gen.seed = seed;

        std::cout << "===== Seed " << seed << " =====\n";

        DataGenerator gen(rc.gen);

        // true jobs
        std::vector<Job*> trueJobs;
        gen.generateTrueJobs(trueJobs);

        // servers for each scheduler / scenario
        std::vector<Server*> serversBaseTruth;
        std::vector<Server*> serversBaseStrat;
        std::vector<Server*> serversDATruth;
        std::vector<Server*> serversDAStrat;

        gen.generateServers(serversBaseTruth);
        gen.generateServers(serversBaseStrat);
        gen.generateServers(serversDATruth);
        gen.generateServers(serversDAStrat);

        // job variants
        std::vector<Job*> jobsBaseTruth;
        std::vector<Job*> jobsBaseStrat;
        std::vector<Job*> jobsDATruth;
        std::vector<Job*> jobsDAStrat;

        gen.makeTruthfulCopy(trueJobs, jobsBaseTruth);
        gen.makeStrategicCopy(trueJobs, jobsBaseStrat);
        gen.makeTruthfulCopy(trueJobs, jobsDATruth);
        gen.makeStrategicCopy(trueJobs, jobsDAStrat);

        BaseScheduler baseSched;
        DAScheduler   daSched;
        Simulation    sim(rc.timeLimit);

        Metrics mBaseTruth = sim.run(baseSched, jobsBaseTruth, serversBaseTruth, rc.batchSize);
        Metrics mBaseStrat = sim.run(baseSched, jobsBaseStrat, serversBaseStrat, rc.batchSize);
        Metrics mDATruth   = sim.run(daSched,   jobsDATruth,   serversDATruth,   rc.batchSize);
        Metrics mDAStrat   = sim.run(daSched,   jobsDAStrat,   serversDAStrat,   rc.batchSize);

        std::cout << "=== Base Truthful ===\n";
        mBaseTruth.print();
        std::cout << "\n=== Base Strategic ===\n";
        mBaseStrat.print();
        std::cout << "\n=== DA Truthful ===\n";
        mDATruth.print();
        std::cout << "\n=== DA Strategic ===\n";
        mDAStrat.print();
        std::cout << "\n";

        // TODO：这里别忘了 delete 所有 new 出来的 Job / Server
        // 你现在先不管，等结构稳定了再加一个统一的 free 函数也行

        std::string configName = configPath;
        std::string writeFileName = "results/results_" + std::to_string(prob) + ".csv";
        ResultWriter::writeCSV(writeFileName, configName, "base_truth", seed, mBaseTruth);
        ResultWriter::writeCSV(writeFileName, configName, "base_strat", seed, mBaseStrat);
        ResultWriter::writeCSV(writeFileName, configName, "da_truth",   seed, mDATruth);
        ResultWriter::writeCSV(writeFileName, configName, "da_strat",   seed, mDAStrat);
    }

    return 0;
}
