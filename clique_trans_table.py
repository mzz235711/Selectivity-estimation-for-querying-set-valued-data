import numpy as np
import pandas as pd
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--partnum", type=int)
parser.add_argument("--keepnum", type=int)
args = parser.parse_args()

idxfp = open("idx.csv")
lines = idxfp.readlines()
word2idx = {}
idx2word = []
pad = "ZZZ"
for line in lines:
    word = line.split(' ')[0]
    idx = int(line.split(' ')[1])
    word2idx[word] = idx
    idx2word.append(word)
word2idx[pad] = len(word2idx)
idx2word.append(pad)
partition_num = args.partnum
folder_name = "color_partition_{}".format(partition_num)
partid = np.loadtxt(folder_name + "/part.txt", dtype=int)
df = pd.read_csv("gn.csv")
setcol = "FEATURE_NAME"
setdata = df[setcol]
setmapfile = open(folder_name +  "/dis_setmap.csv")
setmaplines = setmapfile.readlines()
setmap = [{} for _ in range(partition_num)]
lineid = 0
for i in range(partition_num):
    line = setmaplines[lineid]
    pid = int(line.split(" ")[0])
    setsize = int(line.split(" ")[1])
    lineid += 1
    for _ in range(setsize):
        line = setmaplines[lineid]
        lineid += 1
        values = line.split(" ")
        key = tuple([int(w) for w in values[1:-1]])
        value = int(values[-1])
        setmap[i][key] = value

df = pd.read_csv("gn.csv")
setname = "FEATURE_NAME"
setcol = df[setname].to_list()
newcols = [[] for _ in range(partition_num)]
for line in setcol:
    words = [[] for _ in range(partition_num)]
    values = line.split(",")
    for val in values:
        idx = word2idx[val]
        pid = partid[idx]
        word[pid].append(idx)
    for i, word in enumerate(words):
        if len(word) == 0:
            word.append(word2idx[pad])
        newval = setmap[i][tuple(word)]
        newcols[i].append(newval)
df.drop(setname, inplace=True)
for i in range(partition_num):
    df[setname + str(i)] = newcols[i]
df.to_csv(folder_name + "/gn_trans_table.csv")





