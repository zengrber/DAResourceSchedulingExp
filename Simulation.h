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

// 简单版 Simulation：
// - 在 t=0 调用一次 scheduler.runBatch 分配所有已到达的任务；
// - 然后时间从 0 跑到 timeLimit，期间任务按 duration 完成并释放资源；
// - 最后计算基本指标（吞吐量、等待时间等）。
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
