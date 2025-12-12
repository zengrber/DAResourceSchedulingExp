#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>
#include <string>

class Job;
class Server;
class Scheduler;
struct Metrics;

struct JobRunRecord {
    int jobId;
    int serverId;
    int startTime;   
    int endTime;     
    int demand;    
};

// Simulation：
// - Apply scheduler.runBatch when t=0；
// - From t=0 to timeLimit，jobs are finished and release capacity according to duration；
// - Finally compute metrics
class Simulation {
public:
    Simulation(int timeLimit)
        : timeLimit_(timeLimit) {}

    Metrics run(Scheduler& scheduler,
                std::vector<Job*>& jobs,
                std::vector<Server*>& servers,
                int batchSize,
                Simulation *sim);
      
    void dumpRunRecordsToCSV(const std::string& filename, std::vector<Server*>& servers) const;
    void logJobStart(Job* job, Server* server, int currentTime);
    void logJobFinish(Job* job, int currentTime);
    void clearRunRecords(); 

private:
    int timeLimit_;
    std::vector<JobRunRecord> runRecords_;

    void updateFinishedJobs(std::vector<Server*>& servers,
                            int currentTime,
                            Simulation *sim);
};

#endif // SIMULATION_H
