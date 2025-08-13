# Pagerank Benchmark

这是一个内存密集型的 PageRank 基准测试程序，修改自 [hhyx/CXL-SHM](https://github.com/hhyx/CXL-SHM) 中的 pagerank benchmark。

此项目的主要功能是读取一个大型有向图（以边列表形式），并对所有节点执行一次模拟的 pagerank 迭代（即对每个节点的 PageRank 值进行一次除法计算）。由于其需要将整个图结构和节点数据载入内存，因此它是一个内存密集型的 benchmark。

## 主要特性

* **内存密集型**: 旨在通过大规模图数据处理来给内存子系统带来压力。
* **数据预处理**: 提供 Python 脚本，可将非紧凑的图节点 ID（如 Twitter UID）转换为从 0 开始的连续整数 ID。
* **大数据集适配**: 修复了原benchmark中对于大型数据集处理失败的问题。
* **负载均衡**: 重写了原benchmark中对于mapper的分组逻辑，使其更能有效利用多线程，实现负载均衡。


## 环境依赖

* 支持 C++11 的 **GCC** 编译器
* **GNU Make** 4.1 或更高版本
* **Python** (用于数据预处理脚本)
    * Python 包: `tqdm` (用于显示处理进度)
        ```shell
        pip install tqdm
        ```

## 使用方法

### 第一步：编译

克隆本仓库后，直接运行 `make` 命令即可编译生成 `pagerank` 可执行文件。

```shell
make
```

### 第二步：准备数据集

Pagerank 算法需要一个以有向边列表为格式的图数据集。可以选择以下两种方式之一来准备数据。

---

#### **选项 A：下载并自行预处理数据集**

1.  **下载原始数据集** (以 Twitter 数据集为例)
    ```shell
    mkdir -p dataset
    cd dataset
    wget [http://twitter.mpi-sws.org/links-anon.txt.gz](http://twitter.mpi-sws.org/links-anon.txt.gz)
    gunzip links-anon.txt.gz
    ```
    原始数据集大小为 10GB，解压后约 37GB。

2.  **预处理数据**
    由于原始数据集的节点编号是离散的 Twitter 用户 UID，需要使用提供的 `preprocess.py` 脚本将其转换为从 0 开始的连续编号，以便程序高效读取。

    ```shell
    # 语法: python preprocess.py [源文件] -p [线程数] -o [输出文件名] -m [节点映射文件名]
    python preprocess.py links-anon.txt -p 8 -o links-anon.processed.txt -m links-anon.map.txt
    ```
    * `-p`: 指定用于加速处理的线程数。
    * `-o`: (可选) 指定处理后输出的文件名。
    * `-m`: (可选) 指定原始节点 ID 与新 ID 的映射关系文件。

---

#### **选项 B：使用已处理好的数据集**

为了方便测试，提供了一个已经预处理好的数据集，该数据集截取了 `links-anon.txt` 的前 10 亿行数据。

> **数据集信息:**
> * **位置**: CloudLab 共享文件夹 `/proj/farmemory-PG0/dataset`
> * **大小**: 17 GB
> * **节点数**: 32,668,489
> * **边数**: 1,000,000,000

---

### 第三步：运行 Benchmark

#### **推荐方式：使用脚本**

项目提供了一个便捷的运行脚本 `pr_benchmark.sh`。您只需提供 mapper/reducer 的数量即可。

```shell
# 示例：使用 8 个 mapper/reducer
./pr_benchmark.sh 8
```
*注意：此脚本默认使用了预处理好的数据集路径和参数，您可能需要根据实际情况修改脚本内容。*

#### **手动执行**

也可以直接调用 `pagerank` 可执行文件，并手动指定所有参数。

```shell
# 语法: ./pagerank [mapper数量] [reducer数量] [节点总数] [边总数] [输入文件]
./pagerank 8 8 32668489 1000000000 ./dataset/links-preprocessed.txt
```

**参数说明:**

| 参数 | 说明 | 注意事项 |
| :--- | :--- | :--- |
| `mapper数量` | Map 任务的并发数量。 | |
| `reducer数量`| Reduce 任务的并发数量。 | **必须**与 `mapper数量` 设置为相同的值。 |
| `节点总数` | 图中唯一节点的总数 (预处理后获得)。 | 即图中最大的节点ID + 1。 |
| `边总数` | 图中边的总数。 | 等于输入文件的总行数。 |
| `输入文件` | 预处理后的数据集文件路径。 | |