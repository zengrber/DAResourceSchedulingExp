#ifndef RESULT_WRITER_H
#define RESULT_WRITER_H

#include <string>
#include "Metrics.h"

namespace ResultWriter {
    
    void writeCSV(const std::string& filename,
                  const std::string& configName,
                  const std::string& schedulerType, // e.g. "base_truth"
                  unsigned int seed,
                  const Metrics& m);

}

#endif
