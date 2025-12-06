#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>

#include "Config.h"
#include "DataGenerator.h"
#include "Simulation.h"
#include "BaseScheduler.h"
#include "DAScheduler.h"
#include "Metrics.h"
#include "Job.h"
#include "Server.h"
#include "ResultWriter.h"

template <typename T>
void freePtrVector(std::vector<T*>& vec) {
    for (T* p : vec) {
        delete p;
    }
    vec.clear();
}

std::string doubleToString(double value, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

void freeAllForOneRun(std::vector<Job*>& trueJobs,
                      std::vector<Job*>& jobsBaseTruth,
                      std::vector<Job*>& jobsBaseStrat,
                      std::vector<Job*>& jobsDATruth,
                      std::vector<Job*>& jobsDAStrat,
                      std::vector<Server*>& serversBaseTruth,
                      std::vector<Server*>& serversBaseStrat,
                      std::vector<Server*>& serversDATruth,
                      std::vector<Server*>& serversDAStrat) {
    freePtrVector(trueJobs);
    freePtrVector(jobsBaseTruth);
    freePtrVector(jobsBaseStrat);
    freePtrVector(jobsDATruth);
    freePtrVector(jobsDAStrat);

    freePtrVector(serversBaseTruth);
    freePtrVector(serversBaseStrat);
    freePtrVector(serversDATruth);
    freePtrVector(serversDAStrat);
}

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
    if (!loadConfigFromFile(configPath, rc)) {
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

    bool enableVisualization = (rc.gen.numJobs <= 50 && rc.numSeeds == 1);

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

        // ---- Base Truthful ----
        sim.clearRunRecords();
        Metrics mBaseTruth = sim.run(baseSched, jobsBaseTruth, serversBaseTruth, rc.batchSize, &sim);
        if(enableVisualization) sim.dumpRunRecordsToCSV("results/schedule_base_truth.csv", serversBaseTruth);

        // ---- Base Strategic ----
        sim.clearRunRecords();
        Metrics mBaseStrat = sim.run(baseSched, jobsBaseStrat, serversBaseStrat, rc.batchSize, &sim);
        if(enableVisualization) sim.dumpRunRecordsToCSV("results/schedule_base_strat.csv", serversBaseStrat);

        // ---- DA Truthful ----
        sim.clearRunRecords();
        Metrics mDATruth = sim.run(daSched, jobsDATruth, serversDATruth, rc.batchSize, &sim);
        if(enableVisualization) sim.dumpRunRecordsToCSV("results/schedule_da_truth.csv", serversDATruth);

        // ---- DA Strategic ----
        sim.clearRunRecords();
        Metrics mDAStrat = sim.run(daSched, jobsDAStrat, serversDAStrat, rc.batchSize, &sim);
        if(enableVisualization) sim.dumpRunRecordsToCSV("results/schedule_da_strat.csv", serversDAStrat);

        sim.clearRunRecords();

        std::cout << "=== Base Truthful ===\n";
        mBaseTruth.print();
        std::cout << "\n=== Base Strategic ===\n";
        mBaseStrat.print();
        std::cout << "\n=== DA Truthful ===\n";
        mDATruth.print();
        std::cout << "\n=== DA Strategic ===\n";
        mDAStrat.print();
        std::cout << "\n";

        freeAllForOneRun(trueJobs,
                        jobsBaseTruth,
                        jobsBaseStrat,
                        jobsDATruth,
                        jobsDAStrat,
                        serversBaseTruth,
                        serversBaseStrat,
                        serversDATruth,
                        serversDAStrat);

        std::string configName;
        std::string writeFileName;
        if(enableVisualization) { //small case visulization
            configName = configPath;
            writeFileName = "results/results_small_case.csv";
        } else {
            configName = configPath;
            writeFileName = "results/results_" + 
                            doubleToString(rc.gen.misreportProb, 2) +
                            "_" +
                            doubleToString(rc.gen.misreportAlpha, 2) +
                            ".csv";
        }   
        ResultWriter::writeCSV(writeFileName, configName, "base_truth", seed, mBaseTruth);
        ResultWriter::writeCSV(writeFileName, configName, "base_strat", seed, mBaseStrat);
        ResultWriter::writeCSV(writeFileName, configName, "da_truth",   seed, mDATruth);
        ResultWriter::writeCSV(writeFileName, configName, "da_strat",   seed, mDAStrat);
    }

    return 0;
}
