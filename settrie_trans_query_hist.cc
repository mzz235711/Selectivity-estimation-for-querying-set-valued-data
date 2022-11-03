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

int dfs_addvalue(TreeNode &curr_node, int &set_value, std::vector<int> &frequency, int level) {
  curr_node.range.first = set_value;
  int total_freq = 0;
  curr_node.level = level;
  if (curr_node.last_flag == true) {
    curr_node.freq = frequency[set_value];
    total_freq += frequency[set_value];
    set_value++;
  }
  for (auto &next_node : curr_node.children) {
    total_freq += dfs_addvalue(next_node, set_value, frequency, level + 1);
  }
  curr_node.total_freq = total_freq - curr_node.freq;
  curr_node.range.second = set_value;
  return total_freq;
}

void add_percentage(TreeNode &curr_node, std::unordered_map<int, double> &percentage) {
  if (percentage.find(curr_node.id) != percentage.end()) {
    percentage[curr_node.id] += curr_node.freq;
  } else {
    percentage[curr_node.id] = curr_node.freq;
  }
  for (auto &next_node : curr_node.children) {
    add_percentage(next_node, percentage);
  }

}	

void search_lowfreq(TreeNode &curr_node) {
  //if (curr_node.level > 0 && curr_node.total_freq < FREQ_THRES) {
  if (curr_node.level > 2) {
    for (auto &next_node : curr_node.children) {
      add_percentage(next_node, curr_node.percentage);
    }
    //curr_node.percentage[1] = 1;
    for (auto &k_v : curr_node.percentage) {
      k_v.second = k_v.second / curr_node.total_freq;
    }
  } else {
    for (auto &next_node : curr_node.children) {
      search_lowfreq(next_node);
    }
  }
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

void get_all_subsets(TreeNode &curr_node, std::vector<int> &set_key, int set_index, std::vector<int> &results, std::vector<double> &results_freq, double &total_freq, double &freq) {
  if (curr_node.last_flag == true) {
    results.push_back(curr_node.range.first);
    results_freq.push_back(1);
    total_freq += curr_node.freq;
    freq += curr_node.freq;
    if (curr_node.range.first < 0) {
      std::cout << curr_node.range.first << " " << curr_node.id << "\n";
      exit(1);
    }
  } 
  if (curr_node.percentage.size() != 0) {
    double est_freq = 1; 
    for (auto &k_v : curr_node.percentage) {
      auto &key = k_v.first;
      if (std::find(set_key.begin() + set_index, set_key.end(), key) == set_key.end()) {
        auto &val = k_v.second;
	if (val > 1 || val < 0) {
	  std::cout << "ERROR: " << val << "\n";
	  exit(0);
	}
        est_freq *= (1 - val);	
	if (est_freq == 0.001) {
	  break;
	}
      }
    }
    /*
    for (int i = set_index - 1; i < set_key.size(); i++) {
      auto &key = set_key[i];
      if (curr_node.percentage.find(key) != curr_node.percentage.end()) {
        auto &val = curr_node.percentage[key];
	if (val > 1 || val < 0) {
	  std::cout << "ERROR invalid value: " << val << "\n";
	  exit(0);
	} else if (val == 0) {
	  est_freq = -1;
	  break;
	}
	est_freq /= (1 - val);
      }
      //else {
      //  std::cout << "ERROR None exist key: " << key << "\n";
      //	exit(0);
      //}
    }
    */
    if (est_freq > 0) {
      results.push_back(curr_node.hist_val);
      results_freq.push_back(est_freq);
      total_freq += curr_node.hist_freq;
      freq += (curr_node.hist_freq * est_freq);
    }
    if (curr_node.hist_val < 0) {
      std::cout << curr_node.hist_val << " " << curr_node.id << "\n";
      exit(1);
    }
  }
  if (set_index == set_key.size()) {
    return;
  }
  std::vector<TreeNode>::iterator it;
  for (it = curr_node.children.begin(); it != curr_node.children.end(); it++) {
    if (it->id == set_key[set_index]) {
      auto &next_node = *it;
      //if (next_node.last_flag == true) {
      //  results.push_back(next_node.range.first);
      //}
      get_all_subsets(next_node, set_key, set_index + 1, results, results_freq, total_freq, freq);
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

void get_all_supersets(TreeNode &curr_node, std::vector<int> &set_key, int set_index, std::vector<int> &results, std::vector<double> &results_freq, double &total_freq, double &freq) {
  if (set_index == set_key.size()) {
    //dfs(curr_node, results, curr_set, set_map);
    results.push_back(curr_node.range.first);
    results.push_back(curr_node.range.second);
    results_freq.push_back(1);
    total_freq += curr_node.total_freq;
    freq += curr_node.total_freq;
    return;
  } else if (curr_node.percentage.size() > 0) {
    double est_freq = 1;
    for (int i = set_index; i < set_key.size(); i++) {
      if (std::find(curr_node.full_nodes.begin(), curr_node.full_nodes.end(), set_key[i]) != curr_node.full_nodes.end()) {
        continue;
      } else if (curr_node.percentage.find(set_key[i]) != curr_node.percentage.end()) {
        est_freq *= curr_node.percentage[set_key[i]];
      } else {
	est_freq = -1;
        break;
      }
    }
    if (est_freq > 0) {
      results_freq.push_back(est_freq);
      results.push_back(curr_node.hist_val);
      results.push_back(curr_node.hist_val + 1);
      total_freq += curr_node.hist_freq;
      freq += (curr_node.hist_freq * est_freq);
    }
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
      get_all_supersets(next_node, set_key, set_index, results, results_freq, total_freq, freq);
    } else if (next_node.id == upto) {
      //auto next_set = curr_set;
      //next_set.push_back(next_node.id);
      get_all_supersets(next_node, set_key, set_index + 1, results, results_freq, total_freq, freq);
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
  file.read((char*)(&new_node.freq), sizeof(int));
  file.read((char*)(&new_node.total_freq), sizeof(int));
  file.read((char*)(&new_node.hist_freq), sizeof(int));
  file.read((char*)(&new_node.range.first), sizeof(int));
  file.read((char*)(&new_node.range.second), sizeof(int));
  file.read((char*)(&new_node.hist_val), sizeof(int));
  size_t hist_size;
  int key;
  double value;
  file.read((char*)(&hist_size), sizeof(size_t));
  for (int j = 0; j < hist_size; j++){
    file.read((char*)(&key), sizeof(int));
    file.read((char*)(&value), sizeof(double));
    new_node.percentage[key] = value;
  }
  size_t vec_size;
  int nodeid;
  file.read((char*)(&vec_size), sizeof(size_t));
  for (int j = 0; j < vec_size; j++) {
    file.read((char*)(&nodeid), sizeof(int));
    new_node.full_nodes.push_back(nodeid);
  }
  file.read((char*)(&new_node.all_null), sizeof(double));
  curr_node_pt->children.emplace_back(new_node);
}

void deserialize_root(TreeNode &root, std::ifstream &file) {
  file.read((char*)(&root.level), sizeof(int));
  file.read((char*)(&root.id), sizeof(int));
  file.read((char*)(&root.last_flag), sizeof(bool));
  file.read((char*)(&root.freq), sizeof(int));
  file.read((char*)(&root.total_freq), sizeof(int));
  file.read((char*)(&root.hist_freq), sizeof(int));
  file.read((char*)(&root.range.first), sizeof(int));
  file.read((char*)(&root.range.second), sizeof(int));
  file.read((char*)(&root.hist_val), sizeof(int));
  size_t hist_size;
  int key;
  double value;
  file.read((char*)(&hist_size), sizeof(size_t));
  std::cout << "hist_size: " << hist_size << "\n";
  for (int j = 0; j < hist_size; j++){
    file.read((char*)(&key), sizeof(int));
    file.read((char*)(&value), sizeof(double));
    root.percentage[key] = value;
  }
  size_t vec_size;
  int nodeid;
  file.read((char*)(&vec_size), sizeof(size_t));
  std::cout << "vec_size: " << vec_size << "\n";
  for (int j = 0; j < vec_size; j++) {
    file.read((char*)(&nodeid), sizeof(int));
    root.full_nodes.push_back(nodeid);
  }
  file.read((char*)(&root.all_null), sizeof(double));
}

void deserialize(TreeNode &root, std::ifstream &file, int &node_num) {
  auto curr_node_pt = &root;
  int curr_level = 0;
  std::cout << "node_num " << node_num << "\n";
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
  int keep_num = atoi(argv[2]);
  std::string folder_name = "./partition_" + std::to_string(partition_num) + "_" + std::to_string(keep_num);
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

  std::vector<TreeNode> settrie(partition_num);
  std::string treefilename = folder_name + "/settrie.txt";
  std::ifstream treefile(treefilename, std::ios::in|std::ios::binary);
  for (int i = 0; i < partition_num; i++) {
    int node_num;
    treefile.read((char*)(&node_num), sizeof(int));
    dis_num[i] = node_num;
    std::cout << node_num << "\n";
    deserialize(settrie[i], treefile, node_num);
  }



  /*
  std::string mapfilename = folder_name + "/dis_set_map.txt";
  std::ifstream mapfile(mapfilename);
  //std::string freqfilename = "/frequency.txt";
  //std::ifstream freqfile(freqfilename);
  int u, v;
  int set_size;
  std::vector<int> max_length(partition_num, 0);
  std::vector<TreeNode> settrie(partition_num, TreeNode());
  std::vector<std::vector<int>> frequency(partition_num, std::vector<int>());
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
      settrie_insert(settrie[partid], set_key);
      if (set_size > max_length[partid]) {
        max_length[partid] = set_size;
      } 
    }
    int freq;
    for (int i = 0; i < u; i++) {
      freqfile >> freq;
      frequency[partid].push_back(freq);
    }
  }
  std::vector<Setvector> set_vector(partition_num, Setvector()); 
  for (int i = 0; i < partition_num; i++) {
    int init_value = 0;
    std::vector<int> curr_set;
    dfs_addvalue(settrie[i], init_value, frequency[i], 0);
    search_lowfreq(settrie[i]);
  }
  */
  std::cout << "cp2\n";
  std::unordered_map<int, std::string> Q_TYPE = {{0, "subset"}, {1, "superset"}, {2, "all"}};
  int query_type = atoi(argv[3]);
  std::ifstream sqlfile("../geotweet_query_5.csv");
  std::unordered_map<std::string, int> coltype = {{"geotweet.createtime",0}, {"geotweet.lat",0}, {"geotweet.lon",0}, {"geotweet.country",1}};
  int query_count = 0;
  int lineidx = 0;
  std::ofstream outsqlfile(folder_name + "/geotweet_query_5_trans_settrie_" + Q_TYPE[query_type] + "_approx.sql");
  std::ofstream outcsvfile(folder_name + "/geotweet_query_5_trans_settrie_" + Q_TYPE[query_type] + "_approx.csv");
  std::ofstream postgresfile(folder_name + "/geotweet_query_5_trans_settrie_" + Q_TYPE[query_type] + "_approx_postgres.sql");
  std::ofstream percentfile(folder_name + "/geotweet_query_5_trans_settrie_" + Q_TYPE[query_type] + "_approx_percent.csv");
  std::ofstream narupercentfile(folder_name + "/geotweet_query_5_trans_settrie_" + Q_TYPE[query_type] + "_approx_narupercent.csv");
  auto start_time = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
  int max_sub_lenth = 0;
  int avg_sub_lenth = 0;
  std::vector<int> sub_lenth_vec(179, 0);
  std::vector<int> sub_time_vec(179, 0);
  std::vector<int> time_state;
  std::cout << "begin\n";
  while (getline(sqlfile, line)) {
    lineidx++;
    if (lineidx % 2 == query_type) {
      continue;
    }
    auto each_start_time = int(duration_cast<microseconds>(system_clock::now().time_since_epoch()).count());
    //if (lineidx > 1) { 
    //  break;
    //}
    query_count++;
    //std::cout << lineidx << "\n";
    std::stringstream s(line);
    std::string token; 
    std::string postgresstr(std::to_string(partition_num) + ":select count(*) from geotweet_settrie_" + std::to_string(partition_num) + "_" + std::to_string(keep_num) + " as geotweet where ");
    std::string sqlstr("select count(*) from geotweet_settrie_" + std::to_string(partition_num) + "_" + std::to_string(keep_num) +  " as geotweet where ");
    std::string csvstr("geotweet##");
    std::string postall("select count(*) from geotweet_settrie_" + std::to_string(partition_num) + "_" + std::to_string(keep_num) +  " as geotweet where ");
    for (int i = 0; i < partition_num; i++) {
      int val = settrie[i].children[settrie[i].children.size() - 1].range.first;
      postall += ("geotweet.tags" + std::to_string(i) + "=" + std::to_string(val) + " AND ");
    }
    std::string percentstr = "";
    std::string narupercentstr = "";
    double est_percentage = 1;
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
	    postall += (col + op + val + " AND ");
	    postgresstr += (col + op + val + " AND ");
	  } else {
	    sqlstr += (col + op + "'" + val + "' AND ");
	    postall += (col + op + "'" + val + "' AND ");
	    postgresstr += (col + op + "'" + val + "' AND ");
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
	    std::vector<double> results_freq; 
            auto in_start_time = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
	    std::sort(query_vertices[i].begin(), query_vertices[i].end());
	    double total_freq;
	    double freq;
	    if (op.compare("<=") == 0) {
	      if (query_vertices[i].size() > 0) {
	        get_all_subsets(settrie[i], query_vertices[i], 0, results, results_freq, total_freq, freq);
	      }
	      if (settrie[i].children.size() > 0) {
		auto &null_node = settrie[i].children[settrie[i].children.size() - 1];
		if (null_node.id == word2idx[pad]) {
		  results.push_back(null_node.range.first);
		  results_freq.push_back(1);
		  total_freq += null_node.freq;
		  freq += null_node.freq;
		}
	      }
	      est_percentage *= (freq / total_freq);
	      if (results.size() > 1) {
	        //std::sort(results.begin(), results.end());
   	        csvstr += ("geotweet.tags" + std::to_string(i) + ",IN,\"(");
	        sqlstr += ("geotweet.tags" + std::to_string(i) + " IN (");
		std::string setstr(std::to_string(i) + ":select count(*) from geotweet_settrie_" + std::to_string(partition_num) 
		            + "_" + std::to_string(keep_num) + " as geotweet where geotweet.tags" + std::to_string(i) + " IN (");
		narupercentstr += ("tags" + std::to_string(i) + ":");
	        for (int j = 0; j < results.size(); j++) {
		  auto &value = results[j];
		  auto &percent = results_freq[j];
	          csvstr += (std::to_string(value) + ",");
		  sqlstr += (std::to_string(value) + ",");
		  narupercentstr += std::to_string(percent) + ",";
		  if (percent > 0.9999) {
		    setstr += (std::to_string(value) + ",");
		  } else if (percent > 0) {
		    postgresfile << i << ":select count(*) from geotweet_settrie_" << partition_num << "_" << keep_num
		          <<" as geotweet where geotweet.tags" << i  << "=" << value << ";";
		    percentstr += (std::to_string(percent) + ",");
		  }
	        }
	        csvstr = csvstr.substr(0, csvstr.size() - 1) + ")\",";
	        sqlstr = sqlstr.substr(0, sqlstr.size() - 1) + ") AND ";
	        setstr = setstr.substr(0, setstr.size() - 1) + ");";
                postgresfile << setstr;
	        percentstr += "1,";
	        narupercentstr = narupercentstr.substr(0, narupercentstr.size() - 1) + ";";
	      } else if (results.size() == 1) {
	        csvstr += ("geotweet.tags" + std::to_string(i) + ",=," + std::to_string(results[0]) + ",");
	        sqlstr += ("geotweet.tags" + std::to_string(i) + "=" + std::to_string(results[0]) + " AND ");
	        std::string setstr = std::to_string(i) + ":select count(*) from geotweet_settrie_" + std::to_string(partition_num) 
			  + "_" + std::to_string(keep_num) + " as geotweet where geotweet.tags" + std::to_string(i) + "=" + std::to_string(results[0]) + ";";
	        postgresfile << setstr;
	        percentstr += (std::to_string(results_freq[0]) + ",");
	        narupercentstr += ("tags" + std::to_string(i) + ":" + std::to_string(results_freq[0]) + ";");
	      }
	    } else {
	      if (query_vertices[i].size() > 0) {
	        get_all_supersets(settrie[i], query_vertices[i], 0, results, results_freq, total_freq, freq);
		if (freq > 0) {
	          est_percentage *= (freq / total_freq);
		}
		sqlstr += "(";
		if (results.size() > 2 || (results[1] - results[0]) > 1) {
                  csvstr += "geotweet.tags" + std::to_string(i) + ",IN,\"("; 
		} else {
                  csvstr += "geotweet.tags" + std::to_string(i) + ",=,"; 
		}
                std::string setstr(std::to_string(i) + ":select count(*) from geotweet_settrie_" + std::to_string(partition_num) 
		 	  + "_" + std::to_string(keep_num) + " as geotweet where ");
	   	narupercentstr += ("tags" + std::to_string(i) + ":");
		for (int j = 0; j < results.size(); j+=2) {
		  int &lb = results[j];
		  int &ub = results[j + 1];
		  sqlstr += ("(geotweet.tags" + std::to_string(i) + ">=" + std::to_string(lb) 
				+ " AND geotweet.tags" + std::to_string(i) + "<" + std::to_string(ub)
			        + ") OR ");
		  auto &percent = results_freq[j / 2];
		  if (percent > 0.99) {
		      setstr += ("(geotweet.tags" + std::to_string(i) + ">=" + std::to_string(lb) 
				+ " AND geotweet.tags" + std::to_string(i) + "<" + std::to_string(ub)
			        + ") OR ");
		  } else {
		    postgresfile << i << ":select count(*) from geotweet_settrie_" << partition_num
			    << "_" << keep_num <<  " as geotweet where geotweet.tags" << i << ">=" << lb << " AND geotweet.tags"
			    << std::to_string(i) << "<" << ub << ";";
		    percentstr += (std::to_string(percent) + ",");
		  }
		  for (int k = lb; k < ub; k++) {
		    csvstr += (std::to_string(k) + ",");
                    narupercentstr += (std::to_string(percent) + ",");
		  }
		}
		sqlstr = sqlstr.substr(0, sqlstr.size() - 4) + ") AND ";
		if (results.size() > 2 || (results[1] - results[0]) > 1) {
		  csvstr = csvstr.substr(0, csvstr.size() - 1) + ")\",";
		} else {
		  csvstr = csvstr.substr(0, csvstr.size() - 1) + ",";
		}
		narupercentstr = narupercentstr.substr(0, narupercentstr.size() - 1) + ";";
		if (setstr[setstr.size() - 2] == 'R') {
		  setstr = setstr.substr(0, setstr.size() - 4) + ";";
		  postgresfile << setstr;
		  percentstr += "1,";
		}
	      }
	    }
	  }
          if (op.compare("<=") == 0) {
	    csvstr += ("geotweet.tags_lfword,=,0,");
	    sqlstr += ("geotweet.tags_lfword=0 AND ");
	    postgresstr += ("geotweet.tags_lfword=0 AND ");
	  }
	}
      }
    }
    std::getline(s, token);
    csvstr = csvstr.substr(0, csvstr.size() - 1) + "#" + token;
    sqlstr = sqlstr.substr(0, sqlstr.size() - 5) + ";";
    postall = postall.substr(0, postall.size() - 5) + ";";
    postgresstr = postgresstr.substr(0, postgresstr.size() - 5);
    if (narupercentstr.size() > 0) {
      narupercentstr = narupercentstr.substr(0, narupercentstr.size() - 1);
    }
    if (postgresstr[postgresstr.size() - 1] != 'w'){
      postgresfile << postgresstr << "\n";
      percentstr += "1,";
    } else {
      postgresfile << "\n";
    }
    if (query_type == 0) {
      outsqlfile << sqlstr << est_percentage << ";" << postall << "\n";
    } else {
      outsqlfile << sqlstr << est_percentage << "\n";
    }
    outcsvfile << csvstr << "\n";
    percentstr = percentstr.substr(0, percentstr.size() - 1);
    percentfile << percentstr << "\n";
    narupercentfile << narupercentstr << "\n";
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
  std::ofstream state("approx_clique_part_" + std::to_string(partition_num) + "_graph/settrie_state_superset.csv");
  std::sort(time_state.begin(), time_state.end());
  for (int i = 0; i < time_state.size(); i++) {
      state << time_state[i] << "us\n";
  }
  state.close();

  return 0;
}
