#include "DAScheduler.h"
#include "Job.h"
#include "Server.h"
#include "Simulation.h"

#include <vector>
#include <algorithm>
#include <cstddef>

// 工具：按 server id 找到在 servers 向量中的下标
static int findServerIndexById(const std::vector<Server*>& servers, int sid) {
    for (std::size_t i = 0; i < servers.size(); ++i) {
        if (servers[i] && servers[i]->id() == sid) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// 工具：判断 job 是否已经在某个 matches 列表里（表示已经被这轮 DA 接受）
static bool isMatched(const std::vector<std::vector<Job*>>& matches, Job* job) {
    for (const auto& vec : matches) {
        for (Job* j : vec) {
            if (j == job) return true;
        }
    }
    return false;
}

void DAScheduler::runBatch(std::vector<Job*>& jobs,
                           std::vector<Server*>& servers,
                           int currentTime,
                           Simulation *sim)
{
    // 1. 收集当前参与本次 DA 的 job：必须是 Waiting 状态
    std::vector<Job*> active;
    active.reserve(jobs.size());
    for (Job* j : jobs) {
        if (!j) continue;
        if (j->isWaiting()) {
            active.push_back(j);
        }
    }

    if (active.empty() || servers.empty()) {
        return;
    }

    // 2. 每个 server 维护一份“新接受的 job 列表”（不动原有 Running job）
    std::vector<std::vector<Job*>> matches(servers.size());
    std::vector<std::vector<Job*>> proposals(servers.size());

    // 3. 重置每个 active job 的偏好迭代指针
    for (Job* j : active) {
        j->resetPreferencesIter();
    }

    // 4. DA 主循环：直到没有新的提案为止
    while (true) {
        bool anyProposal = false;

        // 4.1 让所有“尚未匹配且还有剩余偏好”的 job 提案
        for (Job* job : active) {
            if (!job) continue;
            if (!job->isWaiting()) continue; // 只管 Waiting
            if (isMatched(matches, job)) continue; // 已经在 matches 的不再提案

            int sid = job->nextPreferredServer();
            if (sid < 0) {
                job->markFailed(currentTime);
                continue; // 没有剩余偏好
            }

            int sIdx = findServerIndexById(servers, sid);
            if (sIdx < 0) {
                // 找不到这个 server，直接跳过这个偏好
                job->advancePreference();
                continue;
            }

            proposals[sIdx].push_back(job);
            anyProposal = true;
        }

        // 如果这一轮没有任何 job 提案，DA 结束
        if (!anyProposal) {
            break;
        }

        // 4.2 每个 server 处理本轮收到的提案
        for (std::size_t si = 0; si < servers.size(); ++si) {
            Server* s = servers[si];
            if (!s) continue;

            auto& curMatches = matches[si];
            auto& newProps   = proposals[si];

            if (newProps.empty()) {
                continue;
            }

            // 候选集合 = 当前已匹配 + 本轮新提案
            std::vector<Job*> candidates;
            candidates.reserve(curMatches.size() + newProps.size());
            candidates.insert(candidates.end(), curMatches.begin(), curMatches.end());
            candidates.insert(candidates.end(), newProps.begin(), newProps.end());

            // 去重一下（防御）
            std::sort(candidates.begin(), candidates.end());
            candidates.erase(std::unique(candidates.begin(), candidates.end()),
                             candidates.end());

            // 服务器偏好：这里仍然用 reportedDemand 越小越好
            std::sort(candidates.begin(), candidates.end(),
                      [](Job* a, Job* b) {
                          return a->reportedDemand() < b->reportedDemand();
                      });

            // 根据“剩余真实容量” greedy 选取 subset
            std::vector<Job*> newMatches;
            newMatches.reserve(candidates.size());

            int remainingCap = s->freeCapacity(); // 注意：freeCapacity 已经考虑了旧 Running job

            for (Job* job : candidates) {
                int d = job->reportedDemand(); // 真实需求作为容量约束
                if (d <= remainingCap) {
                    newMatches.push_back(job);
                    remainingCap -= d;
                }
            }

            // 对于本 server 来说，本轮 DA 的结果就是 newMatches
            // 原来的 curMatches 里被移除的 job，就视为被拒绝，需要尝试下一个偏好
            // proposals 中未被选中的 job 同样被拒绝

            // 先记录老的 matches（用于比较谁被踢了）
            std::vector<Job*> oldMatches = curMatches;
            curMatches = newMatches;

            // oldMatches 中被踢出的 job：advancePreference (如果还有 server 可以试)
            for (Job* job : oldMatches) {
                // 不在新 matches 里，说明被 server 拒绝了
                if (std::find(curMatches.begin(), curMatches.end(), job) == curMatches.end()) {
                    job->advancePreference();
                }
            }

            // proposals 中没被选中的 job：同样 advancePreference
            for (Job* job : newProps) {
                if (std::find(curMatches.begin(), curMatches.end(), job) == curMatches.end()) {
                    job->advancePreference();
                }
            }

            // 清空本 server 的 proposals，为下一轮准备
            newProps.clear();
        }
    }

    // 5. DA 收敛后，把 matches 写回 Server，并标记 job 为 Running
    //    注意：我们不会动原有正在 Running 的 job，只在剩余 freeCapacity 上加新 job
    for (std::size_t si = 0; si < servers.size(); ++si) {
        Server* s = servers[si];
        if (!s) continue;

        auto& curMatches = matches[si];
        for (Job* job : curMatches) {
            if (!job) continue;
            // 再次确认这个 job 仍是 Waiting（防御）
            if (job->isWaiting()) {
                if (s->accept(job)) {      // accept 用 trueDemand 检查容量
                    job->markRunning(currentTime);
                    if (sim) sim->logJobStart(job, s, currentTime);
                } else {
                    job->markFailed(currentTime);
                }
            }
        }
    }
}
