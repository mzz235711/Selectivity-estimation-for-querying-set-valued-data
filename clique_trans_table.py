import pandas as pd
import numpy as np
import re
import queue
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--partnum', default=20, type=int)
parser.add_argument('--keepnum', default=20000, type=int)
args = parser.parse_args()

partnum = args.partnum 
keep_num = args.keepnum
idx_file_path = './idx.csv'
idx_file = open(idx_file_path, 'r')
idx_lines = idx_file.readlines()
idx2word = ["" for _ in range(len(idx_lines))]
word2idx = {}
folder_path = './partition_{}_{}'.format(partnum, keep_num)
#folder_path = './graph_color'
#folder_path = './color_partition_{}'.format(partnum)
#part_file_path = folder_path + '/approx_clique_part_{}.txt'.format(partnum)
part_file_path = folder_path + '/part.txt'
#part_file_path = './t_{}/approx_clique_part_{}.txt'.format(partnum, partnum)
partid = np.loadtxt(part_file_path, dtype=np.int)
partnum = max(partid) + 1
pad = 'ZZZ'
for line in idx_lines:
    w = line.split('\t')[0]
    num = int(line.split('\t')[1])
    idx2word[num] = w
    word2idx[w] = num
idx2word.append(pad)
word2idx[pad] = len(idx_lines)
df = pd.read_csv('../geotweet.csv', escapechar='\\', quotechar='"')
df['tags'].fillna('', inplace=True)
lines = df['tags']
texts = []
avglenth = 0
for line in lines:
    #line = line.lower()
    #text = re.findall('[a-zA-Z]+', line)
    text = line.split(',')
    tmp_text = []
    for t in text:
        if t not in tmp_text:
            tmp_text.append(t)
    texts.append(tmp_text)
    avglenth += len(tmp_text)
avglenth /= len(lines)
print("AvgLenth: ", avglenth)
set_map_file = open(folder_path + '/dis_set_map.txt', 'r')
set_map_lines = set_map_file.readlines()
set_map = [{} for _ in range(partnum)]
line_id = 0
for i in range(partnum):
    line = set_map_lines[line_id]
    dis_num = int(line.split(' ')[1])
    line_id += 1
    print(line_id, i, dis_num)
    for line_id in range(line_id, line_id + dis_num):
        line = set_map_lines[line_id]
        line_split = line.split(' ')
        line_split = [int(i) for i in line_split]
        key = tuple(line_split[1:-1])
        value = line_split[-1]
        set_map[i][key] = value
    #set_map[i][tuple([word2idx[pad]])] = len(set_map[i])
    line_id += 1

new_column_data = []
#frequency = []
#for i in range(partnum):
#    frequency.append([0] * len(set_map[i]))
for lineid, text in enumerate(texts):
    data = [[] for _ in range(partnum)]
    has_lfword = 0
    for word in text:
        if word in word2idx:
            wid = word2idx[word]
            pid = partid[wid]
            data[pid].append(wid)
        else:
            has_lfword = 1
    for i in range(partnum):
        if len(data[i]) == 0:
            data[i].append(word2idx[pad])
        data[i].sort()
        if tuple(data[i]) not in set_map[i]:
            print(text)
            print(lineid)
            print(i)
            print(data[i])
            exit()
        set_value = set_map[i][tuple(data[i])]
        data[i] = set_value
        #frequency[i][set_value] += 1;
    data.append(has_lfword)
    new_column_data.append(data)
new_column_data = np.array(new_column_data, dtype=np.int)
df.drop(columns=['tags'], inplace=True)
for i in range(partnum):
    col_name = 'tags' + str(i)
    df[col_name] = pd.Series(new_column_data[:,i], index=df.index)
col_name = 'tags_lfword'
df[col_name] = pd.Series(new_column_data[:,-1], index=df.index)
df.to_csv( folder_path + '/geotweet_approx_clique.csv'.format(partnum, keep_num), index=False)
#frequency_file = open("./partition_{}_{}/frequency.txt".format(partnum, keep_num), "w")
#for i in range(partnum):
#    for freq in frequency[i]:
#        frequency_file.write(str(freq) + "\n")
#frequency_file.close()
for i in range(partnum):
    print(len(set_map[i]))


    
