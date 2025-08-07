# Pagerank Benchmark

## 使用方法
下载数据集
```shell
mkdir dataset
cd dataset
wget https://snap.stanford.edu/data/soc-LiveJournal1.txt.gz
apt install gzip
gunzip soc-LiveJournal1.txt.gz
```
测试使用的是CXL-SHM里提供的数据集，之后~~换成ATLAS pagerank里的数据集应该也没问题~~。  
ATLAS的数据集在原论文的主页上已经404了，替换为http://twitter.mpi-sws.org/links-anon.txt.gz ，包含1,963,263,821条边，54,981,152个点。脚本中设置的数据集是soc-LiveJournal1，如果替换则需要修改节点总数和边总数。

运行脚本
```shell
make
./pr_benchmark.sh [mapper/reducer 数量]
```

## 参数设置
```shell
./pagerank [mapper数量] [reducer数量] [节点总数] [边总数] [输入文件]
```
mapper数量与reducer数量需要设置为一致，节点总数即图中最大节点数，边总数等于文件行数（Pagerank是一个用于计算图中节点重要性的算法，数据集以邻接表形式给出）。