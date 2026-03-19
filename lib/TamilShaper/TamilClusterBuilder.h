#pragma once

#include <cstdint>
#include <vector>

#include "TamilCharacter.h"
#include "TamilCluster.h"

namespace TamilShaper {
class TamilClusterBuilder {
 public:
  static std::vector<TamilCluster> buildClusters(const char* text);
  static TamilCluster buildNextCluster(const uint8_t** text);
};

}  // namespace TamilShaper
