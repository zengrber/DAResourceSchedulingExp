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

    // try accept a job（基于 reported demand）
    bool canAccept(const Job* job) const;

    // accept a job
    bool accept(Job* job);

    // remove a job
    void remove(Job* job);

    // remove temporary matches
    void clearAssignments();

    // remove finished jobs
    void removeFinishedJobs(int currentTime, Simulation *sim);

private:
    int id_ = -1;

    int capacity_ = 0;
    int usedCapacity_ = 0;

    std::vector<Job*> assignedJobs_;
};

#endif // SERVER_H
