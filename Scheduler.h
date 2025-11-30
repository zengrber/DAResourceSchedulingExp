#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>

class Job;
class Server;

class Scheduler {
public:
    virtual ~Scheduler() = default;

    // 在一个批次上做调度决策：
    //  - jobs: 当前参与调度的 job 指针（通常是 waiting 且已到达的）
    //  - servers: 所有 server
    //  - currentTime: 当前时间，用于标记 job 开始/结束等
    //
    // 具体的调度算法（baseline / DA）在子类里实现。
    virtual void runBatch(
        std::vector<Job*>& jobs,
        std::vector<Server*>& servers,
        int currentTime
    ) = 0;
};

#endif // SCHEDULER_H
