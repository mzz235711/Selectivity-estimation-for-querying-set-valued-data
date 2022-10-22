#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>

#include "utils.h"

int main(int argc, char **argv) {
  std::string pad = "ZZZ";
  std::ifstream idxfile("idxfile");
  std::vector<std::string> idx2word;
  std::unordered_map<std::string, int> word2idx;
  std::string word;
  int idx;
  int partition_num = 0;
  while (idxfile >> word >> idx) {
    idx2word.push_back(word);
    word2idx[word] = idx;
    if (idx > partition_num) {
      partition_num = idx;
    }
  }
  partition_num += 1;
  idx2word.push_back(pad);
  word2idx[pad] = idx2word.size() - 1;
  
  std::string folder_name = "./graph_color";
  std::ifstream partfile(folder_name + "/part.txt");
  std::vector<int> partid;
  int p;
  while (partfile >> p) {
    partid.push_back(p);
  }
  std::vector<int> idx2val(partid.size());
  std::vector<int> pad2val(partition_num);
  std::ifstream mapfile(folder_name + "/dis_setmap.txt");
  int pid, setnum, setlen, val;
  while (mapfile >> pid >> setnum) {
    for (int i = 0; i < setnum; i++) {
      mapfile >> setlen >> idx >> val;
      if (idx != word2idx[pad]) {
        idx2val[idx] = val;
      } else {
        pad2val[pid] = val;
      }
    }
  }
  std::unordered_map<int, std::string> Q_TYPE={{0, "subset"}, {1, "superset"}, {2, "all"}};
  std::string setname = "gn.FEATURE_NAME";
  std::unordered_map<std::string, int> col_type={{"gn.FEATURE_TYPE", 0}};
  int query_type = atoi(argv[1]);
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
            for (auto &w : query_words[i]) {
              result.push_back(idx2val[w]);
            }
            result.push_back(pad2val[i]);
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
              if (query_words[i].size() > 1) {
                std::cout << "ERROR\n";
              }
              csvstr += (currname + ",=," + std::to_string(result[0]) + ",");
              sqlstr += (currname + " = " + std::to_string(result[0]) + " AND ");
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
      sqlstr = sqlstr.substr(0, sqlstr.size()) + ";";
    }
    csvfile << csvstr << "\n";
    sqlfile << sqlstr << "\n";
  }
  csvfile.close();
  sqlfile.close();
}