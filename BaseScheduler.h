#ifndef BASE_SCHEDULER_H
#define BASE_SCHEDULER_H

#include "Scheduler.h"

// 一个简单的基线调度器：
// 对当前批次中所有处于 Waiting 状态的 job，
// 用 best-fit（剩余容量最小但仍能容纳）的策略选择 server。
class BaseScheduler : public Scheduler {
public:
    BaseScheduler() = default;
    ~BaseScheduler() override = default;

    void runBatch(std::vector<Job*>& jobs,
                  std::vector<Server*>& servers,
                  int currentTime) override;
};

#endif 
