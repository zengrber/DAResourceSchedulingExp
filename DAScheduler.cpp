#include "DAScheduler.h"
#include "Job.h"
#include "Server.h"
#include "Simulation.h"

#include <vector>
#include <algorithm>
#include <cstddef>

static int findServerIndexById(const std::vector<Server*>& servers, int sid) {
    for (std::size_t i = 0; i < servers.size(); ++i) {
        if (servers[i] && servers[i]->id() == sid) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

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
    // S1. grab jobs that participated in current DA round
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

    // S2. Each server maintains a "newly accepted job list"
    std::vector<std::vector<Job*>> matches(servers.size());
    std::vector<std::vector<Job*>> proposals(servers.size());
    
    // S3. Restore active job's preference iterator
    for (Job* j : active) {
        j->resetPreferencesIter();
    }

    // S4. DA main loop
    while (true) {
        bool anyProposal = false;

        // 4.1 Let every "not matched and still have nextpreference" jobs propose
        for (Job* job : active) {
            if (!job) continue;
            if (!job->isWaiting()) continue; 
            if (isMatched(matches, job)) continue; 

            int sid = job->nextPreferredServer();
            if (sid < 0) {
                job->markFailed(currentTime);
                continue; 
            }

            int sIdx = findServerIndexById(servers, sid);
            if (sIdx < 0) {
                job->advancePreference();
                continue;
            }

            proposals[sIdx].push_back(job);
            anyProposal = true;
        }

        if (!anyProposal) {
            break;
        }

        // 4.2 Each server handles this round's propose
        for (std::size_t si = 0; si < servers.size(); ++si) {
            Server* s = servers[si];
            if (!s) continue;

            auto& curMatches = matches[si];
            auto& newProps   = proposals[si];

            if (newProps.empty()) {
                continue;
            }

            // Candidates = matched + newly proposed
            std::vector<Job*> candidates;
            candidates.reserve(curMatches.size() + newProps.size());
            candidates.insert(candidates.end(), curMatches.begin(), curMatches.end());
            candidates.insert(candidates.end(), newProps.begin(), newProps.end());

            std::sort(candidates.begin(), candidates.end());
            candidates.erase(std::unique(candidates.begin(), candidates.end()),
                             candidates.end());

            // Server preference
            std::sort(candidates.begin(), candidates.end(),
                      [](Job* a, Job* b) {
                          return a->reportedDemand() < b->reportedDemand();
                      });

            // Choose subset
            std::vector<Job*> newMatches;
            newMatches.reserve(candidates.size());

            int remainingCap = s->freeCapacity(); 

            for (Job* job : candidates) {
                int d = job->reportedDemand(); 
                if (d <= remainingCap) {
                    newMatches.push_back(job);
                    remainingCap -= d;
                }
            }
            
            // As per server, current DA result is 'newMatches'
            // Jobs removed in previous curMatches is regarded as rejected, try next preference
            // Jobs not selected by proposal also needs to try next preference

            // record previous matches
            std::vector<Job*> oldMatches = curMatches;
            curMatches = newMatches;

            // jobs removed from old matches
            for (Job* job : oldMatches) {
                if (std::find(curMatches.begin(), curMatches.end(), job) == curMatches.end()) {
                    job->advancePreference();
                }
            }

            // Jobs not selected by proposal
            for (Job* job : newProps) {
                if (std::find(curMatches.begin(), curMatches.end(), job) == curMatches.end()) {
                    job->advancePreference();
                }
            }

            newProps.clear();
        }
    }

    // S5. When DA converges, writeback matches to server and mark corresponding jobs as Running
    for (std::size_t si = 0; si < servers.size(); ++si) {
        Server* s = servers[si];
        if (!s) continue;

        auto& curMatches = matches[si];
        for (Job* job : curMatches) {
            if (!job) continue;
            if (job->isWaiting()) {
                if (s->accept(job)) { 
                    job->markRunning(currentTime);
                    if (sim) sim->logJobStart(job, s, currentTime);
                } else {
                    job->markFailed(currentTime);
                }
            }
        }
    }
}
