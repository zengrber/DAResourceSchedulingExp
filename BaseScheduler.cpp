#include "BaseScheduler.h"
#include "Job.h"
#include "Server.h"
#include "Simulation.h"

#include <climits>   // INT_MAX
#include <cstddef>   // std::size_t

// Simple Best-Fit on reportedDemand
// - For each Waiting job：
//   - Find server that satisfies freeCapacity >= reportedDemand and minimize 
//     (freeCapacity - reportedDemand) 
//   - If found, assign job to that server and mark job as 'Running'
//   - Otherwise mark the job 'failed'
void BaseScheduler::runBatch(std::vector<Job*>& jobs,
                             std::vector<Server*>& servers,
                             int currentTime,
                             Simulation* sim)
{
    // try assign every single job
    for (Job* job : jobs) {
        if (!job) continue;
        if (!job->isWaiting()) continue; 

        int demand = job->reportedDemand();
        if (demand <= 0) {
            continue;
        }

        Server* bestServer = nullptr;
        int bestSlack = INT_MAX; // freeCapacity - demand

        for (Server* server : servers) {
            if (!server) continue;

            int slack = server->freeCapacity() - demand;
            if (slack >= 0 && slack < bestSlack) {
                bestSlack = slack;
                bestServer = server;
            }
        }

        // find capable server
        if (bestServer != nullptr) {
            if (bestServer->accept(job)) {
                job->markRunning(currentTime);
                if (sim) sim->logJobStart(job, bestServer, currentTime);
            }  else {
                    job->markFailed(currentTime);
            }
        }
    }
}
