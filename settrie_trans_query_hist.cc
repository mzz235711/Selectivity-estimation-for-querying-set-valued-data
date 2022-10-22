#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>

#include "utils.h"


void subset_search(TreeNode &node, std::vector<int> &query, int index, std::vector<int> &result,
     double est_freq, int &total_freq, std::vector<double> &percent) {
  if (node.last_flag == true) {
    result.push_back(node.range.first);
    est_freq += node.frequency;
    total_freq += node.frequency;
    percent.push_back(1);
  }
  if (index == query.size()) {
    return;
  }
  if (node.hist.size() > 0) {
    double p = 1;
    for (auto &k_v : node.hist) {
      auto &key = k_v.first;
      auto &value = k_v.second;
      if (std::find(query.begin() + index, query.end(), key) == query.end()) {
        p *= (1 - value);
      }
    }
    if (p > 0.001) {
      result.push_back(node.hist_label);
      est_freq += (p * node.hist_freq);
      total_freq += node.hist_freq;
      percent.push_back(p);
    }
  }
  for (int i = 0; i < node.children.size(); i++) {
    auto &child = node.children[i];
    if (child.id == query[index]) {
      subset_search(child, query, index + 1, result, est_freq, total_freq, percent);
    } else if (child.id > query[index]) {
      index++;
      i--;
    }
  }
}

void superset_search(TreeNode &node, std::vector<int> &query, int index, 
    std::vector<int> &result, double &est_freq, int &total_freq, std::vector<double> &percent) {
  if (index == query.size()) {
    result.push_back(node.range.first);
    result.push_back(node.range.second);
    est_freq += node.total_freq;
    total_freq += node.total_freq;
    percent.push_back(1);
    return;
  } else if (node.hist.size() > 0) {
    double p = 1;
    for (int i = index; i < query.size(); i++) {
      if (node.hist.find(query[i]) == node.hist.end()) {
        p = 0;
        break;
      } else {
        p *= node.hist[query[i]];
      }
    }
    if (p > 0.001) {
      result.push_back(node.hist_label);
      result.push_back(node.hist_label + 1);
      est_freq += (p * node.hist_freq);
      total_freq += node.hist_freq;
      percent.push_back(p);
    }
  }
  int from;
  if (index == 0) {
    from = -1;
  } else {
    from = query[index - 1];
  }
  int upto = query[index];
  for (int i = 0; i < node.children.size(); i++) {
    auto &child = node.children[i];
    if (child.id < upto) {
      superset_search(child, query, index, result, est_freq, total_freq, percent);
    } else if (child.id = upto) {
      superset_search(child, query, index + 1, result, est_freq, total_freq, percent);
    } else {
      break;
    }
  }
}

void deserialize_node(TreeNode &node, std::ifstream &infile) {
  infile.read((char*)&node.level, sizeof(int));
  infile.read((char*)&node.id, sizeof(int));
  infile.read((char*)&node.last_flag, sizeof(bool));
  infile.read((char*)&node.frequency, sizeof(int));
  infile.read((char*)&node.total_freq, sizeof(int));
  infile.read((char*)&node.range.first, sizeof(int));
  infile.read((char*)&node.range.second, sizeof(int));
  infile.read((char*)&node.hist_label, sizeof(int));
  size_t hist_size;
  infile.read((char*)&hist_size, sizeof(size_t));
  for (size_t i = 0; i < hist_size; i++) {
    int key;
    double value;
    infile.read((char*)&key, sizeof(int));
    infile.read((char*)&value, sizeof(double));
    node.hist[key] = value;
  }
}

void deserialize(TreeNode &root, std::ifstream &infile, int &node_num) {
  deserialize_node(root, infile);
  TreeNode *curr_node_pt = &root;
  for (int i = 0; i < node_num - 1; i++) {
    TreeNode new_node;
    deserialize_node(new_node, infile);
    if (curr_node_pt->level + 1 == new_node.level) {
      curr_node_pt->children.push_back(new_node);
      curr_node_pt = &(curr_node_pt->children[curr_node_pt->children.size() - 1]);
    } else {
      curr_node_pt = &root;
      while (curr_node_pt->level < new_node.level - 1) {
        curr_node_pt = &(curr_node_pt->children[curr_node_pt->children.size() - 1]);
      }
      curr_node_pt->children.push_back(new_node);
      curr_node_pt = &(curr_node_pt->children[curr_node_pt->children.size() - 1]);
    }
  }

}

int main(int argc, char **argv) {
  std::string pad = "ZZZ";
  std::ifstream idxfile("idxfile");
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
  
  int partition_num = atoi(argv[1]);
  int keep_num = atoi(argv[2]);
  std::string folder_name = "./partition_" + std::to_string(partition_num) 
       + "_" + std::to_string(keep_num);
  std::ifstream partfile(folder_name + "/part.txt");
  std::vector<int> partid;
  int p;
  while (partfile >> p) {
    partid.push_back(p);
  }
  std::vector<TreeNode> settrie(partition_num, TreeNode());
  std::ifstream settriefile(folder_name + "/settrie.txt");
  for (int i = 0; i < partition_num; i++) {
    int node_num;
    settriefile.read((char*)&node_num, sizeof(int));
    deserialize(settrie[0], settriefile, node_num);
  }

  std::unordered_map<int, std::string> Q_TYPE={{0, "subset"}, {1, "superset"}, {2, "all"}};
  std::string setname = "gn.FEATURE_NAME";
  std::unordered_map<std::string, int> col_type={{"gn.FEATURE_TYPE", 0}};
  int query_type = atoi(argv[2]);
  std::ifstream queryfile("gn_query_5.csv");
  std::ofstream csvfile(folder_name + "/gn_query_5_settrie_trans_query_" + Q_TYPE[query_type] + "hist.csv");
  std::ofstream narupercentfile(folder_name + "/gn_query_5_settrie_trans_query_" + Q_TYPE[query_type] + "_hist_narupercent.csv");
  std::ofstream sqlfile(folder_name + "/gn_query_5_settrie_trans_query_" + Q_TYPE[query_type] + ".sql");
  std::string line;
  int lineid = 0;
  int query_count = 0;
  while (std::getline(queryfile, line)) {
    if (lineid % 2 != query_type) {
      lineid++;
      continue;
    }
    bool has_subset = false;
    lineid++;
    query_count++;
    std::stringstream liness(line);
    std::string sqlstr = "SELECT COUNT(*) FROM gn_settrie_" + std::to_string(partition_num)
                           + "AS gn WHERE "; 
    std::string csvstr = "gn##";
    std::string narustr = "";
    std::string token, col, op, val, card;
    std::getline(liness, token, '#');
    std::getline(liness, token, '#');
    std::getline(liness, token, '#');
    std::getline(liness, card, '\n');
    std::stringstream predicates(token);
    double total_percent = 1;
    while (std::getline(predicates, col, ',')) {
      std::getline(predicates, op, ',');
      std::getline(predicates, val, ',');
      if (col_type.find(col) != col_type.end()) {
        csvstr += (col + "," + op + "," + val + ",");
        if (col_type[col] == 1) {
          sqlstr += (col + op + val + " AND ");
        } else {
          sqlstr += (col + op + "\"" + val + "\"" + " AND ");
        }
      } else {
        std::stringstream words(val);
        std::string word;
        std::vector<std::vector<int>> query_words(partition_num, std::vector<int>());
        while(std::getline(words, word, '|')) {
          int idx = word2idx[word];
          int pid = partid[idx];
          query_words[pid].push_back(idx);
        }
        for (int i = 0; i < partition_num; i++) {
          std::vector<int> result;
          double est_freq;
          int total_freq;
          std::vector<double> percent;
          std::string currname = setname + std::to_string(i);
          if (op == "<@") {
            has_subset = true;
            if (query_words[i].size() > 0) {
              subset_search(settrie[i], query_words[i], 0, result, est_freq, total_freq, percent);
            }
            result.push_back(settrie[i].children[settrie[i].children.size() - 1].id);
            total_percent *= (est_freq / total_freq);
            if (result.size() == 1) {
              csvstr += (currname + ",=," + std::to_string(result[0]) + ",");
              sqlstr += (currname + "=" + std::to_string(result[0]) + " AND ");
            } else {
              csvstr += (currname + ",IN,\"(");
              sqlstr += (currname + " IN (");
              for (auto &r : result) {
                csvstr += (std::to_string(r) + ",");
                sqlstr += (std::to_string(r) + ",");
              }
              csvstr = csvstr.substr(0, csvstr.size() - 1) + ")\",";
              sqlstr = sqlstr.substr(0, sqlstr.size() - 1) + ") AND ";
              narustr += currname + ":";
              for (auto &p : percent) {
                narustr += (std::to_string(p) + ",");
              }
              narustr = narustr.substr(0, narustr.size() - 1) + ";";
            }
          } else {
            if (query_words[i].size() > 0) {
              superset_search(settrie[i], query_words[i], 0, result, est_freq, total_freq, percent);
              if (result.size() == 2 && result[0] + 1 == result[1]) {
                csvstr += (currname + ",=," + std::to_string(result[0]) + ",");
                sqlstr += (currname + " = " + std::to_string(result[0]) + " AND ");
                narustr += (currname + ":" + std::to_string(percent[0]) + ";");
              } else if (result.size() > 0) {
                csvstr += (currname + ",IN,\"(");
                sqlstr += ("(");
                narustr += (currname + ":");
                for (int i = 0; i < result.size(); i++) {
                  auto &lb = result[i];
                  auto &ub = result[i + 1];
                  auto &p = percent[i / 2];
                  sqlstr += ("(" + currname + ">=" + std::to_string(lb) + " AND "
                             + currname + "<" + std::to_string(ub) + ") OR ");
                  for (int j = lb; j < ub; j++) {
                    csvstr += (std::to_string(j) + ",");
                    narustr += (std::to_string(p) + ",");
                  }
                }
                sqlstr = sqlstr.substr(0, sqlstr.size() - 4) + " AND ";
                csvstr = csvstr.substr(0, csvstr.size() - 1) + ")\",";
                narustr = narustr.substr(0, narustr.size() - 1) + ";";
              }
            }
          }
        }
      }
    } 
    if (has_subset) {
      csvstr += (setname + "_lfword,=,0");
      sqlstr += (setname + "_lfword=0;");
    } else {
      csvstr = csvstr.substr(0, csvstr.size() - 1);
      sqlstr = sqlstr.substr(0, sqlstr.size() - 5) + ";";
    }
    csvfile << csvstr << "\n";
    sqlfile << sqlstr << std::to_string(total_percent) << "\n";
    narupercentfile << narustr << "\n";
  }
  csvfile.close();
  sqlfile.close();
  narupercentfile.close();
  return 0;
}