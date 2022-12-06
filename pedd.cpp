#include <iostream>
#include <random>
#include <algorithm>
#include <array>
#include <chrono>


extern "C"{
void cpedd(int *np_sample_ptr, int *np_field_ptr, int *x,
           int x_size, int y_size, int x_stride, int y_stride, int size,
           int num_iter);
}

int V[8][2] = {{ 1,  0},    // 0
               { 1,  1},    // 1
               { 0,  1},    // 2
               {-1,  1},    // 3
               {-1,  0},    // 4
               {-1, -1},    // 5
               { 0, -1},    // 6
               { 1, -1}};   // 7

class NpArr {
  private:
    int *arr_ptr;
    int x_int_stride;
    int y_int_stride;
    int size;

  public:
    int xs;
    int ys;
    NpArr(int*, int, int, int, int, int);
    int getitem(int, int);
    void setitem(int, int, int);




};

NpArr::NpArr(int *arr, int x_size, int y_size, int x_stride, int y_stride, int arr_size) {
    arr_ptr = arr;
    xs = x_size;
    ys = y_size;
    x_int_stride = x_stride / sizeof(int);
    y_int_stride = y_stride / sizeof(int);
    size = arr_size;
};

int NpArr::getitem(int i, int j) {
    int idx = (x_int_stride * i) + (y_int_stride * j);

    if(idx > size){
        std::cout << ">>> cpp error in NpArr::getitem : " << "index " << idx;
        std::cout << " is out of bounds of array with size " << size << '\n';
        throw idx;
    };
    return arr_ptr[idx];
};

void NpArr::setitem(int i, int j, int val){
    int idx = x_int_stride * i + y_int_stride * j;
    if(idx > size){
        std::cout << ">>> cpp error in NpArr::getitem : " << "index " << idx;
        std::cout << " is out of bounds of array with size " << size << '\n';
        throw idx;
    };
    arr_ptr[idx] = val;
};


void cpedd(int *np_sample_ptr, int *np_field_ptr, int *x,
           int x_size, int y_size, int x_stride, int y_stride, int size,
           int num_iter){

    double calc_K(NpArr& sample, NpArr& field, int N_sample, int N_field);

    void make_step(NpArr& sample, NpArr& field, int *x, int N_sample, int* N_field,
                     std::array<int, 8>& v, double* K, int x_size, int y_size);


    NpArr sample(np_sample_ptr, x_size, y_size, x_stride, y_stride, size); 
    NpArr field(np_field_ptr, x_size, y_size, x_stride, y_stride, size);

    int x_left = 0;
    int x_right = x_size - 1;
    int y_lower = 0;
    int y_upper = y_size - 1;

    int N_sample = 0;
    int N_field = 0;

    for(int i = 0; i < x_size; i++) {
        for(int j = 0; j < y_size; j++) {
            N_sample = N_sample + sample.getitem(i, j);
            N_field = N_field + field.getitem(i, j);
        };
    };
    
    std::array<int, 8> rng = {0, 1, 2, 3, 4, 5, 6, 7};
    std::array<int, 8> *rand_directions;
    rand_directions = new std::array<int, 8> [num_iter];
    unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();

    for(int i = 0; i < num_iter; i++) {
        shuffle(rng.begin(), rng.end(), std::default_random_engine(seed));
        for(int j = 0; j < 8; j++) {
            rand_directions[i][j] = rng[j];
        };
    };

    double K = calc_K(sample, field, N_sample, N_field);


    for(int iter = 0; iter < num_iter; iter++) {
        make_step(sample, field, x, N_sample, &N_field,
                         rand_directions[iter], &K, x_size, y_size);
        N_field = N_field + 1;
    };
};



/*

// general case

void make_middle_step(NpArr& sample, NpArr& field, int *x, int N_sample, int* N_field,
                     std::array<int, 8>& v, double* K) {


    // x[0] = 1000;
    
    double calc_K(NpArr& sample, NpArr& field, int N_sample, int N_field);

    double a_old, a_new;
    double K_new = 1e200;
    double NsNf = (double)N_sample / (double)((*N_field) + 1);
    int x_new[2];
    int xx, yy;

    double test_K = calc_K(sample, field, N_sample, ((*N_field) + 1));
    

    for(int dir = 0; dir < 8; dir++) {

        xx = x[0] + V[v[dir]][0];
        yy = x[1] + V[v[dir]][1];

        a_old = ( sample.getitem(xx, yy) - NsNf * field.getitem(xx, yy) );
        if(a_old <= 0.0){
            a_old = 0.0;
        };
        test_K = test_K - a_old;

        a_new = sample.getitem(xx, yy) - NsNf * (field.getitem(xx, yy) + 1);
        if(a_new <= 0.0){
            a_new = 0.0;
        };
        test_K = test_K + a_new;

        if(test_K < K_new){
            K_new = test_K;
            x_new[0] = xx;
            x_new[1] = yy;
        };
    };

    x[0] = x_new[0];
    x[1] = x_new[1];

    *N_field = *N_field + 1;
    *K = K_new;
    field.setitem(x[0], x[1], field.getitem(x[0], x[1]) + 1 );

};

*/



  // N ~ N + 1 approximation 

void make_step(NpArr& sample, NpArr& field, int *x, int N_sample, int* N_field,
                     std::array<int, 8>& v, double* K, int x_size, int y_size) {
    
    double test_K, a_old, a_new;
    double K_new = 1e200;
    int x_new[2];
    int xx, yy;

    for(int dir = 0; dir < 8; dir++) {

        xx = x[0] + V[v[dir]][0];
        yy = x[1] + V[v[dir]][1];

        if(xx == -1) {
            xx = x_size - 1; // periodic boudaries
            // continue;     // bounce-back boundaries
        } else if(xx >= x_size) {
            xx = 0;
            // continue;
        };
        if(yy == -1) {
            // continue;
            yy = y_size - 1;
        } else if(yy >= y_size) {
            // continue;
            yy = 0;
        };

        a_old = ( sample.getitem(xx, yy) - ((double)N_sample / (double)*N_field) * field.getitem(xx, yy) );
        if(a_old <= 0.0){
            a_old = 0.0;
        };

        a_new = sample.getitem(xx, yy) - ((double)N_sample / (double)*N_field) * (field.getitem(xx, yy) + 1);
        if(a_new <= 0.0){
            a_new = 0.0;
        }

        test_K = *K - a_old + a_new;

        if(test_K < K_new){
            K_new = test_K;
            x_new[0] = xx;
            x_new[1] = yy;
        };
    };

    x[0] = x_new[0];
    x[1] = x_new[1];

    *K = K_new;
    field.setitem(x[0], x[1], field.getitem(x[0], x[1]) + 1 );
};

double calc_K(NpArr& sample, NpArr& field, int N_sample, int N_field) {

    double K = 0;
    double a;
    for(int i = 0; i < sample.xs; i++){
        for(int j = 0; j < sample.ys; j++){
            a = sample.getitem(i, j) - ((double)N_sample / (double)N_field) * field.getitem(i, j);
            if(a > 0){
                K = K + a;
            };
        };
    };
    return K;
};


