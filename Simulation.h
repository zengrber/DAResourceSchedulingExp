#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>

class Job;
class Server;
class Scheduler;
struct Metrics;

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
                int batchSize);

private:
    int timeLimit_;

    void updateFinishedJobs(std::vector<Server*>& servers,
                            int currentTime);
};

#endif // SIMULATION_H
