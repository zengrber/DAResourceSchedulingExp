#ifndef METRICS_H
#define METRICS_H

struct Metrics {
    int totalJobs = 0;
    int finishedJobs = 0;

    double avgCompletionTime = 0.0; // finishTime - arrivalTime
    double avgWaitingTime    = 0.0; // startTime  - arrivalTime

    double avgUtilization    = 0.0; // 非严格定义，简单近似

    void print() const;
};

#endif // METRICS_H
