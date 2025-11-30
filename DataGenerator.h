#ifndef DATA_GENERATOR_H
#define DATA_GENERATOR_H

#include <vector>
#include <random>

class Job;
class Server;

// 用来统一控制数据分布的配置
struct GeneratorConfig {
    int numJobs          = 100;
    int numServers       = 5;

    int serverCapMin     = 10;   // 单个 server 最小容量
    int serverCapMax     = 20;   // 单个 server 最大容量

    int demandMin        = 1;    // job 真实需求下界
    int demandMax        = 10;   // job 真实需求上界

    int durationMin      = 1;    // 运行时间下界
    int durationMax      = 5;    // 运行时间上界

    int maxArrivalTime   = 20;   // job最大到达时间

    double misreportProb = 0.0;  // 谎报率 p
    double misreportAlpha= 0.0;  // 谎报幅度 α（0.5 表示 ±50%）

    unsigned int seed    = 42;   // 随机种子（保证可复现）
};

class DataGenerator {
public:
    explicit DataGenerator(const GeneratorConfig& cfg);

    // 生成 ground-truth jobs（只有 trueDemand/arrival/duration + 偏好）
    // 返回的是“基准版本”，后面基于它生成 truthful / strategic 拷贝
    void generateTrueJobs(std::vector<Job*>& outJobs) const;

    // 基于 trueJobs 生成一个“完全诚实上报”的版本
    void makeTruthfulCopy(const std::vector<Job*>& trueJobs,
                          std::vector<Job*>& outJobs) const;

    // 基于 trueJobs 生成一个“部分任务谎报”的版本
    void makeStrategicCopy(const std::vector<Job*>& trueJobs,
                           std::vector<Job*>& outJobs) const;

    // 生成服务器列表
    void generateServers(std::vector<Server*>& outServers) const;

private:
    GeneratorConfig cfg_;
};

#endif // DATA_GENERATOR_H
