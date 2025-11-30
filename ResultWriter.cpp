#include "ResultWriter.h"
#include <fstream>
#include <filesystem>

namespace ResultWriter {

void writeCSV(const std::string& filename,
              const std::string& configName,
              const std::string& schedulerType,
              unsigned int seed,
              const Metrics& m)
{
    bool newFile = !std::filesystem::exists(filename);

    std::ofstream fout(filename, std::ios::app);

    if (newFile) {
        fout << "seed,config,type,totalJobs,finishedJobs,avgCompletion,avgWaiting,util\n";
    }

    fout << seed << ","
         << configName << ","
         << schedulerType << ","
         << m.totalJobs << ","
         << m.finishedJobs << ","
         << m.avgCompletionTime << ","
         << m.avgWaitingTime << ","
         << m.avgUtilization << "\n";
}

}
