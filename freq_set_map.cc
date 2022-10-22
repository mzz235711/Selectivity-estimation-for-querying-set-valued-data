#include <vector>
#include <iostream>
#include <utility>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "utils.h"

void add_freq(TreeNode &node, std::unordered_map<int, double> &hist,
              int &label, Setmap &setmap, std::vector<int> &curr_set) {
  if (node.last_flag) {
    setmap[curr_set] = label;
  }
  if (hist.find(node.id) == hist.end()) {
    hist[node.id] = node.total_freq;
  } else {
    hist[node.id] += node.total_freq;
  }
  for (auto &child : node.children) {
    auto next_set = curr_set;
    add_freq(child, hist, label, setmap, next_set);
  }
}

void construct_hist(TreeNode &node, int &min_freq,
       int &label, Setmap &setmap, std::vector<int> &curr_set, int &node_num) {
  node_num++;
  node.range.first = label;
  curr_set.push_back(node.id);
  if (node.last_flag) {
    setmap[curr_set] = label;
    label++;
  }
  for (auto &child : node.children) {
    if (child.total_freq <= min_freq && child.level > 1) {
      node.hist_freq += child.total_freq;
      auto next_set = curr_set;
      add_freq(child, node.hist, label, setmap, next_set);
    }
  }
  if (node.hist.size() > 0) {
    node.hist_label = label;
    label++;
  }
  for (auto &k_v : node.hist) {
    auto &k = k_v.first;
    auto &v = k_v.second;
    v = k / node.hist_freq;
  }
  for (auto &child : node.children) {
    if (child.total_freq > min_freq || child.level <= 1) {
      auto next_set = curr_set;
      construct_hist(child, min_freq, label, setmap, next_set, node_num);
    }
  }
  node.range.second = label;
}

void get_freq(TreeNode &node, std::vector<int> &all_freq) {
  if (node.level > 1) {
    all_freq.push_back(node.total_freq);
  }
  for (auto &child : node.children) {
    get_freq(child, all_freq);
  }
}

void insert_node(TreeNode &node, std::vector<int> &set_vector, int index, int level) {
  node.level = level;
  node.total_freq += 1;
  if (index == set_vector.size()) {
    node.last_flag = true;
    node.frequency += 1;
    return;
  }
  auto &w = set_vector[index];
  int i;
  for (i = 0; i < node.children.size(); i++) {
    auto &child = node.children[i];
    if (w < child.id) {
      node.children.insert(node.children.begin() + i, TreeNode());
      node.children[i].id = w;
      auto &new_node = node.children[i];
      insert_node(new_node, set_vector, index + 1, level + 1);
      break;
    } else if (w == child.id) {
      insert_node(child, set_vector, index + 1, level + 1);
      break;
    }
  }
  if (i == node.children.size()) {
    node.children.push_back(TreeNode());
    auto &new_node = node.children[node.children.size() - 1]; 
    new_node.id = w;
    insert_node(new_node, set_vector, index + 1, level + 1);
  }
}

void serialize(TreeNode &node, std::ofstream &outfile) {
  outfile.write((char*)&node.level, sizeof(int));
  outfile.write((char*)&node.id, sizeof(int));
  outfile.write((char*)&node.last_flag, sizeof(bool));
  outfile.write((char*)&node.frequency, sizeof(int));
  outfile.write((char*)&node.total_freq, sizeof(int));
  outfile.write((char*)&node.range.first, sizeof(int));
  outfile.write((char*)&node.range.second, sizeof(int));
  outfile.write((char*)&node.hist_label, sizeof(int));
  size_t hist_size = node.hist.size();
  outfile.write((char*)&hist_size, sizeof(size_t));
  for (auto &k_v : node.hist) {
    outfile.write((char*)&k_v.first, sizeof(int));
    outfile.write((char*)&k_v.second, sizeof(double));
  }
  for (auto &child : node.children) {
    serialize(child, outfile);
  }
}

int main(int argc, char **argv) {
  std::ifstream dataset("gn_feature.csv"); 
  std::vector<std::vector<std::string>> texts;
  std::string line;
  std::getline(dataset, line);
  std::unordered_map<std::string, int> frequency;
  std::string pad = "ZZZ";
  while (std::getline(dataset, line)) {
    if (line[0] == '"') {
      line = line.substr(1, line.size() - 2);
    }
    std::istringstream ss(line);
    std::string token;
    std::vector<std::string> text;  
    while (std::getline(ss, token, ',')) {
      if (std::find(text.begin(), text.end(), ss) == text.end()) {
        text.push_back(token);
        if (frequency.find(token) == frequency.end()) {
          frequency[token] = 0;
        } else {
          frequency[token] += 1;
        }
      }
    }
  }
  std::ifstream idxfile("idx.csv");
  std::vector<std::string> idx2word;
  std::unordered_map<std::string, int> word2idx;
  std::string word;
  int idx;
  while (idxfile >> word >> idx) {
    idx2word.push_back(word);
    word2idx[word] = idx;
  }
  idx2word.push_back(pad);
  word2idx[pad] = idx2word.size() - 1;

  std::vector<std::vector<int>> textidxes;
  for (auto &text : texts) {
    std::vector<int> textidx;
    for (auto &t : text) {
      if (word2idx.find(t) != word2idx.end()) {
        textidx.push_back(word2idx[t]);
      }
    }
    std::sort(textidx.begin(), textidx.end());
    textidxes.push_back(textidx);
  }

  int partition_num = atoi(argv[1]);
  int keep_num = atoi(argv[2]);
  std::string folder_name = "./partition_" + std::to_string(partition_num) + "_" + std::to_string(keep_num);
  std::ifstream partfile(folder_name + "/part.txt");
  std::vector<int> partid;
  int p;
  while (partfile >> p) {
    partid.push_back(p);
  }
  std::vector<TreeNode> settrie(partition_num, TreeNode());
  for (auto &text : textidxes) {
    std::vector<std::vector<int>> set_vectors(partition_num);
    for (auto &t : text) {
      int pid = partid[t];
      set_vectors[pid].push_back(t);
    }
    for (int i = 0; i < partition_num; i++) {
      if (set_vectors[i].size() == 0) {
        set_vectors[i].push_back(word2idx[pad]);
      }
      insert_node(settrie[i], set_vectors[i], 0, 0);
    }
  }

  std::vector<Setmap> setmaps(partition_num, Setmap());
  std::vector<int> node_num(partition_num, 0);
  for (int i = 0; i < partition_num; i++) {
    std::vector<int> all_freq;
    get_freq(settrie[i], all_freq);
    std::sort(all_freq.begin(), all_freq.end());
    int min_freq;
    if (keep_num >= all_freq.size()) {
      min_freq = 0;
    } else {
      min_freq = all_freq[all_freq.size() - keep_num];
    }
    int label = 0;
    std::vector<int> curr_set;
    construct_hist(settrie[i], min_freq, label, setmaps[i], curr_set, node_num[i]);
  }

  std::ofstream outfile(folder_name + "/dis_setmap.txt");
  for (int i = 0; i < partition_num; i++) {
    outfile << i << " " << setmaps[i].size() << "\n";
    for (auto &k_v : setmaps[i]) {
      auto &key = k_v.first;
      auto &value = k_v.second;
      outfile << key.size() << " ";
      for (auto &k : key) {
        outfile << k << " ";
      }
      outfile << value << "\n";
    }
  }
  outfile.close();

  std::ofstream seriafile(folder_name + "/settrie.txt", std::ios::app|std::ios::binary);
  for (int i = 0; i < partition_num; i++) {
    seriafile.write((char*)&node_num[i], sizeof(int));
    serialize(settrie[i], seriafile);
  }
  seriafile.close();
  return 0;

}