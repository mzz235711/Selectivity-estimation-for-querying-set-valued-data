#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>

#include "utils.h"


void subset_search(TreeNode &node, std::vector<int> &query, int index, std::vector<int> &result) {
  if (node.last_flag == true) {
    result.push_back(node.range.first);
  }
  if (index == query.size()) {
    return;
  }
  for (int i = 0; i < node.children.size(); i++) {
    auto &child = node.children[i];
    if (child.id == query[index]) {
      subset_search(child, query, index + 1, result);
    } else if (child.id > query[index]) {
      index++;
      i--;
    }
  }
}

void superset_search(TreeNode &node, std::vector<int> &query, int index, std::vector<int> &result) {
  if (index == query.size()) {
    result.push_back(node.range.first);
    result.push_back(node.range.second);
    return;
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
      superset_search(child, query, index, result);
    } else if (child.id == upto) {
      superset_search(child, query, index + 1, result);
    } else {
      break;
    }
  }
}

void deserialize_node(TreeNode &node, std::ifstream &infile) {
  infile.read((char*)&node.level, sizeof(int));
  infile.read((char*)&node.id, sizeof(int));
  infile.read((char*)&node.last_flag, sizeof(bool));
  infile.read((char*)&node.range.first, sizeof(int));
  infile.read((char*)node.range.second, sizeof(int));
}

void deserialize(TreeNode &root, std::ifstream &infile, int &node_num) {
  deserialize_node(root, infile);
  TreeNode* curr_node_pt = &root;
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
  std::string folder_name = "./color_partition_" + std::to_string(partition_num);
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
  std::ofstream csvfile(folder_name + "/gn_query_5_settrie_trans_query_" + Q_TYPE[query_type] + ".csv");
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
    std::string token, col, op, val, card;
    std::getline(liness, token, '#');
    std::getline(liness, token, '#');
    std::getline(liness, token, '#');
    std::getline(liness, card, '\n');
    std::stringstream predicates(token);
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
          std::string currname = setname + std::to_string(i);
          if (op == "<@") {
            has_subset = true;
            if (query_words[i].size() > 0) {
              subset_search(settrie[i], query_words[i], 0, result);
            }
            result.push_back(settrie[i].children[settrie[i].children.size() - 1].range.first);
            if (result.size() == 1) {
              csvstr += (currname + ",=," + std::to_string(result[0]) + ",");
              sqlstr += (currname + " = " + std::to_string(result[0]) + " AND ");
            } else {
              csvstr += (currname + ",IN,\"(");
              sqlstr += (currname + " IN (");
              for (auto &r : result) {
                csvstr += (std::to_string(r) + ",");
                sqlstr += (std::to_string(r) + ",");
              }
              csvstr = csvstr.substr(0, csvstr.size() - 1) + ")\",";
              sqlstr = sqlstr.substr(0, sqlstr.size() - 1) + ") AND ";
            }
          } else {
            if (query_words[i].size() > 0) {
              superset_search(settrie[i], query_words[i], 0, result);
              if (result.size() == 2 && result[0] + 1 == result[1]) {
                csvstr += (currname + ",=," + std::to_string(result[0]) + ",");
                sqlstr += (currname + " = " + std::to_string(result[0]) + " AND ");
              } else if (result.size() > 0) {
                csvstr += (currname + ",IN,\"(");
                sqlstr += ("(");
                for(int i = 0; i < result.size(); i+=2) {
                  auto &lb = result[i];
                  auto &ub = result[i + 1];
                  sqlstr += ("(" + currname + ">=" + std::to_string(lb) + " AND "
                              + currname + "<" + std::to_string(ub) + ") OR ");
                  for (int j = lb; j < ub; j++) {
                    csvstr += (std::to_string(j) + ",");
                  }
                }
                sqlstr = sqlstr.substr(0, sqlstr.size() - 4) + " AND ";
                csvstr = csvstr.substr(0, csvstr.size() - 1) + ")\",";
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
    sqlfile << sqlstr << "\n";
  }
  csvfile.close();
  sqlfile.close();
  return 0;
} 