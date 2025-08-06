#include "common.h"

#include <fstream>
#include <iostream>
#include <map>

class PageRank: public MapReduce {
public:
    size_t mat_wid;
    size_t row_dis;
    // std::vector<double> final_ranks;
    PageRank(int map_num_=1, int reduce_num_=1, size_t mat_wid_=1):
        MapReduce(map_num_, reduce_num_), mat_wid(mat_wid_){
        row_dis = mat_wid / map_num;
        // final_ranks.resize(mat_wid);
    }

    // const std::vector<double>& get_results() const {
    //     return final_ranks;
    // }

    int shuffle_func(uint64_t id) {
        // uint64_t hash = djb_hash(h);
        // printf("HASH: %d %d %d\n", hash, reduce_num, ret_id);
        return (id / row_dis) % map_num;
    }

    void map_func(void *map_data, int task_id,  int data_length) {
        uint64_t *mat_data = (uint64_t *)map_data;
        // printf("%p\n", map_data);
        size_t mat_data_len = data_length / sizeof(uint64_t);
        size_t index = 0;
        while (index < mat_data_len) {
            size_t row_len = *(mat_data+1);
            mat_data += 2;
            double *P = (double *)mat_data;
            *P = *P / row_len;
            mat_data += 1;
            if (row_len > 875712) {
                printf("[D]%ld\n", row_len);
            }

            for (int i = 0; i < row_len; i ++ ) {
                int reduce_id = shuffle_func(mat_data[i]);
                // printf("%d %d %d %d\n", row_len, task_id,  reduce_id, mat_data[i]);
                emit_intermediate(vec->at(get_vec_index(task_id, reduce_id)),  \
                    (char *)mat_data + sizeof(uint64_t) * i, sizeof(uint64_t));
                emit_intermediate(vec->at(get_vec_index(task_id, reduce_id)),  \
                    (char *)P, sizeof(double)); 
            }
            index += row_len + 3;
        }
                // printf("Map %d Reduce %d: %s\n", task_id, reduce_id, word);
    }
    
    void reduce_func(int task_id) {
        double *row_hmap = (double *)malloc(sizeof(double) * row_dis);
        for (int map_id = 0; map_id < map_num; map_id ++ ) {
            std::list<imm_data> *inter = vec->at(get_vec_index(map_id, task_id));
            std::list<imm_data>::iterator it;
            int iter_count = 0;
            
            for (it = inter->begin(); it != inter->end(); it ++) {
                uint64_t *id_arr = (uint64_t *)it->data;
                double *p_arr = (double *)it->data;
                int data_length = it->count / (sizeof(uint64_t) + sizeof(double));
                // printf("%d %ld %lf\n",task_id, *id_arr, *(p_arr+1));
                for (int i = 0; i < data_length; i ++ ) {
                    row_hmap[id_arr[2*i] % row_dis] += p_arr[2*i+1];
                }
                
            }
        }
        
        // for (size_t i = 0; i < row_dis; ++i) {
        //     // 计算全局节点ID并存入
        //     size_t global_node_id = task_id * row_dis + i;
        //     if (global_node_id < mat_wid) {
        //         final_ranks[global_node_id] = row_hmap[i];
        //     }
        // }
    }

    void splice(char **data_arr, size_t *data_dis, char *map_data, int data_length) {
        uint64_t *mat_data = (uint64_t *)map_data;
        size_t mat_data_len = data_length / sizeof(uint64_t);
        size_t index = 0;
        size_t pre_index = 0;
        // int counter = 0;
        int phase = 0;
        while (index < mat_data_len) {
            uint64_t src = mat_data[index];
            size_t row_len = mat_data[index + 1];
            // counter ++;
            index += row_len + 3;
            if (src / row_dis == phase && src / row_dis != map_num) {
                data_dis[phase] = (index - pre_index) * sizeof(uint64_t);
                data_arr[phase] = map_data + pre_index * sizeof(uint64_t);
                pre_index = index;
                phase ++;
            }
        }
    }

};

// void simple_test(int argc, char **argv) {
//     printf("--- Running Correctness Verification Test ---\n");

//     // 1. 定义测试图和预期结果
//     const int NUM_NODES = 2;
//     const double initial_rank = 1.0 / NUM_NODES; // 初始值为 0.5
//     std::vector<double> expected_ranks = {0.25, 0.75}; // PR(0)=0.25, PR(1)=0.75

//     // 2. 构造符合格式的输入数据 (workload)
//     // 记录1: [src=0, links=1, rank=0.5, dest=1] -> 4 * 8 = 32字节
//     // 记录2: [src=1, links=2, rank=0.5, dest=0, dest=1] -> 5 * 8 = 40字节
//     // 总大小 = 72字节
//     const size_t workload_size = 72;
//     char* workload = (char*)malloc(workload_size);
//     uint64_t* mem = (uint64_t*)workload;

//     // 填充记录1 (Node 0)
//     mem[0] = 0;                         // src_id
//     mem[1] = 1;                         // num_links
//     ((double*)mem)[2] = initial_rank;   // pagerank_value
//     mem[3] = 1;                         // dest_id

//     // 移动指针到记录2的起始位置
//     mem += 4; 

//     // 填充记录2 (Node 1)
//     mem[0] = 1;                         // src_id
//     mem[1] = 2;                         // num_links
//     ((double*)mem)[2] = initial_rank;   // pagerank_value
//     mem[3] = 0;                         // dest_id 1
//     mem[4] = 1;                         // dest_id 2

//     // 3. 运行MapReduce任务
//     // 使用 2个mapper, 2个reducer, 总节点数2
//     PageRank *mp = new PageRank(2, 2, NUM_NODES);
//     mp->run_mr(workload, workload_size);

//     // 4. 获取结果并进行验证
//     const auto& actual_ranks = mp->get_results();
//     bool passed = true;
//     double epsilon = 1e-9; // 用于浮点数比较的容差

//     printf("\n--- Verification Results ---\n");
//     for (int i = 0; i < NUM_NODES; ++i) {
//         printf("Node %d: Expected = %lf, Actual = %lf\n", i, expected_ranks[i], actual_ranks[i]);
//         if (std::fabs(expected_ranks[i] - actual_ranks[i]) > epsilon) {
//             passed = false;
//         }
//     }

//     if (passed) {
//         printf("\n[SUCCESS] Test Passed!\n");
//     } else {
//         printf("\n[FAILURE] Test Failed!\n");
//     }

//     // 5. 清理内存
//     free(workload);
//     delete mp;
// }

void stress_test(int argc, char **argv) {

    int map_num = atoi(argv[1]);
    int reduce_num = atoi(argv[2]);
    int max_id = atoi(argv[3]);
    int line_num = atoi(argv[4]);
    char *file_name = argv[5];

    srand(time(0));

    std::vector<uint64_t> *arr = new std::vector<uint64_t>[max_id];
    std::map<uint64_t, uint64_t> mp;

    std::ifstream myfile;
	myfile.open(file_name);
    uint64_t src, dst;
    for (int i = 0; i < line_num; i ++ ) {
        myfile >> src >> dst;
        arr[src].push_back(dst);
    }

    size_t length = ((max_id * 3) + line_num) * sizeof(uint64_t);
    char *workload = nullptr;
    posix_memalign((void **)&workload, 4096, length);
    uint64_t *mem = (uint64_t *)workload;

    for (int i = 0; i < max_id; i ++ ) {
        mem[0] = i;
        int item_len = arr[i].size();
        mem[1] = item_len;
        ((double *)mem)[2] = 0.0; //1.0 * (rand() % 100) / 100;
        for (int j = 0; j < item_len; j ++ ) {
            mem[3 + j] = arr[i][j];
        }
        mem += item_len + 3;
    }

    PageRank *pr = new PageRank(map_num, reduce_num, max_id);
    pr->run_mr(workload, length);
}

int main(int argc, char **argv) {
    stress_test(argc, argv);
    return 0;
}