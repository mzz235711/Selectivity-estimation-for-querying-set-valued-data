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
#include <queue>
#include <chrono>
#include <sys/time.h>
#include <unistd.h>

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::microseconds;
using std::chrono::seconds;
using std::chrono::system_clock;


#include "utils.h"

void generate_subgraph(Weightedgraph &graph,
		std::vector<int> &vertices, Weightedgraph &subgraph); 

void enumerate_all_cliques(Weightedgraph &graph,
	       	std::queue<std::pair<std::vector<int>, std::vector<int>>> clique_queue,
	       	Vectormap &set_map,
	       	int &max_length,
		std::vector<int> &results);

void subquery_search(Weightedgraph &graph,
	       	std::vector<int> &vertices,
		Vectormap &set_map,
		int &max_length,
	       	std::vector<int> &results);

void superquery_search(Weightedgraph &graph,
	       	std::vector<int> &vertices, 
		Vectormap &set_map,
		int &max_length,
		std::vector<int> &results);

void generate_subgraph(Weightedgraph &graph,
		std::vector<int> &vertices, Weightedgraph &subgraph) {
  for (int i = 0; i < vertices.size(); i++) {
    int u = vertices[i];
    for (int j = i + 1; j < vertices.size(); j++) {
      int v = vertices[j];
      if (graph[u].find(v) != graph[u].end()) {
	int weight = graph[u][v];
        subgraph[u].emplace(v, weight);
	subgraph[v].emplace(u, weight);
      }
    }
  }
}	

void enumerate_all_cliques(Weightedgraph &graph,
	       	std::queue<std::pair<std::vector<int>, std::vector<int>>> clique_queue,
	       	Vectormap &set_map,
	       	int &max_length, std::vector<int> &results) {
  while (clique_queue.size() > 0) {
    auto &clique_neighbor = clique_queue.front();
    auto &clique = clique_neighbor.first;
    auto &nbrs = clique_neighbor.second;
    int reduced_weight = 0;
    std::sort(clique.begin(), clique.end());
    if (set_map.find(clique) != set_map.end()) {
      int set_value = set_map[clique];
      results.push_back(set_value);
    }
    bool should_append = true;
    /*
    if (reduced_weight > 0) { 
      for (int i = 0; i < clique.size(); i++) {
        int u = clique[i];
        for (int j = i + 1; j < clique.size(); j++) {
          int v = clique[j];
          graph[u][v] -= reduced_weight;
	  graph[v][u] -= reduced_weight;
          if (graph[u][v] <= 0) {
	    should_append = false;
          }
        }
      }
    }
    */
    if (should_append && clique.size() < max_length) {
      for (int i = 0; i < nbrs.size(); i++) {
	int u = nbrs[i];
        bool is_common_nbr = true;
	for (auto &v : clique) {
	  if (graph[v].find(u) == graph[v].end()) {
	    is_common_nbr = false;
	    break;
	  }
	}
	if (is_common_nbr) {
	  auto new_base = clique;
	  new_base.push_back(u);
	  std::vector<int> new_nbrs(nbrs.begin() + i + 1, nbrs.end());
          clique_queue.emplace(new_base, new_nbrs);
	}
      }
    }
    clique_queue.pop();
  }
}

void subquery_search(Weightedgraph &graph,
	       	std::vector<int> &vertices,
		Vectormap &set_map,
		int &max_length,
	       	std::vector<int> &results) {
  std::sort(vertices.begin(), vertices.end());
  Weightedgraph subgraph(graph.size());
  generate_subgraph(graph, vertices, subgraph);
  std::queue<std::pair<std::vector<int>, std::vector<int>>> clique_queue;
  for (int i = 0; i < vertices.size(); i++) {
    int u = vertices[i];
    std::vector<int> base = {u};
    std::vector<int> nbrs;
    for (int j = i + 1; j < vertices.size(); j++) {
      int v = vertices[j];
      if (subgraph[u].find(v) != subgraph[u].end()) {
        nbrs.push_back(v);
      }
    }
    clique_queue.emplace(base, nbrs);
  }
  enumerate_all_cliques(subgraph, clique_queue, set_map, max_length, results);
  
}

void superquery_search(Weightedgraph &graph,
	       	std::vector<int> &vertices,
		Vectormap &set_map,
		int &max_length,
	       	std::vector<int> &results) {
  std::sort(vertices.begin(), vertices.end());
  // Check whether vertices is a clique
  for (int i = 0; i < vertices.size(); i++) {
    int u = vertices[i];
    for (int j = i + 1; j < vertices.size(); j++) {
      int v = vertices[j];
      if (graph[u].find(v) == graph[u].end()) {
        return;
      }
    }
  }

  // Get common neighbors
  int base_u = vertices[0];
  std::vector<int> induced_vertices = vertices; 
  for (auto &u_w : graph[base_u]) {
    bool common_nbr = true;
    int u = u_w.first;
    for (int i = 1; i < vertices.size(); i++) {
      int v = vertices[i];
      if (graph[v].find(u) == graph[v].end()) {
        common_nbr = false;
	break;
      }
    }
    if (common_nbr) {
      induced_vertices.push_back(u);
    }
  }
  // Enumerate all inner cliques
  //std::sort(induced_vertices.begin(), induced_vertices.end());
  Weightedgraph subgraph(graph.size());
  generate_subgraph(graph, induced_vertices, subgraph);
  std::queue<std::pair<std::vector<int>, std::vector<int>>> inner_clique_queue;
  for (int i = 0; i < vertices.size(); i++) {
    std::vector<int> base = {vertices[i]};
    std::vector<int> nbrs(vertices.begin() + i + 1, vertices.end());
    inner_clique_queue.emplace(base, nbrs);
  }
  std::vector<int> inner_clique_results;
  enumerate_all_cliques(subgraph, inner_clique_queue, set_map, max_length, inner_clique_results);
  
  // Reduce weight for inner cliques
  if (vertices.size() > 1 && set_map.find(vertices) != set_map.end()) {
    int set_value = set_map[vertices];
    //int set_frequency = frequency[set_value];
    //for (int i = 0; i < vertices.size(); i++) {
     // for (int j = i + 1; j < vertices.size(); j++) {
      //  subgraph[vertices[i]][vertices[j]] += set_frequency;
       // subgraph[vertices[j]][vertices[i]] += set_frequency;
     // }
   // }
  }
 
  // Enumrate all supercliques 
  std::queue<std::pair<std::vector<int>, std::vector<int>>> clique_queue;
  std::vector<int> nbrs(induced_vertices.begin() + vertices.size(), induced_vertices.end());
  clique_queue.push(std::make_pair(vertices, nbrs));
  enumerate_all_cliques(subgraph, clique_queue, set_map, max_length, results);
}

int main(int argc, char** argv) {

  std::ifstream idxfile("idx.csv");
  std::vector<std::string> idx2word;
  std::unordered_map<std::string, int> word2idx;
  int index;
  std::string line;
  std::string word;
  std::string pad = "ZZZ";
  while (getline(idxfile, line)) {
    std::stringstream s(line);
    s >> word >> index;
    idx2word.push_back(word);
    word2idx.emplace(word, index);
  }
  word2idx[pad] = idx2word.size();
  idx2word.push_back(pad);
  printf("Number of keyword: %lu\n", idx2word.size());
  /*
  std::ofstream idxfile("idx.csv");
  for (int i = 0; i < idx2word.size(); i++) {
    idxfile << idx2word[i] << " " << i << "\n";
  }
  return 0;
  */
  int v_num = idx2word.size();
  int partition_num = atoi(argv[1]);
  std::string folder_name = "/home_nfs/peizhi/zizhong/dataset/geotweet/new_idx_related/color_partition_" + std::to_string(partition_num);
  std::string partfilename = folder_name + "/part.txt";
  std::ifstream partfile(partfilename);
  std::vector<int> partition(v_num);
  int partid;
  for (int i = 0; i < v_num; i++) {
    partfile >> partid;
    partition[i] = partid;
  }
  std::cout << "cp1\n"; 
  std::vector<int> dis_num(partition_num, 0);
  std::vector<Vectormap> set_map(partition_num, Vectormap());
  std::string mapfilename = folder_name + "/dis_set_map.txt";
  std::ifstream mapfile(mapfilename);
  int u, v;
  int set_size;
  std::vector<int> max_length(partition_num, 0);
  //std::string frequencyfilename = folder_name + "/frequency.txt";
  //std::ifstream frequencyfile(frequencyfilename);
  //std::vector<std::vector<int>> frequency(partition_num, std::vector<int>());
  while (getline(mapfile, line)) {
    std::stringstream s(line);
    s >> partid >> u;
    dis_num[partid] = u + 1;
    for (int i = 0; i < u; i++) {
      getline(mapfile, line);
      std::stringstream setstr(line);
      setstr >> set_size;
      std::vector<int> set_key(set_size);
      for (int j = 0; j < set_size; j++) {
        setstr >> set_key[j];
      }
      int set_value;
      setstr >> set_value;
      set_map[partid][set_key] = set_value;
      if (set_size > max_length[partid]) {
        max_length[partid] = set_size;
      } 
    }
    std::vector<int> set_key = {word2idx[pad]};
    set_map[partid][set_key] = u;
    int weight;
    //for (int i = 0; i < u + 1; i++) {
    //  frequencyfile >> weight;
    //  frequency[partid].push_back(weight);
    //}
  }
  std::cout << "cp2\n";
  std::vector<Weightedgraph> graph(partition_num, Weightedgraph(v_num));
  std::string graphfilename = "./geotweet.mtx";
  std::ifstream graphfile(graphfilename);
  getline(graphfile, line);
  int e_num;
  int weight;
  graphfile >> u >> v >> e_num;
  for (int j = 0; j < e_num; j++) {
    graphfile >> u >> v >> weight;
    int uid = partition[u];
    int vid = partition[v];
    if (uid == vid) {
      if (u < v) {
        graph[uid][u].emplace(v, weight);
      } else{
        graph[uid][v].emplace(u, weight);
      }
    }
  }
  std::cout << "cp3\n";
  std::ifstream sqlfile("../geotweet_query_5.csv");
  std::unordered_map<std::string, int> coltype = {{"geotweet.createtime",0}, {"geotweet.lat",0}, {"geotweet.lon",0}, {"geotweet.country",1}};
  int querycount = 0;
  int lineidx = 0;
  int query_type = atoi(argv[2]);
  std::unordered_map<int, std::string> Q_TYPE = {{0, "subset"}, {1, "superset"}, {2, "all"}};
  std::ofstream outsqlfile(folder_name + "/geotweet_query_5_trans_clique" + Q_TYPE[query_type] + ".sql");
  std::ofstream outcsvfile(folder_name + "/geotweet_query_5_trans_clique" + Q_TYPE[query_type] + ".csv");
  auto start_time = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
  std::cout << "print\n";
  int max_sub_size = 0;
  while (getline(sqlfile, line)) {
    lineidx++;
    if (lineidx % 2 == query_type) {
      continue;
    }
    if (lineidx > 10 && query_type == 1) {
      break;
    }
    querycount++;
    std::cout << lineidx << "\n";
    std::stringstream s(line);
    std::string token; 
    std::string sqlstr("select count(*) from geotweet_settrie_" + Q_TYPE[query_type] + " where ");
    std::string csvstr("geotweet##");
    for (int i = 0; i < 3; i++) {
      std::getline(s, token, '#');
      if (i < 2) {
        continue;
      }
      std::stringstream predicates(token);
      std::string col, op, val;
      while (getline(predicates, col, ',')) {
        getline(predicates, op, ',');
	getline(predicates, val, ',');
	if (col.compare("geotweet.tags") != 0) {
	  csvstr += (col + "," + op + "," + val + ",");
	  if (coltype[col] == 0) {
	    sqlstr += (col + op + val + " AND ");
	  } else {
	    sqlstr += (col + op + "'" + val + "'" + " AND ");
	  }
	} else {
	  std::vector<std::vector<int>> query_vertices(partition_num, std::vector<int>());
	  std::stringstream setstream(val);
	  std::string queryelement;
	  int tnum = 0;
          while (getline(setstream, queryelement, '|')) {
	    tnum += 1;
	    int elementid = word2idx[queryelement];
	    int elementpartid = partition[elementid];
	    query_vertices[elementpartid].push_back(elementid);
	  }
	  for (auto &q_v : query_vertices) {
	    if (q_v.size() > max_sub_size) {
	      max_sub_size = q_v.size();
	    }
	  }
	  for (int i = 0; i < partition_num; i++) {
	    std::vector<int> results;
	    if (query_vertices[i].size() > 0) {
	      if (op.compare("<=") == 0) {
	        subquery_search(graph[i], query_vertices[i], set_map[i], max_length[i], results);
	      } else {
	        superquery_search(graph[i], query_vertices[i], set_map[i], max_length[i], results);
	      }
	    }
	    if (op.compare("<=") == 0 && results.size() == 0) {
	      std::vector<int> null_idx = {word2idx[pad]};
	      int null_value = set_map[i][null_idx];
	      results.push_back(null_value);
	    }
	    if (results.size() > 1) {
	      std::sort(results.begin(), results.end());
	      csvstr += ("geotweet.tags" + std::to_string(i) + ",IN,\"(");
	      sqlstr += (" tags_" + std::to_string(i) + " IN (");
	      for (auto &value : results) {
	        csvstr += (std::to_string(value) + ",");
		sqlstr += (std::to_string(value) + ",");
	      }
	      csvstr = csvstr.substr(0, csvstr.size() - 1) + ")\",";
	      sqlstr = sqlstr.substr(0, sqlstr.size() - 1) + ") AND ";
	    } else if (results.size() == 1) {
	      csvstr += ("geotweet.tags" + std::to_string(i) + ",=," + std::to_string(results[0]) + ",");
	      sqlstr += (" tags" + std::to_string(i) + "=" + std::to_string(results[0]) + " AND ");
	    }
	  }
          if (op.compare("<=") == 0) {
	    csvstr += ("geotweet.tags_lfword,=,0,");
	    sqlstr += ("tags_lfword=0 AND ");
	  }
	}
      }
    }
    std::getline(s, token);
    csvstr = csvstr.substr(0, csvstr.size() - 1) + "#" + token;
    sqlstr = sqlstr.substr(0, sqlstr.size() - 5) + ";";
    outsqlfile << sqlstr << "\n";
    outcsvfile << csvstr << "\n";
  }
  auto end_time = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
  auto last_time = (end_time - start_time) / querycount;
  std::cout << "Avg time: " << last_time << "\n";
  std::cout << "Max sub size: " << max_sub_size << "\n";
  return 0;
}
