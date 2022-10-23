#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <queue>
#include <utility>

#include "utils.h"
void enumerate_all_cliques(Graph &graph, CanQueue &candidates, 
     Setmap &setmap, std::vector<int> &result, int &maxlenth) {
  while (candidates.size() > 0) {
    auto &set_rest = candidates.front();
    candidates.pop();
    auto &set = set_rest.first;
    auto &rest = set_rest.second;
    if (setmap.find(set) != setmap.end()) {
      result.push_back(setmap[set]);
    }
    if (set.size() < maxlenth) {
      std::vector<int> newcand;
      for (auto &w : rest) {
        bool neigh = true;
        for (auto &s : set) {
          if (graph[s].find(w) == graph[s].end()) {
            neigh = false;
            break;
          }
        }
        if (neigh) {
          newcand.push_back(w);
        }
      }
      for (int i = 0; i < newcand.size(); i++) {
        auto newset = set;
        newset.push_back(newcand[i]);
        std::vector<int> newcand(newcand.begin() + i, newcand.end());
        candidates.push(std::make_pair(newset, newcand));
      }
    }
  }
}

void generate_subgraph(Graph &graph, std::vector<int> &vertices, Graph &subgraph) {
  for (int i = 0; i < vertices.size(); i++) {
    for (int j = i + 1; j < vertices.size(); j++) {
      auto &u = vertices[i];
      auto &v = vertices[j];
      if (graph[u].find(v) != graph[u].end()) {
        subgraph[u].emplace(v);
      }
    }
  }
}

void subset_search(Graph &graph, std::vector<int> &query, Setmap &setmap,
       std::vector<int> &result, int &maxlenth) {
  Graph subgraph(graph.size(), std::unordered_set<int>());
  generate_subgraph(graph, query, subgraph);
  CanQueue candidates;
  for (int i = 0; i < query.size(); i++) {
    auto &u = query[i];
    std::vector<int> set{u};
    std::vector<int> rest;
    for (int j = i + 1; j < query.size(); j++) {
      auto &v = query[j];
      if (subgraph[u].find(v) != subgraph[u].end()) {
        rest.push_back(u);
      }
    }
    candidates.push(std::make_pair(set, rest));
  }  
  enumerate_all_cliques(subgraph, candidates, setmap, result, maxlenth);
}

void superset_search(Graph &graph, std::vector<int> &query, Setmap &setp,
       std::vector<int> &result, int &maxlenth) {
  Graph subgraph(graph.size(), std::unordered_set<int>());
  std::vector<int> cand_vertices;
  auto set = query;
  std::vector<int> rest;
  for (int u = 0; u < graph.size(); u++) {
    bool iscand = true;
    for (auto &v : query) {
      if (u < v && graph[u].find(v) == graph[u].end()) {
        iscand = false;
        break;
      } else if (v < u && graph[v].find(u) == graph[v].end()) {
        iscand = false;
        break;
      }
    }
    if (iscand) {
      cand_vertices.push_back(u);
      if (std::find(query.begin(), query.end(), u) == query.end()) {
        rest.push_back(u);
      }
    }
  }
  generate_subgraph(graph, cand_vertices, subgraph);
  CanQueue candidates;
  candidates.push(std::make_pair(set, rest));
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
  std::vector<int> padvec = {word2idx[pad]};
  int partition_num = atoi(argv[1]);
  std::string folder_name = "./color_partition_" + std::to_string(partition_num);
  std::ifstream partfile(folder_name + "/part.txt");
  std::vector<int> partid;
  int p;
  while (partfile >> p) {
    partid.push_back(p);
  }
  std::vector<Setmap> setmap(partition_num, Setmap());
  std::vector<int> maxlenth(partition_num, 0);
  std::ifstream mapfile(folder_name + "/dis_setmap.txt");
  int pid, setnum, setlen, val;
  while (mapfile >> pid >> setnum) {
    for (int i = 0; i < setnum; i++) {
      mapfile >> setlen;
      if (setlen > maxlenth[i]) {
        maxlenth[i] = setlen;
      }
      std::vector<int> words(setlen);
      for (int j = 0; j < setlen; j++) {
        mapfile >> words[j];
      }
      mapfile >> val;
      setmap[pid][words] = val;
    }
  }
  partfile.close();
  mapfile.close();

  std::vector<Graph> graph(partition_num, Graph(idx2word.size())); 
  std::ifstream graphfile("gn.mtx");
  std::string line;
  std::getline(graphfile, line);
  int nodenum, edgenum, u, v;
  graphfile >> nodenum >> edgenum;
  for (int i = 0; i < edgenum; i++) {
    graphfile >> u >> v;
    if (partid[u] == partid[v]) {
      p = partid[u];
      if (u < v) {
        graph[p][u].emplace(v);
      } else {
        graph[p][v].emplace(u);
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
            if (query_words[i].size() > 0) {
              subset_search(graph[i], query_words[i], setmap[i], result, maxlenth[i]);
            }
            result.push_back(setmap[i][padvec]);
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
              superset_search(graph[i], query_words[i], setmap[i], result, maxlenth[i]);
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