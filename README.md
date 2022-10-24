# Selectivity-estimation-for-querying-set-valued-data
##### Graph color based method
Generate element partition by graph coloring algorithm.  
Compile and run `set_map.cc` for setmap generation.  
Run `clique_trans_table.py` to generate converted dataset.  
Compile and run `color_trans_query.cc` to generate converted query with graph coloring based method.

##### Clique enumeration based method
Generate an element partition.  
Compile and run `set_map.cc` for setmap generation.  
Run `clique_trans_table.py` to generate converted dataset.  
Compile and run `clique_trans_query.cc` to generate converted query with clique enumeration based method.

##### Settrie based method
Generate an element partition.  
Compile and run `set_map.cc` for setmap and settrie generation.  
Run `clique_trans_table.py` to generate converted dataset.  
Compile and run `settrie_trans_query.cc` to generate converted query with settrie based method.

##### Settrie with histogram based method
Generate an element partition.  
Compile and run `freq_set_map.cc` for setmap and settrie with histogram generation.  
Run `clique_trans_table.py` to generate converted dataset.  
Compile and run `settrie_trans_query_hist.cc` to generate converted query with settrie-hist based method.
