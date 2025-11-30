#include "Metrics.h"
#include <iostream>

void Metrics::print() const {
    std::cout << "Total jobs: "        << totalJobs       << "\n"
              << "Finished jobs: "     << finishedJobs    << "\n"
              << "Avg completion time: " << avgCompletionTime << "\n"
              << "Avg waiting time: "    << avgWaitingTime    << "\n"
              << "Approx avg utilization: " << avgUtilization << "\n";
}
