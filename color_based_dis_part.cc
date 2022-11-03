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


template <typename Container> // we can make this generic for any container [1]
struct container_hash {
    std::size_t operator()(Container const& c) const {
        return boost::hash_range(c.begin(), c.end());
    }
};

typedef std::pair<int, int> pair_int;
struct pair_hash {
    template <class T1, class T2>
    size_t operator()(const std::pair<T1, T2>& p) const
    {
        auto hash1 = std::hash<T1>{}(p.first);
        auto hash2 = std::hash<T2>{}(p.second);

        if (hash1 != hash2) {
            return hash1 ^ hash2;
        }

        // If hash1 == hash2, their XOR is zero.
          return hash1;
    }
};

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

int main(int argc, char** argv) {
  std::ifstream infile("../geotweet_tags.csv");
  std::string s;
  getline(infile, s);
  char delimiter = ',';
  std::regex e("[a-z]+");
  std::unordered_map<std::string, int> all_keywords;
  std::vector<std::vector<std::string>> texts;
  int tmpidx = 0;
  while (getline(infile, s)) {
    //size_t pos = s.find(delimiter);
    //auto t = pos + delimiter.length();
    //size_t pos2 = s.find(delimiter, t);
    //std::string text = s.substr(pos + 1, pos2 - pos - 1);
    std::string text(s.begin() + 1, s.end() - 1);
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
  std::vector<std::string> idx2word;
  std::unordered_map<std::string, int> word2idx;
  int index = 0;
  std::ifstream idxinfile("idx.csv");
  while (idxinfile >> s >> index) {
    word2idx.emplace(s, index);
    idx2word.push_back(s);
    if (s == "abcxyz") {
      std::cout << s << "\t" << index << "\t" << idx2word.size() << "\n" << std::flush;
    }
  }
  printf("Number of keyword: %lu\n", idx2word.size());

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
  std::ifstream color_part_file("graph_color/part.txt");
  std::vector<int> color_partid(v_num);
  int p;
  int color_num = 0;
  for (int i = 0; i < v_num; i++) {
    color_part_file >> p;
    color_partid[i] = p;
    if (p > color_num) {
      color_num = p;
    }
  }
  color_num++;
  std::vector<int> color_dis_num(color_num, 0);
  std::vector<int> color_value(v_num);
  for (int i = 0; i < v_num; i++) {
    p = color_partid[i];
    color_dis_num[p]++;
    color_value[i] = color_dis_num[p];
  }
  std::vector<std::vector<int>> color_texts(color_num, std::vector<int>(texts.size(), 0));
  for (int i = 0; i < texts.size(); i++) {
    auto &text = text_idx[i];
    for (auto &t : text) {
      p = color_partid[t];
      int value = color_value[t];
      color_texts[p][i] = value;
    }
  }
  int partition_num = atoi(argv[1]);
  std::vector<int> centers;
  std::vector<int> partid(color_num, -1);
  std::vector<std::vector<int>> col_data(partition_num, std::vector<int>(texts.size()));
  std::vector<int> dis_num(partition_num);
  while (centers.size() < partition_num) {
    int randid = rand() % color_num;
    while (std::find(centers.begin(), centers.end(), randid) != centers.end()) {
      randid = rand() % v_num;
    }
    centers.push_back(randid);
    partid[randid] = centers.size() - 1;
    col_data[centers.size() - 1] = color_texts[randid];
    dis_num[centers.size() - 1] = color_dis_num[randid];
  }
  for (int i = 0; i < color_num; i++) {
    if (i % 20 == 0) {
      std::cout << i << "\n";
    }
    if (std::find(centers.begin(), centers.end(), i) != centers.end()) {
      continue;
    }
    std::vector<std::unordered_map<pair_int, int, pair_hash>> tmp_dis_values(partition_num, std::unordered_map<pair_int, int, pair_hash>());
    std::vector<std::vector<int>> tmp_col_data(partition_num, std::vector<int>(text_idx.size()));
    //std::cout << "cp1\n";
    //if (i > 0) {
    //  break;
    //}
    for (int j = 0; j < texts.size(); j++) {
      auto &text = color_texts[i][j];
      for (int k = 0; k < partition_num; k++) {
        //int new_value = text * dis_num[k] + col_data[k][j];
	pair_int new_value = std::make_pair(text, col_data[k][j]);
	//std::cout << text << " " << col_data[k][j] << "\n";
	if (tmp_dis_values[k].find(new_value) == tmp_dis_values[k].end()) {
	  tmp_dis_values[k][new_value] = tmp_dis_values[k].size();
	}
	tmp_col_data[k][j] = tmp_dis_values[k][new_value];
      }
    }
    int cid = -1;
    int min_increase = 2e30;
    //std::cout << "cp2\n";
    for (int j = 0; j < partition_num; j++) {
      //std::cout << tmp_dis_values[j].size() << " " << dis_num[j] << "\n";
      if ((tmp_dis_values[j].size() - dis_num[j]) < min_increase) {
        min_increase = tmp_dis_values[j].size() - dis_num[j];
	cid = j;
      }
    }
    //std::cout << cid << " cp3\n";
    dis_num[cid] = tmp_dis_values[cid].size();
    col_data[cid] = tmp_col_data[cid];
    /*
    for (int j = 0; j < texts.size(); j++) {
      pair_int new_value = std::make_pair(color_texts[i][j], col_data[cid][j]);
      if (tmp_dis_values[cid].find(new_value) == tmp_dis_values[cid].end()) {
	std::cout << "Not find\n";
        return 0;
      }
      col_data[cid][j] = tmp_dis_values[cid][new_value];
    }
    */
    partid[i] = cid;
  }
  std::string folderpath = "./color_partition_"  + std::to_string(partition_num);
  std::string partfilename = folderpath + "/part.txt";
  std::ofstream partfp(partfilename);
  for (int i = 0; i < v_num; i++) {
    partfp << partid[color_partid[i]] << "\n";
  }
  for (int i = 0; i < partition_num; i++) {
    std::cout << dis_num[i] << "\n" << std::flush;
  }
  partfp.close();
  return 0;
}
  

