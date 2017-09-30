# compile
> make
---------------------------------------------------
# run quick sort
> ./qs

# run radix sort
> ./rs 

# run prehashed parallelism of quick sort an radix sort
> ./hs <threads_num for quicksort> <threads_num for radixsort>
- example: ./hs 100 100

# run bitonic sort
> ./bs <lower bound chunk size> <upper bound chunk size>
- example: ./bs 256 4096 
- tips: <lower bound chunk size> better be bigger than 256 for large array size; <upper bound chunk size> better be smaller than 32768

# run Gaussian elimination
> ./gau <threads_num> 
- example: ./gau 1000

# Report
[report](https://github.com/hualiu01/ParallelComputing_pthread/blob/master/Report.pdf)
