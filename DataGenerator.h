#ifndef DATA_GENERATOR_H
#define DATA_GENERATOR_H

#include <vector>
#include <random>

class Job;
class Server;

struct GeneratorConfig {
    int numJobs          = 100;
    int numServers       = 5;

    int serverCapMin     = 10;   // min server capacity
    int serverCapMax     = 20;   // max server capacity

    int demandMin        = 1;    // min true demand
    int demandMax        = 10;   // max true demand

    int durationMin      = 1;    // min durantion
    int durationMax      = 5;    // max duration

    int maxArrivalTime   = 20;   // max arrival time

    double misreportProb = 0.0;  // lie rate p
    double misreportAlpha= 0.0;  // lie degree α（0.5 stands for ±50%）

    unsigned int seed    = 42;   // random seed
};

class DataGenerator {
public:
    explicit DataGenerator(const GeneratorConfig& cfg);

    // generate ground-truth jobs（trueDemand/arrival/duration）
    void generateTrueJobs(std::vector<Job*>& outJobs) const;

    // generate a full truly reported version base on GT
    void makeTruthfulCopy(const std::vector<Job*>& trueJobs,
                          std::vector<Job*>& outJobs) const;

    // generate a strategically reported version base on GT
    void makeStrategicCopy(const std::vector<Job*>& trueJobs,
                           std::vector<Job*>& outJobs) const;

    // generate server lists
    void generateServers(std::vector<Server*>& outServers) const;

private:
    GeneratorConfig cfg_;
};

#endif // DATA_GENERATOR_H
