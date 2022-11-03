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

int binary_search(std::vector<int> &arr, int l, int r, int &x) {
  if (r >= l) {
    int mid = l + (r - l) / 2;
    if (arr[mid] == x) {
      return mid;
    } else if (arr[mid] > x) {
      return binary_search(arr, l, mid - 1, x);
    } else {
      return binary_search(arr, mid + 1, r, x);
    }
  }
  return l;
}

void settrie_insert(TreeNode &root, std::vector<int> &set_key, int &node_num) {
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
	node_num++;
	break;
      }
    }
    if (it == curr_node->children.end()) {
      TreeNode new_node;
      new_node.id = element;
      curr_node->children.push_back(new_node);
      curr_node = &(curr_node->children[curr_node->children.size() - 1]);
      node_num++;
    }
  }
  curr_node->last_flag = true;
}

void dfs_addvalue(TreeNode &curr_node, int &set_value, int &node_num) {
  node_num++;
  curr_node.range.first = set_value;
  if (curr_node.last_flag == true) {
    //if (set_map.find(curr_set) != set_map.end()) {
    //  auto set_value = set_map[curr_set];
    //results.push_back(curr_node.last_flag);
    //}
    set_value++;
  }
  for (auto &next_node : curr_node.children) {
    //auto next_set = curr_set;
    //next_set.push_back(next_node.id);
    dfs_addvalue(next_node, set_value, node_num);
  }
  curr_node.range.second = set_value;
}



void dfs(TreeNode &curr_node, std::vector<int> &results, std::vector<int> &curr_set, Vectormap &set_map) {
  if (curr_node.last_flag > -1) {
    //if (set_map.find(curr_set) != set_map.end()) {
    //  auto set_value = set_map[curr_set];
    results.push_back(curr_node.last_flag);
    //}
  }
  for (auto &next_node : curr_node.children) {
    auto next_set = curr_set;
    next_set.push_back(next_node.id);
    dfs(next_node, results, next_set, set_map);
  }
}

void get_all_subsets(TreeNode &curr_node, std::vector<int> &set_key, int set_index, std::vector<int> &results) {
  if (set_index == set_key.size()) {
    return;
  }
  std::vector<TreeNode>::iterator it;
  for (it = curr_node.children.begin(); it != curr_node.children.end(); it++) {
    if (it->id == set_key[set_index]) {
      auto &next_node = *it;
      if (next_node.last_flag == true) {
        results.push_back(next_node.range.first);
      }
      get_all_subsets(next_node, set_key, set_index + 1, results);
      set_index++;
      if (set_index == set_key.size()) {
        break;
      }
    } else if (it->id > set_key[set_index]) {
      set_index++;
      if (set_index == set_key.size()) {
        break;
      }
      it--;
    }
  }
}

void get_all_supersets(TreeNode &curr_node, std::vector<int> &set_key, int set_index, std::vector<int> &results) {
  if (set_index == set_key.size()) {
    //dfs(curr_node, results, curr_set, set_map);
    results.push_back(curr_node.range.first);
    results.push_back(curr_node.range.second);
    return;
  }
  int from;
  if (set_index == 0) {
    from = -1;
  } else {
    from = set_key[set_index - 1];
  }
  int upto = set_key[set_index];
  for (auto &next_node : curr_node.children) {
    if (next_node.id > from && next_node.id < upto) {
      //auto next_set = curr_set;
      //next_set.push_back(next_node.id);
      get_all_supersets(next_node, set_key, set_index, results);
    } else if (next_node.id == upto) {
      //auto next_set = curr_set;
      //next_set.push_back(next_node.id);
      get_all_supersets(next_node, set_key, set_index + 1, results);
    } else {
      break;
    }
  }

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
  std::string folder_name = "./graph_color";
  std::string partfilename = folder_name +  "/part.txt";
  //std::string partfilename = "./min_dis_part_" + std::to_string(partition_num) +  "/approx_clique_part_" + std::to_string(partition_num) + ".txt";
  std::ifstream partfile(partfilename);
  std::vector<int> partition(v_num);
  int partid;
  int maxid = 0;
  for (int i = 0; i < v_num; i++) {
    partfile >> partid;
    partition[i] = partid;
    if (partid > maxid) {
      maxid = partid;
    }
  }
  std::cout << "cp1\n"; 
  int partition_num = maxid + 1;
  std::vector<int> dis_num(partition_num, 1);
  for (auto &id : partition) {
    dis_num[id]++;
  }
  std::string mapfilename = folder_name + "/dis_set_map.txt";
  std::ifstream mapfile(mapfilename);
  std::vector<int> idx2colval(v_num);
  std::vector<int> pad2colval(partition_num);
  int u, v;
  int set_size;
  std::vector<int> max_length(partition_num, 0);
  int node_num = 0;
  while (getline(mapfile, line)) {
    std::stringstream s(line);
    s >> partid >> u;
    std::cout << partid << " " << u << "\n";
    dis_num[partid] = u;
    for (int i = 0; i < u; i++) {
      getline(mapfile, line);
      std::stringstream setstr(line);
      setstr >> set_size;
      int set_key, set_value;
      setstr >> set_key;
      setstr >> set_value;
      if (set_key != v_num - 1) {
        idx2colval[set_key] = set_value;
      } else {
        pad2colval[partid] = set_value;
      }
      if (set_size > max_length[partid]) {
        max_length[partid] = set_size;
      }
      if (set_size > 1) {
        std::cout << "ERROR size: " << set_size << "\n";
      } 
    }
  }
  std::cout << "node_num: " << node_num << "\n";

  //std::string mapfilename = "./min_dis_part_" + std::to_string(partition_num) + "/dis_set_map.txt";
  std::cout << "cp2\n";
  std::unordered_map<int, std::string> Q_TYPE = {{0, "subset"}, {1, "superset"}, {2, "all"}};
  std::ifstream sqlfile("../geotweet_query_5.csv");
  std::unordered_map<std::string, int> coltype = {{"geotweet.createtime",0}, {"geotweet.lat",0}, {"geotweet.lon",0}, {"geotweet.country",1}};
  int query_count = 0;
  int lineidx = 0;
  int query_type = atoi(argv[1]);
  std::ofstream outsqlfile(folder_name + "/geotweet_query_5_trans_settrie_" + Q_TYPE[query_type] + ".sql");
  std::ofstream outdeepdbsqlfile(folder_name + "/geotweet_query_5_trans_settrie_" + Q_TYPE[query_type] +"_deepdb.sql");
  std::ofstream outdeepdbcardsqlfile(folder_name + "/geotweet_query_5_trans_settrie_" + Q_TYPE[query_type] + "_deepdbcard.csv");
  std::ofstream outcsvfile(folder_name + "/geotweet_query_5_trans_settrie_" + Q_TYPE[query_type] + ".csv");
  //std::ofstream outsqlfile("min_dis_part_" + std::to_string(partition_num) + "/geotweet_query_1_trans_settrie_superset.sql");
  //std::ofstream outcsvfile("min_dis_part_" + std::to_string(partition_num) + "/geotweet_query_1_trans_settrie_superset.csv");
  auto start_time = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
  int max_sub_lenth = 0;
  int avg_sub_lenth = 0;
  std::vector<int> sub_lenth_vec(179, 0);
  std::vector<int> sub_time_vec(179, 0);
  std::vector<int> time_state;
  std::string table_name = "geotweet_color";
  outdeepdbcardsqlfile << "query_no,query,cardinality_true\n";
  while (getline(sqlfile, line)) {
    lineidx++;
    if (lineidx % 2 == query_type) {
      continue;
    }
    auto each_start_time = int(duration_cast<microseconds>(system_clock::now().time_since_epoch()).count());
    //if (lineidx > 1) { 
    //  break;
    //}
    //std::cout << lineidx << "\n";
    std::stringstream s(line);
    std::string token; 
    std::string sqlstr("select count(*) from geotweet_color as geotweet where ");
    std::string deepdbsqlstr("select count(*) from " + table_name + " where ");
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
	  std::string cname, tname;
	  std::stringstream t_c(col);
	  getline(t_c, tname, '.'); 
	  getline(t_c, cname, '.'); 
	  if (coltype[col] == 0) {
	    sqlstr += (col + op + val + " AND ");
	    deepdbsqlstr += (table_name + "." + cname + op + val + " AND ");
	  } else {
	    sqlstr += (col + op + "'" + val + "' AND ");
	    deepdbsqlstr += (table_name + "." + cname + op + "'" + val + "' AND ");
	  }
	} else {
	  std::vector<std::vector<int>> query_vertices(partition_num, std::vector<int>());
	  std::stringstream setstream(val);
	  std::string queryelement;
          while (getline(setstream, queryelement, '|')) {
	    int elementid = word2idx[queryelement];
	    int elementpartid = partition[elementid];
	    query_vertices[elementpartid].push_back(elementid);
	  }  
	  for (auto &q_v : query_vertices) {
	    if (q_v.size() > max_sub_lenth) {
	      max_sub_lenth = q_v.size();
	    }
	    //avg_sub_lenth += q_v.size();
            //sub_lenth_vec[q_v.size()]++;
	    //std::cout << q_v.size() << " ";
	  }
	  for (int i = 0; i < partition_num; i++) {
	    std::vector<int> results;
	    std::vector<int> curr_set;
            auto in_start_time = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
	    std::sort(query_vertices[i].begin(), query_vertices[i].end());
	    for (auto &v : query_vertices[i]) {
	      results.push_back(idx2colval[v]);
	    }
	    if (op.compare("<=") == 0) {
              //get_all_subsets(settrie[i], query_vertices[i], 0, results);
	      results.push_back(pad2colval[i]);
	      if (results.size() > 1) {
	        std::sort(results.begin(), results.end());
   	        csvstr += ("geotweet.tags" + std::to_string(i) + ",IN,\"(");
	        sqlstr += ("geotweet.tags" + std::to_string(i) + " IN (");
	        deepdbsqlstr += (table_name + ".tags" + std::to_string(i) + " IN (");
	        for (auto &value : results) {
	          csvstr += (std::to_string(value) + ",");
	          sqlstr += (std::to_string(value) + ",");
	          deepdbsqlstr += (std::to_string(value) + "," + std::to_string(value) + ",");
	        }
	        csvstr = csvstr.substr(0, csvstr.size() - 1) + ")\",";
	        sqlstr = sqlstr.substr(0, sqlstr.size() - 1) + ") AND ";
	        deepdbsqlstr = deepdbsqlstr.substr(0, deepdbsqlstr.size() - 1) + ") AND ";
	      } else if (results.size() == 1) {
	        csvstr += ("geotweet.tags" + std::to_string(i) + ",=," + std::to_string(results[0]) + ",");
	        sqlstr += ("geotweet.tags" + std::to_string(i) + "=" + std::to_string(results[0]) + " AND ");
	        deepdbsqlstr += (table_name + ".tags" + std::to_string(i) + "=" + std::to_string(results[0]) + " AND ");
	      }
	    } else {
              //get_all_supersets(settrie[i], query_vertices[i], 0, results);
	      if (results.size() > 1) {
	        std::cout << "ERROR: " << i << " " << results.size() << "\n";
	      } else if (results.size() == 1) {
		std::sort(results.begin(), results.end());
	        sqlstr += "geotweet.tags" + std::to_string(i) + "=" + std::to_string(results[0]) + " AND ";
	        deepdbsqlstr += (table_name + ".tags" + std::to_string(i) + "=" + std::to_string(results[0])) + " AND ";
                csvstr += "geotweet.tags" + std::to_string(i) + ",=," + std::to_string(results[0]) + ","; 
	      }
	    }
	  }
          if (op.compare("<=") == 0) {
	    csvstr += ("geotweet.tags_lfword,=,0,");
	    sqlstr += ("geotweet.tags_lfword=0 AND ");
	    deepdbsqlstr += (table_name + ".tags_lfword=0 AND ");
	  }
	}
      }
    }
    std::getline(s, token);
    csvstr = csvstr.substr(0, csvstr.size() - 1) + "#" + token;
    sqlstr = sqlstr.substr(0, sqlstr.size() - 5) + ";";
    deepdbsqlstr = deepdbsqlstr.substr(0, deepdbsqlstr.size() - 5) + ";";
    outsqlfile << sqlstr << "\n";
    outdeepdbsqlfile << deepdbsqlstr << "\n";
    outdeepdbcardsqlfile << query_count << ",\""<< deepdbsqlstr << "\"," << token << "\n";
    outcsvfile << csvstr << "\n";
    query_count++;
    auto each_end_time = int(duration_cast<microseconds>(system_clock::now().time_since_epoch()).count());
    time_state.push_back(each_end_time - each_start_time);
  }
  auto end_time = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
  auto last_time = (end_time - start_time) / query_count;
  std::cout << "Avg time: " << last_time << "\n";
  std::cout << "Max sub lenth: " << max_sub_lenth << "\n";
  std::cout << "Avg sub lenth: " << avg_sub_lenth / (query_count * partition_num) << "\n";
  for (int i = 0; i < sub_lenth_vec.size(); i++) {
    if (sub_lenth_vec[i] > 0) {
      std::cout << i << " " << sub_lenth_vec[i] << "\n";
    }
  }
  /*
  std::ofstream state("min_dis_part_" + std::to_string(partition_num) + "/settrie_state_subset.csv");
  std::sort(time_state.begin(), time_state.end());
  for (int i = 0; i < time_state.size(); i++) {
      state << time_state[i] << "us\n";
  }
  state.close();
  */
  return 0;
}
