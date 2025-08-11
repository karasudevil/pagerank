#if !defined(COMMON_H)
#define COMMON_H

#include <list>
#include <list>
#include <vector>
#include <thread>
#include <chrono>
#include <unordered_map>
#include <cstring>





struct map_parameter {
    void *map_data;
    size_t length;
    int task_id;
};

struct reduce_parameter {
    int task_id;
};

struct imm_data {
    void *data;
    int count;
};

const static int imm_data_size = 512;
const static size_t cxl_length = 1ULL << 34;

size_t length;
int shm_id;

class MapReduce
{
public:
    int map_num;
    int reduce_num;
    // struct timeval starttime,endtime;
    
    std::vector<std::list<imm_data>*> *vec;
    inline int get_vec_index(int map_id, int reduce_id) {
        return map_id * reduce_num + reduce_id;
    }
    MapReduce(int map_num_=1, int reduce_num_=1):
        map_num(map_num_), reduce_num(reduce_num_) {
        vec = new std::vector<std::list<imm_data> *>();
        for (int i = 0; i < map_num * reduce_num; i ++) {
            vec->push_back(new std::list<imm_data>());
        }
    }
    ~MapReduce() {}

    virtual void map_func(void *map_data, int task_id,  size_t data_length){};

    virtual void reduce_func(int task_id){};

    virtual void splice(char **data_arr, size_t *data, char *map_data, size_t data_length) {};

    void emit_intermediate(std::list<imm_data> *inter, char *data, int len) {
        if (inter->empty() || inter->back().count + len + 1 > imm_data_size) {
            struct imm_data inter_en;
            inter_en.count = 0;
            inter_en.data = new char[imm_data_size];
            // printf("SHIT\n");
            inter->push_back(inter_en);
        }
        
        memcpy((char *)inter->back().data + inter->back().count, data, len);
        inter->back().count += len;
    }

    void mapper (void *arg) {

        struct map_parameter *para = (struct map_parameter*)arg;
        int task_id = para->task_id;
        void *data = para->map_data;
        size_t data_length = para->length;
        // cxl_shm *shm = para->shm;
    
        
        printf("[Mapper %d Start] %p\n", task_id, data);
        
        map_func(data, task_id,  data_length);
        return;
    }

    void reducer (void *arg) {

        struct reduce_parameter *para = (struct reduce_parameter*)arg;
        int task_id = para->task_id;

        printf("[Reducer %d Start]\n", task_id);

        reduce_func(task_id);
        return;
    }

    void run_mr(char *map_data, size_t data_length) {
        printf("[Mapper] %d, [Reducer]%d\n", map_num, reduce_num);
        std::vector<std::thread *> map_threads, reduce_threads;
    

        char **map_data_arr = (char **)malloc(sizeof(char *) * map_num);
        size_t *map_data_dis = (size_t *)malloc(sizeof(size_t) * map_num);
        printf("%ld\n", data_length); 
        splice(map_data_arr, map_data_dis, map_data, data_length);
        // map_threads = new std::thread[map_num];
        // reduce_threads = new std::thread[reduce_num];
        // gettimeofday(&starttime,NULL);
        auto start = std::chrono::system_clock::now();        

        for (int i = 0; i < map_num; i++ ) {
            map_parameter *mp = new map_parameter();
            mp->task_id = i;
            mp->length = map_data_dis[i];//(i == map_num - 1) ? final_dis : map_dis;
            mp->map_data = map_data_arr[i]; // map_data + i * map_dis;
            map_threads.push_back(new std::thread(&MapReduce::mapper, this, mp));
             
        }
        for (int i = 0; i < map_num; i ++ ) {
            map_threads[i]->join();
        }
        for (int i = 0; i < reduce_num; i++ ) {
            reduce_parameter *rd = new reduce_parameter();
            rd->task_id = i;
            reduce_threads.push_back(new std::thread(&MapReduce::reducer, this, rd));
        }
        for (int i = 0; i < reduce_num; i ++ ) {
            reduce_threads[i]->join();
        }

        auto end = std::chrono::system_clock::now();  
        std::chrono::duration<double> diff = end-start;
		printf("[TIME]%lf\n", diff.count());
    }
};


static void run_mr(MapReduce *mr) {
    std::vector<std::thread *> map_threads, reduce_threads;

    // map_threads = new std::thread[map_num];
    // reduce_threads = new std::thread[reduce_num];
    for (int i = 0; i < mr->map_num; i++ ) {
        map_parameter mp;
        mp.task_id = i;
        // mp.length = xxx;
        // mp.map_data = xxx;
        map_threads.push_back(new std::thread(&MapReduce::mapper, mr, &mp));
    }

    for (int i = 0; i < mr->reduce_num; i++ ) {
        reduce_parameter rd;
        rd.task_id = i;
        reduce_threads.push_back(new std::thread(&MapReduce::reducer, mr, &rd));
    }
}


#endif // COMMON_H
