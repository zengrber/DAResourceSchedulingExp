#include "BaseScheduler.h"
#include "Job.h"
#include "Server.h"

#include <climits>   // INT_MAX
#include <cstddef>   // std::size_t

// 简单启发式：Best-Fit on reportedDemand
// - 对每个 Waiting job：
//   - 在所有 server 中寻找 freeCapacity >= reportedDemand 且
//     (freeCapacity - reportedDemand) 最小的 server（尽量不浪费空间）。
//   - 如果找到，则将 job 分配给该 server，标记为 Running。
//   - 如果找不到，job 保持 Waiting（之后批次再尝试）。
void BaseScheduler::runBatch(std::vector<Job*>& jobs,
                             std::vector<Server*>& servers,
                             int currentTime)
{
    // 对每个 job 尝试分配一次
    for (Job* job : jobs) {
        if (!job) continue;
        if (!job->isWaiting()) continue;  // 只管等待中的任务

        int demand = job->trueDemand();
        if (demand <= 0) {
            // 理论上不应该发生，但防御一下
            continue;
        }

        Server* bestServer = nullptr;
        int bestSlack = INT_MAX; // freeCapacity - demand, 越小越好（但 >= 0）

        for (Server* server : servers) {
            if (!server) continue;

            int slack = server->freeCapacity() - demand;
            if (slack >= 0 && slack < bestSlack) {
                bestSlack = slack;
                bestServer = server;
            }
        }

        // 如果找到了可容纳的 server，就分配
        if (bestServer != nullptr) {
            // Server::accept 里会更新 usedCapacity_ 和 assignedJobs_
            if (bestServer->accept(job)) {
                job->markRunning(currentTime);
                // 后续由 Simulation 按 duration 减去时间并在完成时 markFinished()
            }
        }
        // 否则 job 继续保持 Waiting，在之后的批次再尝试
    }
}
