#include "Simulation.h"
#include "Job.h"
#include "Server.h"
#include "Scheduler.h"
#include "Metrics.h"

#include <vector>

// 在当前时间点检查：哪些 running job 已经跑完 duration
void Simulation::updateFinishedJobs(std::vector<Server*>& servers, int currentTime)
{
    for (Server* s : servers) {
        if (!s) continue;
        s->removeFinishedJobs(currentTime);
    }
}

Metrics Simulation::run(Scheduler& scheduler,
                        std::vector<Job*>& jobs,
                        std::vector<Server*>& servers,
                        int batchSize)
{
    Metrics metrics;
    metrics.totalJobs = static_cast<int>(jobs.size());

    if (jobs.empty() || servers.empty()) {
        return metrics;
    }

    int currentTime = 0;
    double totalCap = 0.0;
    for (Server* s : servers) {
        if (s) totalCap += s->capacity();
    }

    double utilizationSum = 0.0;
    int   timeSteps       = 0;
    std::vector<Job*> waiting;

    for (currentTime = 0; currentTime <= timeLimit_; ++currentTime) {
        for (Job* j : jobs) {
            if (j && j->arrivalTime() == currentTime) {
                waiting.push_back(j);
            }
        }

        updateFinishedJobs(servers, currentTime);

        if (currentTime % batchSize == 0) {
            scheduler.runBatch(waiting, servers, currentTime);
        }

        // 统计本时间步的资源使用
        double usedCap = 0.0;
        for (Server* s : servers) {
            if (s) usedCap += s->usedCapacity();
        }

        if (totalCap > 0.0) {
            utilizationSum += (usedCap / totalCap);
        }
        ++timeSteps;
    }

    // 统计指标
    int finished = 0;
    double sumCompletion = 0.0;
    double sumWaiting = 0.0;

    for (Job* job : jobs) {
        if (!job) continue;
        if (job->isFinished()) {
            ++finished;
            int completionTime = job->finishTime() - job->arrivalTime();
            int waitingTime    = job->startTime()  - job->arrivalTime();
            sumCompletion += completionTime;
            sumWaiting    += waitingTime;
        }
    }

    metrics.finishedJobs = finished;
    if (finished > 0) {
        metrics.avgCompletionTime = sumCompletion / finished;
        metrics.avgWaitingTime    = sumWaiting    / finished;
    }

    if (timeSteps > 0) {
        metrics.avgUtilization = utilizationSum / timeSteps;
    }

    return metrics;
}
