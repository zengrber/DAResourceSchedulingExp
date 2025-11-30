#include "Server.h"
#include "Job.h"

bool Server::canAccept(const Job* job) const {
    return job && (job->reportedDemand() <= freeCapacity());
}

bool Server::accept(Job* job) {
    if (!canAccept(job)) return false;
    assignedJobs_.push_back(job);
    usedCapacity_ += job->trueDemand();
    return true;
}

void Server::remove(Job* job) {
    if (!job) return;
    for (std::size_t i = 0; i < assignedJobs_.size(); ++i) {
        if (assignedJobs_[i] == job) {
            usedCapacity_ -= job->reportedDemand();
            assignedJobs_.erase(assignedJobs_.begin() + i);
            break;
        }
    }
}

void Server::clearAssignments() {
    assignedJobs_.clear();
    usedCapacity_ = 0;
}

void Server::removeFinishedJobs(int currentTime) {
    std::vector<Job*> stillRunning;
    stillRunning.reserve(assignedJobs_.size());

    int freed = 0;

    for (Job* job : assignedJobs_) {
        if (!job) continue;

        if (job->isRunning()) {
            int elapsed = currentTime - job->startTime();
            if (elapsed >= job->duration()) {
                job->markFinished(currentTime);
                freed += job->trueDemand(); 
                continue; 
            }
        }

        stillRunning.push_back(job);
    }

    assignedJobs_.swap(stillRunning);
    usedCapacity_ -= freed;
}
