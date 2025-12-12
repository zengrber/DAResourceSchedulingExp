#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "Simulation.h"

#include <vector>

class Job;
class Server;

class Scheduler {
public:
    virtual ~Scheduler() = default;

    // Schedule a batch：
    //  - jobs: waiting job pointers
    //  - servers: all servers
    //  - currentTime: current time, use to mark job's state
    //
    // Specific schedulers are implemented in other child classes
    virtual void runBatch(
        std::vector<Job*>& jobs,
        std::vector<Server*>& servers,
        int currentTime,
        Simulation *sim
    ) = 0;
};

#endif // SCHEDULER_H
