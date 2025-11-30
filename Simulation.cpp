#include "Simulation.h"
#include "Job.h"
#include "Server.h"
#include "Scheduler.h"
#include "Metrics.h"

#include <vector>
#include <string>
#include <fstream>

// 在当前时间点检查：哪些 running job 已经跑完 duration
void Simulation::updateFinishedJobs(std::vector<Server*>& servers, 
                                    int currentTime,
                                    Simulation *sim)
{
    for (Server* s : servers) {
        if (!s) continue;
        s->removeFinishedJobs(currentTime, sim);
    }
}

Metrics Simulation::run(Scheduler& scheduler,
                        std::vector<Job*>& jobs,
                        std::vector<Server*>& servers,
                        int batchSize,
                        Simulation *sim)
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

        updateFinishedJobs(servers, currentTime, sim);

        if (currentTime % batchSize == 0) {
            scheduler.runBatch(waiting, servers, currentTime, this);
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

void Simulation::logJobStart(Job* job, Server* server, int currentTime) {
    if (!job || !server) return;
    JobRunRecord rec;
    rec.jobId    = job->id();          
    rec.serverId = server->id();
    rec.startTime = currentTime;
    rec.endTime   = -1;
    rec.demand    = job->trueDemand();
    runRecords_.push_back(rec);
}

void Simulation::logJobFinish(Job* job, int currentTime) {
    if (!job) return;
    int jid = job->id();
    // 从后往前找最后一个还没填 endTime 的记录
    for (int i = static_cast<int>(runRecords_.size()) - 1; i >= 0; --i) {
        if (runRecords_[i].jobId == jid && runRecords_[i].endTime < 0) {
            runRecords_[i].endTime = currentTime;
            break;
        }
    }
}

void Simulation::dumpRunRecordsToCSV(const std::string& filename, std::vector<Server*>& servers) const {
    std::ofstream out(filename);
    if (!out) return;
    out << "jobId,serverId,startTime,endTime,demand,serverCap\n";
    for (const auto& r : runRecords_) {
        if (r.endTime < 0) continue; // 只导出完整跑完的
        int cap = servers[r.serverId]->capacity();
        out << r.jobId    << ","
            << r.serverId << ","
            << r.startTime << ","
            << r.endTime   << ","
            << r.demand    << ","
            << cap         << "\n";
    }
}

void Simulation::clearRunRecords() {
    runRecords_.clear();
}