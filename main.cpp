#include <thread>
#include <algorithm>
#include <numeric>
#include <vector>

template<typename Iterator,typename T>
struct accumulate_block {
    void operator()(Iterator first,Iterator last,T& result) const {
        result = accumulate(first,last,result);
    }
};

template<typename Iterator,typename T>
T parallel_threads(Iterator first,Iterator last,T& init) {
    unsigned long length=std::distance(first,last);
    unsigned long min_per_thread=25;
    unsigned long max_threads=(length+min_per_thread-1)/min_per_thread;
    unsigned long hardware_threads=std::thread::hardware_concurrency();
    unsigned long num_threads=std::min(hardware_threads!=0?hardware_threads:2,max_threads);
    unsigned long blocks_size=length/num_threads;

    std::vector<T> results(num_threads);
    std::vector<std::thread> threads(num_threads-1);

    Iterator block_start=first;
    for(unsigned long i=0;i<num_threads-1;++i) {
        Iterator block_end=block_start;
        std::advance(block_end,blocks_size);
        threads[i]=std::thread(accumulate_block<Iterator,T>(),block_start,block_end,std::ref(results[i]));
        block_start=block_end;
    }
    accumulate_block<Iterator,T>()(block_start,last,results[num_threads-1]);
    for(auto& thread : threads) {
        thread.join();
    }
    return std::accumulate(results.begin(),results.end(),init);
}