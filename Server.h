#ifndef SERVER_H
#define SERVER_H

#include "Simulation.h"

#include <vector>
#include <cstddef> // std::size_t

class Job; 

class Server {
public:
    Server() = default;

    Server(int id, int capacity)
        : id_(id),
          capacity_(capacity),
          usedCapacity_(0) {}

    int id() const { return id_; }

    int capacity() const { return capacity_; }
    int usedCapacity() const { return usedCapacity_; }
    int freeCapacity() const { return capacity_ - usedCapacity_; }

    const std::vector<Job*>& assignedJobs() const { return assignedJobs_; }

    // 尝试接收一个 job（基于 reported demand）
    bool canAccept(const Job* job) const;

    // 实际接受：修改 usedCapacity_，加入 assignedJobs_
    bool accept(Job* job);

    // 从当前分配中移除一个 job（DA 调整时会用到）
    void remove(Job* job);

    // 清空当前批次临时匹配（开始新一轮 DA 或新批次）
    void clearAssignments();

    // 清除当前已经完成的作业
    void removeFinishedJobs(int currentTime, Simulation *sim);

private:
    int id_ = -1;

    int capacity_ = 0;
    int usedCapacity_ = 0;

    // 在调度器里会只存指针，不拥有 Job 的生命周期
    std::vector<Job*> assignedJobs_;
};

#endif // SERVER_H
