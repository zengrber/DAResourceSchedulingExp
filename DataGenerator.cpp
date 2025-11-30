#include "DataGenerator.h"
#include "Job.h"
#include "Server.h"

#include <algorithm>
#include <cmath>

DataGenerator::DataGenerator(const GeneratorConfig& cfg)
    : cfg_(cfg) {}

// 工具：生成 [lo, hi] 区间内的整数（若 lo > hi，则返回 fallback）
static int randIntInRange(std::mt19937& rng,
                          int lo, int hi,
                          int fallback) {
    if (lo > hi) return fallback;
    std::uniform_int_distribution<int> dist(lo, hi);
    return dist(rng);
}

void DataGenerator::generateTrueJobs(std::vector<Job*>& outJobs) const {
    outJobs.clear();
    outJobs.reserve(cfg_.numJobs);

    std::mt19937 rng(cfg_.seed);

    std::uniform_int_distribution<int> demandDist(cfg_.demandMin, cfg_.demandMax);
    std::uniform_int_distribution<int> durationDist(cfg_.durationMin, cfg_.durationMax);
    std::uniform_int_distribution<int> arrivalDist(0, cfg_.maxArrivalTime);

    for (int j = 0; j < cfg_.numJobs; ++j) {
        int trueDemand  = demandDist(rng);
        int duration    = durationDist(rng);
        int arrivalTime = arrivalDist(rng);

        // 简单偏好：对 server id 0..numServers-1 做一个随机打乱
        std::vector<int> prefs(cfg_.numServers);
        for (int s = 0; s < cfg_.numServers; ++s) {
            prefs[s] = s;
        }
        std::shuffle(prefs.begin(), prefs.end(), rng);

        // 这里先让 reportedDemand = trueDemand（ground truth 不关心报什么）
        Job* job = new Job(j,
                           trueDemand,
                           trueDemand,
                           duration,
                           arrivalTime,
                           prefs);
        outJobs.push_back(job);
    }
}

void DataGenerator::makeTruthfulCopy(const std::vector<Job*>& trueJobs,
                                     std::vector<Job*>& outJobs) const {
    outJobs.clear();
    outJobs.reserve(trueJobs.size());

    for (const Job* src : trueJobs) {
        if (!src) continue;

        Job* j = new Job(
            src->id(),
            src->trueDemand(),
            /*reportedDemand=*/src->trueDemand(),
            src->duration(),
            src->arrivalTime(),
            src->preferences()
        );
        outJobs.push_back(j);
    }
}

void DataGenerator::makeStrategicCopy(const std::vector<Job*>& trueJobs,
                                      std::vector<Job*>& outJobs) const {
    outJobs.clear();
    outJobs.reserve(trueJobs.size());

    std::mt19937 rng(cfg_.seed); // 保证每次调用生成相同的谎报模式

    std::bernoulli_distribution misreportCoin(cfg_.misreportProb);
    std::bernoulli_distribution underOverCoin(0.5); // 一半 under-report，一半 over-report

    for (const Job* src : trueJobs) {
        if (!src) continue;

        int id          = src->id();
        int trueDemand  = src->trueDemand();
        int duration    = src->duration();
        int arrivalTime = src->arrivalTime();
        auto prefs      = src->preferences();

        int reported = trueDemand;

        if (misreportCoin(rng) && cfg_.misreportAlpha > 0.0) {
            // 该任务选择谎报
            bool doUnder = underOverCoin(rng);

            if (doUnder) {
                // 下报：在 [(1-α)*true, true-1] 之间选，取整并限制在 [demandMin, demandMax]
                int lo = static_cast<int>(std::round((1.0 - cfg_.misreportAlpha) * trueDemand));
                int hi = trueDemand - 1;
                lo = std::max(lo, cfg_.demandMin);
                hi = std::max(lo, hi); // 保证 lo <= hi，否则 randIntInRange 会用 fallback

                reported = randIntInRange(rng, lo, hi, trueDemand);
            } else {
                // 上报：在 [true+1, (1+α)*true] 之间选
                int lo = trueDemand + 1;
                int hi = static_cast<int>(std::round((1.0 + cfg_.misreportAlpha) * trueDemand));
                lo = std::min(lo, cfg_.demandMax);
                hi = std::min(hi, cfg_.demandMax);
                hi = std::max(lo, hi);

                reported = randIntInRange(rng, lo, hi, trueDemand);
            }
        }

        Job* j = new Job(
            id,
            trueDemand,
            reported,
            duration,
            arrivalTime,
            prefs
        );
        outJobs.push_back(j);
    }
}

void DataGenerator::generateServers(std::vector<Server*>& outServers) const {
    outServers.clear();
    outServers.reserve(cfg_.numServers);

    std::mt19937 rng(cfg_.seed + 2025); // 跟 job 随机数稍微错开一下
    std::uniform_int_distribution<int> capDist(cfg_.serverCapMin, cfg_.serverCapMax);

    for (int s = 0; s < cfg_.numServers; ++s) {
        int cap = capDist(rng);
        Server* server = new Server(s, cap);
        outServers.push_back(server);
    }
}
