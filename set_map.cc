#include <vector>
#include <iostream>
#include <utility>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "utils.h"

void add_value(TreeNode &node, int &label, Setmap &setmap, std::vector<int> &curr_set, int &node_num) {
  node.range.first = label;
  node_num++;
  if (node.last_flag == true) {
    setmap[curr_set] = label;
    label++;
  }
  for (auto &child : node.children) {
    auto next_set = curr_set;
    next_set.push_back(child.id);
    add_value(child, label, setmap, next_set, node_num);
  }
  node.range.second = label;
}

void insert_node(TreeNode &node, std::vector<int> &set_vector, int index, int level) {
  node.level = level;
  if (index == set_vector.size()) {
    node.last_flag = true;
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
  outfile.write((char*)&node.range.first, sizeof(int));
  outfile.write((char*)&node.range.second, sizeof(int)); 
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
  std::string folder_name = "./color_partition_" + std::to_string(partition_num);
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
    int label = 0;
    std::vector<int> curr_set;
    add_value(settrie[i], label, setmaps[i], curr_set, node_num[i]);
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

  std::ofstream seriafile(folder_name + "/settrie.txt", std::ios::app | std::ios::binary);
  for (int i = 0; i < partition_num; i++) {
    seriafile.write((char*)&node_num[i], sizeof(int));
    serialize(settrie[i], seriafile);
  }
  return 0;
}