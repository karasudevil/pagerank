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
测试使用的是CXL-SHM里提供的数据集，之后换成ATLAS pagerank里的数据集应该也没问题。

运行脚本
```shell
make
./pr_benchmark.sh
```

## 脚本相关
```shell
./pagerank [mapper数量] [reducer数量] [节点总数] [边总数] [输入文件]
```
mapper数量与reducer数量需要设置为一致，节点总数即图中最大节点数，边总数等于文件行数（Pagerank是一个用于计算图中节点重要性的算法，数据集以邻接表形式给出）。