#include <boost/functional/hash.hpp>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stdlib.h>
#include <utility>

//typedef std::vector<std::vector<std::pair<int, int>>> Weightedgraph;
//typedef std::vector<std::unordered_map<int, int>> Subgraph;

struct TreeNode{
  int level = 0;
  int id = -1;
  bool last_flag = false;
  int freq = 0;
  int total_freq = 0;
  int hist_freq = 0;
  std::pair<int, int> range = std::make_pair(-1, -1);
  int hist_val = -1;
  std::vector<TreeNode> children;
  std::unordered_map<int, double> percentage;
  std::vector<int> full_nodes;
  double all_null = 1;
};



template<typename Container> // we can make this generic for any container [1]
struct container_hash {
    std::size_t operator()(Container const& c) const {
        return boost::hash_range(c.begin(), c.end());
    }
};


template<class InputIterator, class T>
InputIterator find_pair(InputIterator first, InputIterator last, const T& val) {
  while (first != last) {
    if ((*first).first == val) return first;
    ++first;
  }
  return last;
}

template<class InputIterator, class T>
InputIterator find_node(InputIterator first, InputIterator last, const T& val) {
  while (first != last) {
    if ((*first).id == val) return first;
    ++first;
  }
  return last;
}


typedef std::vector<std::unordered_map<int, int>> Weightedgraph;
typedef std::vector<std::vector<int>> Unweightedgraph;
typedef std::vector<std::pair<std::vector<int>, std::vector<int>>> UnweightedDirectgraph;
//typedef std::vector<std::vector<int>> Weightedgraph;
typedef std::unordered_map<std::vector<int>, int, container_hash<std::vector<int>>> Vectormap;
typedef std::vector<std::vector<int>> Setvector;
typedef std::vector<std::vector<int>> Adjmatrix;

const int FREQ_THRES = 10;
