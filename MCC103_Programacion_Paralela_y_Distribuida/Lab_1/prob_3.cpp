#include <cstdlib>
#include <iostream>
#include <stdlib.h>
#include <omp.h>
#include <fstream>

#define IMAGE_LENGTH 16
#define KERNEL_LENGTH 3
#define MAX_NUMBER 12
#define length(x) (sizeof(x)/sizeof(x[0]))

using namespace std;
ofstream fs("prob_3.txt");

#define COMMAND(NAME)  { #NAME, NAME ## _command }

/*

g++ -std=c++1z prob_3.cpp -fopenmp

struct command{
  char *name;
  void (*function) (void);
};

struct command commands[] = {
  { "quit", quit_command }, { "help", help_command },
  COMMAND (exit), COMMAND (hello)
};

*/

void print_Matrix(int** matrix, int n, int m){
  for(int i = 0; i < n; i++){
    for(int j = 0; j < m; j++){
      cout<<" "<<matrix[i][j];            
    }
    cout<<endl;
  }
}

void print_Part_Matrix(int** matrix, int a, int b, int n, int m){
  for(int i = a; i < n; i++){
    for(int j = b; j < m; j++){
      cout<<" "<<matrix[i][j];            
    }
    cout<<endl;
  }
}

int** create_Matrix(int n, int m){
  int **matrix;
  //allocate space for values at once, so as to be contiguous in memory
  matrix = (int **)malloc(n*sizeof(int *));
  matrix[0] = (int *)malloc(m*sizeof(int));
  for(int i=1; i < n; i++) {
      //matrix[i]=&matrix[0][m*i];
      matrix[i] = (int *)malloc(m*sizeof(int));
  }
  return matrix;
}

void generate_Kernel_Matrix(int** a, int n, int m){
  srand( (unsigned)time( NULL ) );
 
  for(int i=0; i<n;i++){
    for(int j=0;j<m;j++){
      int random_number = (rand()%MAX_NUMBER)+1;
      //a[i][j] = random_number;
      a[i][j] = 1;
    }
  }
}

void generate_Image_Matrix(int** a, int n, int m, int index){
  srand( (unsigned)time( NULL ) );
 
  for(int i=index; i<n-index;i++){
    for(int j=index;j<m-index;j++){
      int random_number = (rand()%MAX_NUMBER)+1;
      a[i][j] = random_number;
    }
  }
}

void init_Matrix(int** a,int n, int m){
  for(int i=0; i<n;i++){
    for(int j=0;j<m;j++){
      a[i][j]=0;
    }
  }
}

bool compare_Matrix(int** A, int** B, int n, int m){
  bool same = true;
  for(int i=0; i<n;i++){
    for(int j=0;j<m;j++){
      if(A[i][j]!=B[i][j]){
        same = false;
        break;
      }
    }
  }
  return same;
}

void convolucionOMP(int num_threads, int** kernel, int** image, int** result, int numThreads){
  omp_set_num_threads(num_threads);
  int mitad, n, m;
  double tStart, tEnd;
  // Hallar la mitad del kernel para posicionar la matriz desde ahi
  mitad = KERNEL_LENGTH / 2;
  // Image
  tStart = omp_get_wtime();
# pragma omp parallel shared(result, mitad)
  {
    int th_id = omp_get_thread_num();
    int init = th_id*IMAGE_LENGTH/num_threads+KERNEL_LENGTH/2;
    int end = (th_id+1)*IMAGE_LENGTH/num_threads;

#   pragma omp parallel for private(n,m) schedule(static, 2)
      for (int i = init; i < end; ++i){
        for (int j = KERNEL_LENGTH/2; j < IMAGE_LENGTH; ++j){
          // Variable acumuladora
          int acumulador = 0;
          int contador = 0;
          for (int n = 0; n < KERNEL_LENGTH; ++n){
            // Indice de la fila del kernel alrevez
            int nn = KERNEL_LENGTH - 1 - n;
            for (int m = 0; m < KERNEL_LENGTH; ++m){
              // Indice de la columna del kernel alrevez
              int mm = KERNEL_LENGTH - 1 - m;
              int ii = i + (n - mitad);
              int jj = j + (m - mitad);
              contador+=kernel[nn][mm];
              acumulador += image[ii][jj] * kernel[nn][mm];
            }
          }
          result[i-KERNEL_LENGTH/2][j-KERNEL_LENGTH/2] = acumulador/contador;
        }
      }
  }
}

void convolucion(int** kernel, int** image, int** result, int index){
  int mitad, n, m;
  // Hallar la mitad del kernel para posicionar la matriz desde ahi
  mitad = KERNEL_LENGTH / 2;
  // Image
  for (int i = index; i < IMAGE_LENGTH; ++i){
    for (int j = index; j < IMAGE_LENGTH; ++j){
      // Variable acumuladora
      int acumulador = 0;
      int contador = 0;
      /*if((i>=KERNEL_LENGTH && i<(IMAGE_LENGTH - KERNEL_LENGTH)) && (j>=KERNEL_LENGTH && j < (IMAGE_LENGTH - KERNEL_LENGTH))){
        n = (i - KERNEL_LENGTH)%KERNEL_LENGTH;
        m = (j - KERNEL_LENGTH)%KERNEL_LENGTH;
        cout << n << " " << m << endl;
        acumulador += image[i][j]*kernel[n][m];
      }*/
      // Kernel
      for (int n = 0; n < KERNEL_LENGTH; ++n){
        // Indice de la fila del kernel alrevez
        int nn = KERNEL_LENGTH - 1 - n;
        for (int m = 0; m < KERNEL_LENGTH; ++m){
          // Indice de la columna del kernel alrevez
          int mm = KERNEL_LENGTH - 1 - m;
          int ii = i + (n - mitad);
          int jj = j + (m - mitad);
          contador+=kernel[nn][mm];
          acumulador += image[ii][jj] * kernel[nn][mm];
        }
      }
      result[i][j] = acumulador/contador;
    }
  }
}

int main(int argc, char** argv) {
  double tStart;
  double serialTime, parallelTime;
  //int kernel[KERNEL_LENGTH][KERNELs_LENGTH];
  //int image[IMAGE_LENGTH][IMAGE_LENGTH];
  //int result[IMAGE_LENGTH][IMAGE_LENGTH];
  int realKernelLenth=0;
  if(KERNEL_LENGTH%2!=0) realKernelLenth = KERNEL_LENGTH/2;
  else{
    cout << "Matrix Convolution siempre debe ser 2*N+1"<<endl;
  }
  int** kernel = create_Matrix(KERNEL_LENGTH, KERNEL_LENGTH);
  int** image = create_Matrix(IMAGE_LENGTH+realKernelLenth, IMAGE_LENGTH+realKernelLenth);
  int** result = create_Matrix(IMAGE_LENGTH, IMAGE_LENGTH);
  int** resultOMP = create_Matrix(IMAGE_LENGTH, IMAGE_LENGTH);

  generate_Kernel_Matrix(kernel, KERNEL_LENGTH,KERNEL_LENGTH);
  generate_Image_Matrix(image, IMAGE_LENGTH+realKernelLenth, IMAGE_LENGTH+realKernelLenth, realKernelLenth);
  init_Matrix(result, IMAGE_LENGTH, IMAGE_LENGTH);
  init_Matrix(resultOMP, IMAGE_LENGTH, IMAGE_LENGTH);

  cout << "Image: "<<endl;
  print_Matrix(image, IMAGE_LENGTH+realKernelLenth, IMAGE_LENGTH+realKernelLenth);
  cout << "Image: "<<endl;
  print_Part_Matrix(image, realKernelLenth, realKernelLenth, IMAGE_LENGTH, IMAGE_LENGTH);
  cout << endl << endl;
  
  cout << "Kernel: "<<endl;
  print_Matrix(kernel, KERNEL_LENGTH, KERNEL_LENGTH);
  cout << endl << endl;

  tStart = omp_get_wtime();
  convolucion(kernel, image, result, realKernelLenth);
  serialTime = (double)(omp_get_wtime() - tStart);

  cout << "Result: "<<endl;
  print_Matrix(result, IMAGE_LENGTH, IMAGE_LENGTH);

  cout << "El tiempo en realizar la convolución en tiempo secuencial es: " << serialTime << endl;
  
  int ns[] = {1, 2, 4, 8};
  
  fs << "ImgLenght KerLenght numthd serialTime parallelTime eff speedUp"<< endl;
  
  for (auto num_threads: ns){
    init_Matrix(resultOMP, IMAGE_LENGTH, IMAGE_LENGTH);
    tStart = omp_get_wtime();
    convolucionOMP(num_threads, kernel, image, resultOMP, num_threads);
    parallelTime = (double)(omp_get_wtime() - tStart);
    
    //print_Matrix(resultOMP, IMAGE_LENGTH, IMAGE_LENGTH);

    cout << "Son iguales: "<< compare_Matrix(result, resultOMP,IMAGE_LENGTH, IMAGE_LENGTH) << endl;

    cout << "El tiempo en realizar la convolución en tiempo Paralelo con " << num_threads <<" es: " << parallelTime << endl;    
    cout << "Speed UP: "<< serialTime/(parallelTime) << endl;
    cout << "Eficiencia: "<< serialTime/(parallelTime*num_threads) << endl;
    
    fs << IMAGE_LENGTH << " " << KERNEL_LENGTH << " " << num_threads <<" " << serialTime <<" " << parallelTime << " " << serialTime/(parallelTime*num_threads)<<" "<< serialTime/(parallelTime) << endl;
  }
  
  return 0;
}
