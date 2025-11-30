#ifndef DA_SCHEDULER_H
#define DA_SCHEDULER_H

#include "Scheduler.h"
#include "Simulation.h"

class DAScheduler : public Scheduler {
public:
    DAScheduler() = default;
    ~DAScheduler() override = default;

    void runBatch(std::vector<Job*>& jobs,
                  std::vector<Server*>& servers,
                  int currentTime,
                  Simulation *sim) override;
};

#endif // DA_SCHEDULER_H
