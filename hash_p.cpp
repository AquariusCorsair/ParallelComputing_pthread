#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <random>   
#include <pthread.h>
#include <assert.h>
#include <math.h>
#include <fstream>

using namespace std;

const int NUM = 10000000;

void shuffle(int *arr, size_t n){
    if (n > 1) {
        size_t i;
        srand(time(NULL));
        for (i = 0; i < n - 1; i++) {
          size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
          int t = arr[j];
          arr[j] = arr[i];
          arr[i] = t;
        }
    }
}

//---------hash the array to a hash table of size(NUM_THREADS+1, NUM/NUM_THREADS)
void hash_arr(int *array, int NUM_THREADS){
    // cout<<"hashing..."<<endl;
    const int rowCount = NUM_THREADS;
    const int colCount = NUM/NUM_THREADS;
    int** ary = new int*[rowCount];
    int cn[NUM_THREADS];
    for(int i = 0; i < rowCount; ++i){
        ary[i] = new int[colCount];
        cn[i] = 0;
    }
    
    for(int i=0; i<NUM; i++){
        int j = array[i]/(NUM/NUM_THREADS);
        int k = cn[j]++;
        ary[j][k] = array[i];
    }
    // cout<<"hash finished"<<endl;
    for(int i=0; i<rowCount; i++)
        for(int j=0; j<colCount; j++)
            array[i*colCount+j] = ary[i][j];
    // cout<<"restore finished"<<endl;
}

//-------------------------------------------------------Serial quicksort
void quicksort(int a[], int low, int high){
    if(low >= high)
        return;
    //partition
    int first = low;
    int last = high;
    int key = a[first];
    
    while(first < last){
        while(first < last && a[last] >= key)
            --last;
        a[first] = a[last];
        while(first < last && a[first] <= key)
            ++first;
        a[last] = a[first];    
    }
    a[first] = key;
    //recursive
    quicksort(a, low, first-1);
    quicksort(a, first+1, high);
}

//------------------------------------------------------parallel quick use prehash
struct arg_sets{
    int *arr;
    int low;
    int high;
};
void *do_work1(void *threadarg){
    struct arg_sets *my_arg;
    my_arg = (struct arg_sets *) threadarg;
    int *a = my_arg->arr;
    int low = my_arg->low;
    int high = my_arg->high;

    quicksort(a, low, high);
}
void parallel_quicksort(int *array, int left, int right, int NUM_THREADS){
    //hash the array to a hash table of size(NUM_THREADS+1, NUM/NUM_THREADS)
    hash_arr(array, NUM_THREADS);

    struct arg_sets *my_args = new arg_sets[NUM_THREADS];
    pthread_t threads[NUM_THREADS];
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for(int i=0;i<NUM_THREADS;i++){
        my_args[i].arr = array;
        my_args[i].low = i*(NUM/NUM_THREADS);
        my_args[i].high = min((i+1)*(NUM/NUM_THREADS)-1,NUM-1);

        pthread_create(&threads[i], &attr, do_work1, (void*) &my_args[i]);
    }
    /* Wait for all threads to complete */ 
    for (int i=0; i<NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
}


//-----------------------------------------------------sequential radix
int findMax(int arr[], int number) {
    int max = arr[0];

    for (int i = 1; i < number; i++) 
    	max = max < arr[i]?arr[i]:max;

    return max;
}
void countSort(int arr[], int number, int exp) {
    int* output = new int[number];
    int count[10] = {0};

    //store count of occurences
    for (int i = 0; i < number; i++) 
    	count[(arr[i]/exp)%10]++;

    for (int i = 1; i < 10; i++) 
    	count[i] += count[i - 1];

    for (int i = number - 1; i >=0; i--) {
        output[count[(arr[i]/exp)%10] - 1] = arr[i];
        count[(arr[i]/exp)%10]--;
    }

    for (int i = 0; i < number; i++) 
    	arr[i] = output[i];
}

void radix_sort(int arr[], int N) {
    int max = findMax(arr, N);

    for (int exp = 1; max / exp > 0; exp *= 10) 
    	countSort(arr, N, exp);
}
//-----------------------------------------------------parallel radix
struct arg_sets2{
    int *arr;
    int tid;
    int N;
};
void *do_work2(void *threadarg){
    struct arg_sets2 *my_arg;
    my_arg = (struct arg_sets2 *) threadarg;
    int tid = my_arg->tid; 
    int start = tid*my_arg->N;
    int *a = &my_arg->arr[start];

    radix_sort(a,my_arg->N);
}
void parallel_radixSort(int *array, int NUM_THREADS){
    //hash the array to a hash table of size(NUM_THREADS+1, NUM/NUM_THREADS)
    hash_arr(array, NUM_THREADS);

    struct arg_sets2 *my_args = new arg_sets2[NUM_THREADS];
    pthread_t threads[NUM_THREADS];
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for(int i=0;i<NUM_THREADS;i++){   	
        my_args[i].arr = array;
        my_args[i].tid = i;
        my_args[i].N = NUM/NUM_THREADS;
        pthread_create(&threads[i], &attr, do_work2, (void*) &my_args[i]);
    }
    /* Wait for all threads to complete */ 
    for (int i=0; i<NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
}


int main(int argc, char *argv[]){
	if(argc != 3){
		cout<<"usage:"<< argv[0] << " <threads_num for quicksort> \
		<threads_num for radixsort>"<<endl;
		return -1;
	}
	stringstream convert1(argv[1]); 
	int thread_num_quicksort;
	if (!(convert1 >> thread_num_quicksort)){
		cout<<"first arg must be an interger"<<endl;
		return -1;
	} 
	stringstream convert2(argv[2]); 
	int thread_num_radixsort;
	if (!(convert2 >> thread_num_radixsort)){
		cout<<"second arg must be an interger"<<endl;
		return -1;
	} 
	cout<<"quick sort threads number: "<<thread_num_quicksort<<"\nradix sort threads number: "<<thread_num_radixsort<<endl;

    int *a = new int[NUM]; 
    int *b = new int[NUM];
    for(int i = 0; i < NUM; i++) a[i] = i;
    shuffle(a,NUM);
	int *a1 = new int[NUM];
    for(int i = 0; i < NUM; i++){
    	b[i] = a[i];
    	a1[i] = a[i];
    } 

    clock_t start,end;

    /*call sequencial quick sort*/
    start = clock();
    quicksort(a, 0, NUM-1);
    end = clock();
    for(int i = 0; i < 10; i++) cout<< a[i] << " ";
    cout << "sequntial quick sort uses " << double(end - start) << " us"<< endl;
    /*call parallel quick sort*/
    start = clock();
    parallel_quicksort(b, 0, NUM-1, thread_num_quicksort);
    end = clock();
    for(int i = 0; i < 10; i++) cout<< b[i] << " ";
    cout << "parallel quick sort with hash uses " << double(end - start) << " us"<< endl;
    

    /*call sequencial radix sort*/
	int *b1 = new int[NUM];
    for(int i = 0; i < NUM; i++) b1[i] = a1[i];
    start = clock();
    radix_sort(a1,NUM);
	end = clock();
 	for(int i = 0; i < 10; i++) cout<< a1[i] << " ";
    cout << "sequencial radix sort uses " << double(end - start) << " us"<< endl;
    /*call parallel radix sort*/
    start = clock();
    parallel_radixSort(b1, thread_num_radixsort);
    end = clock();
    for(int i = 0; i < 10; i++) cout<< b1[i] << " ";
    cout << "parallel radix sort with hash uses " << double(end - start) << " us"<< endl;   
    
    return 0;
}
