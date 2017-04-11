# include <cstdlib>
# include <iostream>
#include <time.h>
# include <math.h>
#include <fstream>
#include <sstream>

#include <pthread.h>

using namespace std;

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
//-----------------------------------------------------------sequntial bitonic
void step_arrow(int a[], int start, int L, bool ascend){
    for(int step = L/2; step>0; step /= 2)
        for(int j = start; j<(start+L);j+=2*step)
            for(int jr = 0; jr < step; jr++)
                if ((a[j+jr]>a[j+jr+step]) == ascend)
                    swap(a[j+jr],a[j+jr+step]);
}

void bitonic_sort(int a[], int N, bool ascend){
    ofstream myfile;
    myfile.open ("timeserial.txt");
    myfile << "chunk size, time"<<endl;
    /*form bitonic sequence*/
    for(int L = 2; L < N; L *= 2){
        clock_t start,end;
        start = clock();
        for(int i=0; i < N; i += 2*L){
            step_arrow(a, i, L, ascend);
            step_arrow(a, i+L, L, !ascend);
        }
        end = clock();      
        myfile << L <<',' <<double(end-start)<< endl;
    }
    myfile.close();

    /*sort bitonic sequence*/
    step_arrow(a, 0, N, ascend); 
}
//-----------------------------------------------------------parallel bitonic
void step_arrow_parallel(int a[], int start, int L, bool ascend){
    for(int step = L/2; step>0; step /= 2) // crucial step => must be serial !
        for(int j = start; j<(start+L);j+=2*step)//parallelable
            for(int jr = 0; jr < step; jr++)//parallelbale
                if ((a[j+jr]>a[j+jr+step]) == ascend)
                    swap(a[j+jr],a[j+jr+step]);
}

struct thread_data{
    int *array;
    int thread_id;
    int L;
    bool ascend;
};
void *do_work(void *threadarg){
    struct thread_data *my_arg;
    my_arg = (struct thread_data *) threadarg;
    int start = my_arg->thread_id * my_arg->L;
    bool direction = my_arg->ascend;
    int *a = my_arg->array;

    step_arrow_parallel(a, start, my_arg->L, direction);

}
int bitonic_sort_parallel(int a[], int N, bool ascend, int L_MIN, int L_MAX){
    int thread_num_total =0;
    ofstream myfile;
    myfile.open ("timeparallel.txt");
    myfile << "chunk size, time"<<endl;
    /*form bitonic sequence*/
    for(int L = 2; L < N; L *= 2){ // crucial step => must be serial!
        if(L>=L_MIN && L<=L_MAX){
            clock_t start,end;
            start = clock();
            /*parallelize execution 1*/
            const int NUM_THREADS = N/L;
            thread_num_total += NUM_THREADS;
            struct thread_data *my_args= new thread_data[NUM_THREADS];
            pthread_t threads[NUM_THREADS];
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
            for(int i = 0; i<NUM_THREADS; i++){
                my_args[i].array = a;
                my_args[i].thread_id = i;
                my_args[i].L = L;
                my_args[i].ascend = i%2?false:true;
                pthread_create(&threads[i], &attr, do_work, (void*) &my_args[i]);
            }

            /* Wait for all threads to complete */ 
            for (int i=0; i<NUM_THREADS; i++) {
                pthread_join(threads[i], NULL);
            }
            end = clock();
            myfile << L <<"," <<double(end-start)<< endl;
        }
        else{
            clock_t start,end;
            start = clock();

            for(int i=0; i < N; i += 2*L){
                step_arrow(a, i, L, ascend);
                step_arrow(a, i+L, L, !ascend);
            }
            end = clock();  
            myfile << L<<"," <<double(end-start)<< endl;
        }
         
    }
    myfile.close();

    /*sort bitonic sequence*/
    step_arrow_parallel(a, 0, N, ascend); 

    return thread_num_total;
}


int main(int argc, char *argv[]){ // thread downlimit 0- 
    int down=0, up=0;
    stringstream convert1(argv[1]);
    convert1 >> down;
    stringstream convert2(argv[2]);
    convert2 >> up;
    const int N = pow(2,10);//10=>1 K; 13=>10K; 17=>100K; 20=>1 M ; 23=>10M
    cout<< "array length: "<< N<<endl;
    int *a = new int[N]; 
    int *b = new int[N];
    for(int i = 0; i < N; i++) a[i] = i;
    shuffle(a,N);
    for(int i = 0; i < N; i++) b[i] = a[i];

    /*call sequencial bitonic sort*/
    clock_t start,end;
    start = clock();
    bitonic_sort(a, N, true);
    end = clock();
    for(int i=0;i<8;i++) cout<<a[i]<<' ';
    cout << "sequntial bitonic uses " << double(end - start) << " us"<< endl;

    /*call parallel bitonic sort*/
    start = clock();
    int total_threads_num = bitonic_sort_parallel(b, N, true, down,up);//1024 8192 16384
    end = clock();
    for(int i=0;i<8;i++) cout<<b[i]<<' ';
    cout<<"parallel bitobic("<<total_threads_num<<") uses "<< double(end-start)<<"us" << endl;
    
}
