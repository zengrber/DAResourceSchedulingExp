#ifndef DA_SCHEDULER_H
#define DA_SCHEDULER_H

#include "Scheduler.h"

class DAScheduler : public Scheduler {
public:
    DAScheduler() = default;
    ~DAScheduler() override = default;

    void runBatch(std::vector<Job*>& jobs,
                  std::vector<Server*>& servers,
                  int currentTime) override;
};

#endif // DA_SCHEDULER_H
