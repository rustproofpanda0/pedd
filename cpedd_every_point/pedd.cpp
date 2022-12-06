#include <iostream>
#include <random>
#include <algorithm>
#include <array>
#include <chrono>


extern "C"{
// void cpedd(int *np_sample_ptr, int *np_field_ptr,
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

/*
velocities on boundaries:

        3  2  1
         \ | /
       4 -- -- 0
         / | \
        5  6  7

left  : x = 0   , y = :     v = { 1, 0, 7 }
right : x = xmax, y = :     v = { 3, 4, 5 }
lower : x = :,    y = 0     v = { 1, 2, 3 }
upper : x = :,    y = ymax  v = { 5, 6, 7 }
*/




class NpArr {
  private:
    int *arr_ptr;
    int x_int_stride;
    int y_int_stride;
    int size;



    int sw;





  public:
    int xs;
    int ys;
    NpArr(int*, int, int, int, int, int);
    int getitem(int, int);
    void setitem(int, int, int);




    void print_switch();




};

NpArr::NpArr(int *arr, int x_size, int y_size, int x_stride, int y_stride, int arr_size) {
    arr_ptr = arr;
    xs = x_size;
    ys = y_size;
    x_int_stride = x_stride / sizeof(int);
    y_int_stride = y_stride / sizeof(int);
    size = arr_size;

    // std::cout << x_stride<<' '<< y_stride << '\n';
    // std::cout << x_int_stride<<' '<< y_int_stride << '\n';

};

void NpArr::print_switch(){
    sw = 1;
};

int NpArr::getitem(int i, int j) {
    // int idx = x_int_stride * i + y_int_stride * j;
    int idx = (x_int_stride * i) + (y_int_stride * j);



    if(sw == 1){
        std::cout << x_int_stride <<' '<< i <<' '<< y_int_stride <<' '<< j <<'\n';
    };



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





// void cpedd(int *np_sample_ptr, int *np_field_ptr,
void cpedd(int *np_sample_ptr, int *np_field_ptr, int *x,
           int x_size, int y_size, int x_stride, int y_stride, int size,
           int num_iter){

    double calc_K(NpArr& sample, NpArr& field, int N_sample, int N_field);
    // void make_middle_step(NpArr& sample, NpArr& field, int *x, int N_sample, int* N_field,
    //                       std::array<int, 8>& v, double* K);
    // void make_boundary_step(NpArr& sample, NpArr& field, int *x, int N_sample, int* N_field,
    //                         std::array<int, 5>& v, double* K);
    // void make_corner_step(NpArr& sample, NpArr& field, int *x, int N_sample, int* N_field,
    //                       std::array<int, 3>& v, double* K);

    void make_step(NpArr& sample, NpArr& field, int x, int y, int N_sample, int* N_field,
               std::array<int, 8>& v, double* K, int x_size, int y_size);
    

    // std::cout << "x_size = " << x_size << '\n';

    NpArr sample(np_sample_ptr, x_size, y_size, x_stride, y_stride, size); 
    NpArr field(np_field_ptr, x_size, y_size, x_stride, y_stride, size);


    // std::cout << "field.xs = " << field.xs << '\n';



    int x_left = 0;
    int x_right = x_size - 1;
    // int x_right = x_size - 10;
    int y_lower = 0;
    int y_upper = y_size - 1;
    // int y_upper = y_size - 10;




    int N_sample = 0;
    int N_field = 0;
    // int x[2] = {x_size / 2 - 40, y_size / 2 - 40};
    // field.setitem(x[0], x[1], 1);

    for(int i = 0; i < x_size; i++) {
        for(int j = 0; j < y_size; j++) {
            N_sample = N_sample + sample.getitem(i, j);
            N_field = N_field + field.getitem(i, j);
        };
    };
    




    // sample.print_switch();
    // field.print_switch();

    // std::cout << x_right <<'\n';
    // std::cout << y_upper << '\n';


    // ((double)N_sample / (double)N_field) * field.getitem(102, 136);





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

        for(int x_coord = 0; x_coord < x_size; x_coord++){
            for(int y_coord = 0; y_coord < y_size; y_coord++){

                // std::cout << x_coord <<' '<< y_coord << '\n';

                make_step(sample, field, x_coord, y_coord, N_sample, &N_field,
                          rand_directions[iter], &K, x_size, y_size);
                N_field = N_field + 1;
            };
        };


        // make_step(sample, field, x, N_sample, &N_field,
        //           rand_directions[iter], &K, x_size, y_size);
        // N_field = N_field + 1;
    };

};


void make_step(NpArr& sample, NpArr& field, int x, int y, int N_sample, int* N_field,
               std::array<int, 8>& v, double* K, int x_size, int y_size) {


    double test_K, a_old, a_new;
    double K_new = 1e200;
    int x_new, y_new;
    int xx, yy;


    // std::cout << "func before " << field.getitem(x[0], x[1]) << '\n';

    // int x_old = x[0];
    // int y_old = x[1];


    for(int dir = 0; dir < 8; dir++) {

        xx = x + V[v[dir]][0];
        yy = y + V[v[dir]][1];

        // if(xx == 100 || xx == x_size || yy == 100 || yy == y_size) {
        //     // std::cout << x[0] << ' ' << x[1] << ' ' << '\n';
        //     continue;
        // };

        // std::cout << "====> " << xx <<' '<< yy << '\n';

        if(xx == -1) {
            // xx = x_size - 1;
            continue;
        } else if(xx >= x_size) {
            continue;
            // xx = 0;
        };
        if(yy == -1) {
            continue;
            // yy = y_size - 1;
        } else if(yy >= y_size) {
            continue;
            // yy = 0;
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
            x_new = xx;
            y_new = yy;
        };
    };

    // x[0] = x_new[0];
    // x[1] = x_new[1];

    *K = K_new;
    field.setitem(x_new, y_new, field.getitem(x_new, y_new) + 1 );

};


/*

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


void make_boundary_step(NpArr& sample, NpArr& field, int *x, int N_sample, int* N_field,
                        std::array<int, 3>& v, double* K) {
    
    double calc_K(NpArr& sample, NpArr& field, int N_sample, int N_field);

    double a_old, a_new;
    double K_new = 1e200;
    double NsNf = (double)N_sample / (double)((*N_field) + 1);
    int x_new[2];
    int xx, yy;

    double test_K = calc_K(sample, field, N_sample, ((*N_field) + 1));
    

    for(int dir = 0; dir < 3; dir++) {

        xx = x[0] + V[v[dir]][0];
        yy = x[1] + V[v[dir]][1];

        // std::cout << "---------------" <<'\n';
        // std::cout << "v[dir] = " << v[dir] << " V[v[dir]][0] = " << V[v[dir]][0] << '\n';
        // std::cout << "v[dir] = " << v[dir] << " V[v[dir]][1] = " << V[v[dir]][1] << '\n';
        // std::cout << "x[0] = " << x[0] << " x[1] = " << x[1] << "x = " << xx << " y = " << yy << '\n';

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


void make_corner_step(NpArr& sample, NpArr& field, int *x, int N_sample, int* N_field,
                      int v, double* K) {
    
    double calc_K(NpArr& sample, NpArr& field, int N_sample, int N_field);

    double a_old, a_new;
    double NsNf = (double)N_sample / (double)((*N_field) + 1);
    double test_K = calc_K(sample, field, N_sample, ((*N_field) + 1));
    
    x[0] = x[0] + V[v][0];
    x[1] = x[1] + V[v][1];

    a_old = ( sample.getitem(x[0], x[1]) - NsNf * field.getitem(x[0], x[1]) );
    if(a_old <= 0.0){
        a_old = 0.0;
    };

    a_new = sample.getitem(x[0], x[1]) - NsNf * (field.getitem(x[0], x[1]) + 1);
    if(a_new <= 0.0){
        a_new = 0.0;
    };

    *K = test_K - a_old + a_new;

    *N_field = *N_field + 1;
    field.setitem(x[0], x[1], field.getitem(x[0], x[1]) + 1 );

};

*/



  // N ~ N + 1 approximation 

// void make_middle_step(NpArr& sample, NpArr& field, int *x, int N_sample, int* N_field,
//                      std::array<int, 8>& v, double* K) {
    
//     double test_K, a_old, a_new;
//     double K_new = 1e200;
//     int x_new[2];
//     int xx, yy;


//     // std::cout << "func before " << field.getitem(x[0], x[1]) << '\n';

//     // int x_old = x[0];
//     // int y_old = x[1];


//     for(int dir = 0; dir < 8; dir++) {

//         xx = x[0] + V[v[dir]][0];
//         yy = x[1] + V[v[dir]][1];

//         a_old = ( sample.getitem(xx, yy) - ((double)N_sample / (double)*N_field) * field.getitem(xx, yy) );
//         if(a_old <= 0.0){
//             a_old = 0.0;
//         };

//         a_new = sample.getitem(xx, yy) - ((double)N_sample / (double)*N_field) * (field.getitem(xx, yy) + 1);
//         if(a_new <= 0.0){
//             a_new = 0.0;
//         }

//         test_K = *K - a_old + a_new;

//         if(test_K < K_new){
//             K_new = test_K;
//             x_new[0] = xx;
//             x_new[1] = yy;
//         };
//     };


//     // std::cout << "func 111 before " << field.getitem(x[0], x[1]) << '\n';

    



//     x[0] = x_new[0];
//     x[1] = x_new[1];

//     // std::cout << N_field << ' ';

//     // *N_field = *N_field + 1;
//     // N_field++;

//     // std::cout << N_field << '\n';

//     // std::cout << "===" << field.getitem(x[0], x[1]) << ' ';

//     *K = K_new;
//     field.setitem(x[0], x[1], field.getitem(x[0], x[1]) + 1 );


//     // std::cout << "func after " << field.getitem(x_old, y_old) << '\n';


//     // std::cout << "func field.xs" << field.xs << '\n';

//     // std::cout << "func" << field.getitem(x[0], x[1]) << '\n';

// };


// void make_boundary_step(NpArr& sample, NpArr& field, int *x, int N_sample, int* N_field,
//                         std::array<int, 5>& v, double* K) {
    
//     double test_K, a_old, a_new;
//     double K_new = 1e200;
//     int x_new[2];
//     int xx, yy;

//     for(int dir = 0; dir < 5; dir++) {

//         xx = x[0] + V[v[dir]][0];
//         yy = x[1] + V[v[dir]][1];

//         a_old = ( sample.getitem(xx, yy) - ((double)N_sample / (double)*N_field) * field.getitem(xx, yy) );
//         if(a_old <= 0.0){
//             a_old = 0.0;
//         };

//         a_new = sample.getitem(xx, yy) - ((double)N_sample / (double)*N_field) * (field.getitem(xx, yy) + 1);
//         if(a_new <= 0.0){
//             a_new = 0.0;
//         }

//         test_K = *K - a_old + a_new;


//         if(test_K < K_new){
//             K_new = test_K;
//             x_new[0] = xx;
//             x_new[1] = yy;
//         };
//     };

//     x[0] = x_new[0];
//     x[1] = x_new[1];

//     *N_field = *N_field + 1;
//     *K = K_new;

//     field.setitem(x[0], x[1], field.getitem(x[0], x[1]) + 1 );
    
// };


// void make_corner_step(NpArr& sample, NpArr& field, int *x, int N_sample, int* N_field,
//                         std::array<int, 3>& v, double* K) {
    
//     double test_K, a_old, a_new;
//     double K_new = 1e200;
//     int x_new[2];
//     int xx, yy;

//     for(int dir = 0; dir < 3; dir++) {

//         xx = x[0] + V[v[dir]][0];
//         yy = x[1] + V[v[dir]][1];

//         a_old = ( sample.getitem(xx, yy) - ((double)N_sample / (double)*N_field) * field.getitem(xx, yy) );
//         if(a_old <= 0.0){
//             a_old = 0.0;
//         };

//         a_new = sample.getitem(xx, yy) - ((double)N_sample / (double)*N_field) * (field.getitem(xx, yy) + 1);
//         if(a_new <= 0.0){
//             a_new = 0.0;
//         }

//         test_K = *K - a_old + a_new;


//         if(test_K < K_new){
//             K_new = test_K;
//             x_new[0] = xx;
//             x_new[1] = yy;
//         };
//     };

//     x[0] = x_new[0];
//     x[1] = x_new[1];

//     *N_field = *N_field + 1;
//     *K = K_new;

//     field.setitem(x[0], x[1], field.getitem(x[0], x[1]) + 1 );
    
// };



// // void make_corner_step(NpArr& sample, NpArr& field, int *x, int N_sample, int* N_field,
// //                       int v, double* K) {
    
// //     double calc_K(NpArr& sample, NpArr& field, int N_sample, int N_field);

// //     double a_old, a_new;
// //     double NsNf = (double)N_sample / (double)((*N_field) + 1);
// //     double test_K = calc_K(sample, field, N_sample, ((*N_field) + 1));
    
// //     x[0] = x[0] + V[v][0];
// //     x[1] = x[1] + V[v][1];

// //     a_old = ( sample.getitem(x[0], x[1]) - NsNf * field.getitem(x[0], x[1]) );
// //     if(a_old <= 0.0){
// //         a_old = 0.0;
// //     };

// //     a_new = sample.getitem(x[0], x[1]) - NsNf * (field.getitem(x[0], x[1]) + 1);
// //     if(a_new <= 0.0){
// //         a_new = 0.0;
// //     };

// //     *K = test_K - a_old + a_new;

// //     *N_field = *N_field + 1;
// //     field.setitem(x[0], x[1], field.getitem(x[0], x[1]) + 1 );

// // };




// double calc_K(NpArr *sample, NpArr *field, int N_sample, int N_field, int x_size, int y_size) {
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

// int make



