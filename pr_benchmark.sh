

path=./dataset/soc-LiveJournal1.txt


sleep 1
# ./cxlmalloc-benchmark-pr $i $i 875713 5105039 /root/web.txt >> out-pr.txt
./pagerank 2 2 4847571 68993773 $path 1>>pr.txt 2>>err.txt



# grep "TIME" out-pr.txt
grep "TIME" pr.txt
