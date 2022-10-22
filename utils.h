#include <vector>
#include <string>
#include <utility>
#include <unordered_map>
#include <unordered_set>

struct TreeNode {
  int level = -1;
  int id = -1;
  bool last_flag = false;
  int frequency = 0;
  int total_freq = 0;
  std::pair<int, int> range;
  int hist_label = -1;
  int hist_freq = 0;
  std::unordered_map<int, double> hist;
  std::vector<TreeNode> children;
};

struct VectorHasher {
    int operator()(const std::vector<int> &V) const {
        int hash = V.size();
        for(auto &i : V) {
            hash ^= i + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

typedef std::unordered_map<std::vector<int>, int, VectorHasher> Setmap;
typedef std::vector<std::unordered_set<int>> Graph;