#ifndef JOB_H
#define JOB_H

#include <vector>
#include <cstddef> // std::size_t

enum class JobState {
    Waiting,
    Running,
    Finished
};

class Job {
public:
    Job() = default;

    Job(int id,
        int trueDemand,
        int reportedDemand,
        int duration,
        int arrivalTime,
        const std::vector<int>& serverPrefs)
        : id_(id),
          trueDemand_(trueDemand),
          reportedDemand_(reportedDemand),
          duration_(duration),
          arrivalTime_(arrivalTime),
          prefs_(serverPrefs),
          state_(JobState::Waiting),
          nextPrefIndex_(0),
          startTime_(-1),
          finishTime_(-1) {}

    // --- basic getters ---
    int id() const { return id_; }

    int trueDemand() const { return trueDemand_; }
    int reportedDemand() const { return reportedDemand_; }

    int duration() const { return duration_; }
    int arrivalTime() const { return arrivalTime_; }

    JobState state() const { return state_; }
    int startTime() const { return startTime_; }
    int finishTime() const { return finishTime_; }

    const std::vector<int>& preferences() const { return prefs_; }

    // --- setters / state updates ---
    void setReportedDemand(int d) { reportedDemand_ = d; }

    void markRunning(int currentTime) {
        state_ = JobState::Running;
        if (startTime_ < 0) {
            startTime_ = currentTime;
        }
    }

    void markFinished(int currentTime) {
        state_ = JobState::Finished;
        finishTime_ = currentTime;
    }

    bool isFinished() const { return state_ == JobState::Finished; }
    bool isWaiting() const { return state_ == JobState::Waiting; }
    bool isRunning() const { return state_ == JobState::Running; }


    // --- DA-related helpers ---

    // current proposed server id, if there is nothing left, return -1
    int nextPreferredServer() const {
        if (nextPrefIndex_ >= prefs_.size()) {
            return -1;
        }
        return prefs_[nextPrefIndex_];
    }

    // after success/failed propose, move index
    void advancePreference() {
        if (nextPrefIndex_ < prefs_.size()) {
            ++nextPrefIndex_;
        }
    }

    void resetPreferencesIter() {
        nextPrefIndex_ = 0;
    }
    
    int waitingTime() const {
        if (finishTime_ < 0) return -1;
        return finishTime_ - arrivalTime_;
    }

private:
    int id_ = -1;

    int trueDemand_ = 0;     
    int reportedDemand_ = 0;

    int duration_ = 0;       
    int arrivalTime_ = 0;    

    std::vector<int> prefs_; 

    JobState state_ = JobState::Waiting;

    std::size_t nextPrefIndex_ = 0;

    int startTime_ = -1;
    int finishTime_ = -1;
};

#endif // JOB_H
