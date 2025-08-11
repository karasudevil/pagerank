

path=./dataset/links.txt


sleep 1
# ./cxlmalloc-benchmark-pr $i $i 875713 5105039 /root/web.txt >> out-pr.txt
./pagerank $1 $1 32668489 1000000000 $path 1>>pr.txt 2>>err.txt



# grep "TIME" out-pr.txt
grep "TIME" pr.txt
