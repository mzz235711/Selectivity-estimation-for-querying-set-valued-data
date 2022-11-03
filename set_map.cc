#include <boost/functional/hash.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <cctype>
#include <regex>
#include <unordered_map>
#include <ctime>
#include <stdlib.h>
#include <iostream>
#include <utility>
#include <limits>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "utils.h"

bool comp(std::pair<std::string,int> &a, std::pair<std::string,int> &b) {
    return a.second > b.second;
}

int BinarySearch(std::vector<int> &arr, int l, int r, int &x) {
  if (r >= l) {
    int mid = l + (r - l) / 2;
    if (arr[mid] == x) {
      return 1;
    } else if (arr[mid] > x) {
      return BinarySearch(arr, l, mid - 1, x);
    } else {
      return BinarySearch(arr, mid + 1, r, x);
    }
  }
  return 0;
}

void settrie_insert(TreeNode &root, std::vector<int> &set_key) {
  auto *curr_node = &root;
  for (auto &element : set_key) {
    std::vector<TreeNode>::iterator it;
    for (it = curr_node->children.begin(); it != curr_node->children.end(); it++) {
      if (it->id == element) {
	auto &new_node = *it;
	curr_node = &new_node;
	break;
      } else if (it->id > element) {
        TreeNode new_node;
	new_node.id = element;
	int index = it - curr_node->children.begin();
	curr_node->children.insert(it, new_node);
	curr_node = &(curr_node->children[index]);
	break;
      }
    }
    if (it == curr_node->children.end()) {
      TreeNode new_node;
      new_node.id = element;
      curr_node->children.push_back(new_node);
      curr_node = &(curr_node->children[curr_node->children.size() - 1]);
    }
  }
  curr_node->last_flag = true;
}

void dfs_addvalue(TreeNode &curr_node, int &set_value, Vectormap &setmap, std::vector<int> &curr_set, int &node_num, int level) {
  curr_node.range.first = set_value;
  curr_node.level = level;
  node_num++;
  if (curr_node.last_flag == true) {
    //if (set_map.find(curr_set) != set_map.end()) {
    //  auto set_value = set_map[curr_set];
    //results.push_back(curr_node.last_flag);
    //}
    setmap[curr_set] = set_value;
    set_value++;
  }
  for (auto &next_node : curr_node.children) {
    //auto next_set = curr_set;
    //next_set.push_back(next_node.id);
    auto next_set = curr_set;
    next_set.push_back(next_node.id);
    dfs_addvalue(next_node, set_value, setmap, next_set, node_num, level + 1);
  }
  curr_node.range.second = set_value;
}

void serialize(TreeNode &curr_node, std::ofstream &file) {
  file.write((char*)(&curr_node.level), sizeof(int));
  file.write((char*)(&curr_node.id), sizeof(int));
  file.write((char*)(&curr_node.last_flag), sizeof(bool));
  file.write((char*)(&curr_node.range.first), sizeof(int));
  file.write((char*)(&curr_node.range.second), sizeof(int));
  for (auto &next_node : curr_node.children) {
    serialize(next_node, file);
  }
}



int main(int argc, char** argv) {
  std::ifstream infile("../geotweet_tags.csv");
  std::string s;
  getline(infile, s);
  char delimiter = ',';
  std::regex e("[a-z]+");
  std::unordered_map<std::string, int> all_keywords;
  std::vector<std::vector<std::string>> texts;
  std::string pad = "ZZZ";
  int tmpidx = 0;
  while (getline(infile, s)) {
    //size_t pos = s.find(delimiter);
    //auto t = pos + delimiter.length();
    //size_t pos2 = s.find(delimiter, t);
    //std::string text = s.substr(pos + 1, pos2 - pos - 1);
    std::string text = s;
    if (s[0] == '"') {
      text = text.substr(1, text.size() - 2);
    }
    std::string keyword;
    //for(unsigned i = 0; i < text.size(); i++) {
    //  text[i] = std::tolower(text[i]);
    //}
    std::stringstream ss(text);
    std::vector<std::string> new_text;
    while (std::getline(ss, keyword, delimiter)) {
      if (std::find(new_text.begin(), new_text.end(), keyword) == new_text.end()) {
        if (all_keywords.find(keyword) == all_keywords.end()){
          all_keywords.emplace(keyword, 1);
        } else {
          all_keywords[keyword] += 1;
	}
        new_text.push_back(keyword);
      }
    }
    texts.push_back(new_text);
  }
  printf("file length %lu\n", texts.size());
  /*
  std::vector<std::pair<std::string, int>> freq_pair(all_keywords.begin(), all_keywords.end());
  std::sort(freq_pair.begin(), freq_pair.end(), comp);
  std::ofstream idxoutfile("idx.csv");
  for (int i = 0; i < freq_pair.size(); i++) {
    auto &k = freq_pair[i].first;
    auto &v = freq_pair[i].second;
    if (v >= 10) {
      idxoutfile << k << "\t" << i << "\n";
    }
  }
  idxoutfile.close();
  return 0;
  */
  std::vector<std::string> idx2word;
  std::unordered_map<std::string, int> word2idx;
  int index = 0;
  std::ifstream idxinfile("idx.csv");
  while (idxinfile >> s >> index) {
    word2idx.emplace(s, index);
    idx2word.push_back(s);
    if (s == "abcxyz") {
      std::cout << s << "\t" << index << "\t" << idx2word.size() << "\n";
    }
  }
  word2idx[pad] = idx2word.size();
  idx2word.push_back(pad);
  printf("Number of keyword: %lu\n", idx2word.size());
  std::ofstream freqfile("frequency.csv");
  for (auto &w : idx2word) {
    freqfile << w << "\t" << all_keywords[w] << "\n";
  }
  freqfile.close();
  std::vector<std::vector<int>> text_idx;
  for (auto &text : texts) {
    std::vector<int> new_text;
    for (auto t: text) {
      if (word2idx.find(t) != word2idx.end()) {
        new_text.push_back(word2idx[t]);
      }
    }
    std::sort(new_text.begin(), new_text.end());
    text_idx.push_back(new_text);
  }

  int v_num = idx2word.size();
  int partition_num = atoi(argv[1]);
  //std::string folder_name = "./graph_color";
  std::string folder_name = "color_partition_" + std::to_string(partition_num);
  //std::string partfilename = folder_name + "/approx_clique_part_" + std::to_string(partition_num) + ".txt";
  std::string partfilename = folder_name + "/part.txt";
  std::ifstream partfile(partfilename);
  std::vector<int> partition(v_num);
  int partid;
  int maxpartid = 0;
  for (int i = 0; i < v_num; i++) {
    partfile >> partid;
    partition[i] = partid;
    if (partid > maxpartid) {
      maxpartid = partid;
    }
  }
  //int partition_num = maxpartid + 1;
  std::cout << "cp1\n"; 
  std::vector<int> dis_num(partition_num, 0);
  std::vector<std::unordered_map<std::vector<int>, int, container_hash<std::vector<int>>>> setmap(
		  partition_num, std::unordered_map<std::vector<int>, int, container_hash<std::vector<int>>>());
  for (auto &idxs : text_idx) {
    std::vector<std::vector<int>> part_idx(partition_num, std::vector<int>());
    for (auto &idx : idxs) {
      partid = partition[idx];
      part_idx[partid].push_back(idx);
    }
    for (int i = 0; i < partition_num; i++) {
      // Add pad to each column
      if (part_idx[i].size() == 0) {
        part_idx[i].push_back(word2idx[pad]);
      }
      if (part_idx[i].size() != 0) {
        std::sort(part_idx[i].begin(), part_idx[i].end());
        if (setmap[i].find(part_idx[i]) == setmap[i].end()) {
          setmap[i][part_idx[i]] = dis_num[i];
	  dis_num[i]++;
        }
      }
    }
  } 
  
  std::vector<TreeNode> settrie(partition_num, TreeNode());
  std::vector<Vectormap> set_map(partition_num, Vectormap());
  std::string output_treename = folder_name  +"/settrie.txt";
  std::ofstream output_tree(output_treename, std::ios::out | std::ios::binary);
  for (int i = 0; i < partition_num; i++) {
    int node_num = 0;
    for (auto &k_v : setmap[i]) {
      auto set_key = k_v.first;
      settrie_insert(settrie[i], set_key);
    }
    int init_value = 0;
    std::vector<int> curr_set;
    dfs_addvalue(settrie[i], init_value, set_map[i], curr_set, node_num, 0);
    output_tree.write((char*)(&node_num), sizeof(int));
    std::cout << "node num " << i << " " << node_num << "\n";
    serialize(settrie[i], output_tree);
  }
  output_tree.close();
  std::cout << "cp2\n"; 
  std::string output_filename = folder_name + "/dis_set_map.txt";
  std::ofstream output_file(output_filename);
  for (int i = 0; i < partition_num; i++) {
    output_file << i << " " << set_map[i].size() << "\n";
    for (auto &k_v : set_map[i]) {
      auto &dis_set = k_v.first;
      auto dis_value = k_v.second;
      output_file << dis_set.size() << " ";
      for (auto idx : dis_set) {
        output_file << idx << " ";
      }
      output_file << dis_value << "\n";
    }
  }


  return 0;
}
