#ifndef BASE_SCHEDULER_H
#define BASE_SCHEDULER_H

#include "Scheduler.h"
#include "Simulation.h"

// Simple Best-Fit on reportedDemand
// - For each Waiting job：
//   - Find server that satisfies freeCapacity >= reportedDemand and minimize 
//     (freeCapacity - reportedDemand) 
//   - If found, assign job to that server and mark job as 'Running'
//   - Otherwise mark the job 'failed'
class BaseScheduler : public Scheduler {
public:
    BaseScheduler() = default;
    ~BaseScheduler() override = default;

    void runBatch(std::vector<Job*>& jobs,
                  std::vector<Server*>& servers,
                  int currentTime,
                  Simulation *sim) override;
};

#endif 
