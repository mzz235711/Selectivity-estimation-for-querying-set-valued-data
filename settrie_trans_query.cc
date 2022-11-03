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



void dfs(TreeNode &curr_node, int &node_num, std::ofstream &outfile) {
  node_num++;
  outfile << curr_node.id << " " << curr_node.level <<  "\n";
  for (auto &next_node : curr_node.children) {
    dfs(next_node, node_num, outfile);
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

void deserialize_node(TreeNode* curr_node_pt, std::ifstream &file, int &level) {
  TreeNode new_node;
  new_node.level = level;
  file.read((char*)(&new_node.id), sizeof(int));
  file.read((char*)(&new_node.last_flag), sizeof(bool));
  file.read((char*)(&new_node.range.first), sizeof(int));
  file.read((char*)(&new_node.range.second), sizeof(int));
  curr_node_pt->children.emplace_back(new_node);
}

void deserialize_root(TreeNode &root, std::ifstream &file) {
  file.read((char*)(&root.level), sizeof(int));
  file.read((char*)(&root.id), sizeof(int));
  file.read((char*)(&root.last_flag), sizeof(bool));
  file.read((char*)(&root.range.first), sizeof(int));
  file.read((char*)(&root.range.second), sizeof(int));
}

void deserialize(TreeNode &root, std::ifstream &file, int &node_num) {
  auto curr_node_pt = &root;
  int curr_level = 0;
  deserialize_root(root, file);
  for (int i = 0; i < node_num - 1; i++) {
    int level;
    file.read((char*)(&level), sizeof(int));
    if (level == curr_level + 1) {
      deserialize_node(curr_node_pt, file, level);
      curr_level++;
      curr_node_pt = &(curr_node_pt->children[curr_node_pt->children.size() - 1]);
    } else {
      curr_node_pt = &root;
      for (curr_level = 0; curr_level < level - 1; curr_level++) {
        curr_node_pt = &(curr_node_pt->children[curr_node_pt->children.size() - 1]);
      }
      deserialize_node(curr_node_pt, file, level);
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
  int partition_num = atoi(argv[1]);
  std::string folder_name = "./color_partition_" + std::to_string(partition_num);
  std::string partfilename = folder_name +  "/part.txt";
  //std::string partfilename = "./min_dis_part_" + std::to_string(partition_num) +  "/approx_clique_part_" + std::to_string(partition_num) + ".txt";
  std::ifstream partfile(partfilename);
  std::vector<int> partition(v_num);
  int partid;
  for (int i = 0; i < v_num; i++) {
    partfile >> partid;
    partition[i] = partid;
  }
  std::cout << "cp1\n"; 
  std::vector<int> dis_num(partition_num, 0);

  std::string mapfilename = folder_name + "/dis_set_map.txt";
  //std::string mapfilename = "./min_dis_part_" + std::to_string(partition_num) + "/dis_set_map.txt";
  std::ifstream mapfile(mapfilename);
  int u, v;
  int set_size;
  std::vector<int> max_length(partition_num, 0);
  std::vector<TreeNode> settrie(partition_num);
  std::string treefilename = folder_name + "/settrie.txt";
  std::ifstream treefile(treefilename, std::ios::in|std::ios::binary);
  std::ofstream tmpfile(folder_name + "/tmp1.csv");
  for (int i = 0; i < partition_num; i++) {
    int node_num;
    treefile.read((char*)(&node_num), sizeof(int));
    dis_num[i] = node_num;
    std::cout << node_num << "\n";
    deserialize(settrie[i], treefile, node_num);
    int nnum = 0;
    dfs(settrie[i], nnum, tmpfile);
    std::cout << nnum << "\n";
  }

  /*
  int node_num = 0;
  std::ofstream tmpfile(folder_name + "/tmp2.csv");
  while (getline(mapfile, line)) {
    std::stringstream s(line);
    s >> partid >> u;
    std::cout << partid << " " << u << "\n";
    dis_num[partid] = u;
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
      settrie_insert(settrie[partid], set_key, node_num);
      if (set_size > max_length[partid]) {
        max_length[partid] = set_size;
      } 
    }
  }
  std::cout << "node_num: " << node_num << "\n";
  std::vector<Setvector> set_vector(partition_num, Setvector()); 
  for (int i = 0; i < partition_num; i++) {
    int init_value = 0;
    int nnum = 0;
    std::vector<int> curr_set;
    dfs_addvalue(settrie[i], init_value, nnum);
    std::cout << i << " " << nnum << "\n";
    int tmpnum = 0;
    dfs(settrie[i], tmpnum, tmpfile);
  }
  */
  std::cout << "cp2\n";
  std::unordered_map<int, std::string> Q_TYPE = {{0, "subset"}, {1, "superset"}, {2, "all"}};
  std::unordered_map<std::string, int> coltype = {{"geotweet.createtime",0}, {"geotweet.lat",0}, {"geotweet.lon",0}, {"geotweet.country",1}};
  std::ifstream sqlfile("../geotweet_query_5.csv");
  int query_count = 0;
  int lineidx = 0;
  int query_type = atoi(argv[2]);
  std::ofstream outsqlfile(folder_name + "/geotweet_query_5_trans_settrie_" + Q_TYPE[query_type] + ".sql");
  std::ofstream outdeepdbsqlfile(folder_name + "/geotweet_query_5_trans_settrie_" + Q_TYPE[query_type] +"_deepdb.sql");
  std::ofstream outdeepdbcardsqlfile(folder_name + "/geotweet_query_5_trans_settrie_" + Q_TYPE[query_type] + "_deepdbcard.csv");
  std::ofstream outcsvfile(folder_name + "/geotweet_query_5_trans_settrie_" + Q_TYPE[query_type] + ".csv");
  auto start_time = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
  int max_sub_lenth = 0;
  int avg_sub_lenth = 0;
  std::vector<int> sub_lenth_vec(179, 0);
  std::vector<int> sub_time_vec(179, 0);
  std::vector<int> time_state;
  std::string table_name = "geotweet_settrie_" + std::to_string(partition_num);
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
    std::string sqlstr("select count(*) from geotweet_settrie_" + std::to_string(partition_num) + " as geotweet where ");
    std::string deepdbsqlstr("select count(*) from " + table_name + " where ");
    std::string csvstr("geotweet##");
    std::string postall("select count(*) from geotweet_settrie_" + std::to_string(partition_num) + " as geotweet where ");
    for (int i = 0; i < partition_num; i++) {
      int val = settrie[i].children[settrie[i].children.size() - 1].range.first;
      postall += ("geotweet.tags" + std::to_string(i) + "=" + std::to_string(val) + " AND ");
    }
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
	    postall += (col + op + val + " AND ");
	  } else {
	    sqlstr += (col + op + "'" + val + "' AND ");
	    deepdbsqlstr += (table_name + "." + cname + op + "'" + val + "' AND ");
	    postall += (col + op + "'" + val + "' AND ");
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
	    if (op.compare("<=") == 0) {
	      get_all_subsets(settrie[i], query_vertices[i], 0, results);
	      results.push_back(settrie[i].children[settrie[i].children.size() - 1].range.first);
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
	      if (query_vertices[i].size() > 0) {
	        get_all_supersets(settrie[i], query_vertices[i], 0, results);
	        sqlstr += "(";
	        deepdbsqlstr += (table_name + ".tags" + std::to_string(i) + " IN (");
		if (results.size() > 2 || results[1] - results[0] > 1) {
                  csvstr += "geotweet.tags" + std::to_string(i) + ",IN,\"(";
		} else {
                  csvstr += "geotweet.tags" + std::to_string(i) + ",=,"; 
		}	
	        for (int j = 0; j < results.size(); j+=2) {
	          int &lb = results[j];
	          int &ub = results[j + 1];
	         sqlstr += ("(geotweet.tags" + std::to_string(i) + ">=" + std::to_string(lb) 
				+ " AND geotweet.tags" + std::to_string(i) + "<" + std::to_string(ub)
			        + ") OR ");
	          deepdbsqlstr += (std::to_string(lb) + "," + std::to_string(ub - 1) + ",");
		  for (int k = lb; k < ub; k++) {
		    csvstr += (std::to_string(k) + ",");
		  }
	        }
	        sqlstr = sqlstr.substr(0, sqlstr.size() - 4) + ") AND ";
	        deepdbsqlstr = deepdbsqlstr.substr(0, deepdbsqlstr.size() - 1) + ") AND ";
		if (results.size() > 2 || (results[1] - results[0]) > 1) {
		  csvstr = csvstr.substr(0, csvstr.size() - 1) + ")\",";
		} else {
		  csvstr = csvstr.substr(0, csvstr.size() - 1) + ",";
		}
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
    postall = postall.substr(0, postall.size() - 5) + ";";
    deepdbsqlstr = deepdbsqlstr.substr(0, deepdbsqlstr.size() - 5) + ";";
    //outsqlfile << sqlstr << "\n";
    if (query_type == 0) {
      outsqlfile << sqlstr << postall << "\n";
    } else {
      outsqlfile << sqlstr << "\n";
    }
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
