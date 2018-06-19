#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

using namespace std;

template<class RandomAccessIterator>
long quickPartition(RandomAccessIterator first, long low, long high){
  auto i = low, j = high + 1;
  while( true ){
    while(first[++i] < first[low])
      if( i == high )
        break;
    while(first[low] < first[--j])
      if(j == low)
        break;
    if(i >= j)
      break;
    swap(first[i], first[j]);
  }
  swap(first[low], first[j] );
  return j;
}

template<class RandomAccessIterator>
long quickPartition_Aux(RandomAccessIterator first, long low, long high){
  double x = first[high];
  auto i = (low - 1);

  for (auto j = low; j <= high-1; j++){
    if (first[j] <= x){
      i++;
      swap(first[i], first[j]);
    }
  }
  swap(first[i + 1], first[high]);
  return (i + 1);
}

template<class RandomAccessIterator>
void quickSort(RandomAccessIterator first, RandomAccessIterator last, long low, long high){
  if( low < high){
    auto j = quickPartition_Aux(first, low, high);
    quickSort(first, last, low, j-1);
    quickSort(first, last, j+1, high);
  }
}

template<class RandomAccessIterator>
void quickSortParallel(RandomAccessIterator first, RandomAccessIterator last, long low, long high){
  if( low < high){
    auto j = quickPartition_Aux(first, low, high);
    if(high-low <1000){
      quickSortParallel(first, last, low, j-1);
      quickSortParallel(first, last, j+1, high);
    } else {
      #pragma omp task
      {
        quickSortParallel(first, last, low, j-1);
      }
      #pragma omp task
      {
        quickSortParallel(first, last, j+1, high);
      }
    }
  }
}

template<class RandomAccessIterator>
void quick_sort_parallel(RandomAccessIterator first, RandomAccessIterator last, int numThreads){
  #pragma omp parallel num_threads(numThreads)
  {	
    #pragma omp single nowait
    {
      quickSortParallel(first, last, 0, last - first - 1);
    }
  }
}

template<class RandomAccessIterator>
void quick_sort(RandomAccessIterator first, RandomAccessIterator last){
  quickSort(first, last, 0, last - first - 1);
}

//return 1 if a is sorted (in non-decreasing order);
//return 0 otherwise
//assume array is allocated and populated with data prior to this call
template<class RandomAccessIterator>
bool issorted(RandomAccessIterator first, RandomAccessIterator last){
  bool isOrdered = true;
  RandomAccessIterator Index;
  for(Index = first; Index != last-1; Index++)
    if(*Index > *(Index+1)){
      isOrdered = false;
      break;
    }
  return isOrdered;
}

vector<double> generate_data(int size){
  vector<double> a;
  for(int i=0; i<size;i++){
    int random_number = (rand()%size)+1;
    a.push_back(random_number);
  }
  return a;
}

int getNumberData(){
  ifstream myfile("data.txt");
  string line;
  int i = 0;
  if (myfile.is_open()){
    while (getline(myfile,line)){
      i++;
    }
    myfile.close();
  }
  return i;
}

vector<double> readFile(){
  string line;
  vector<double> arr;
  double var;
  ifstream myfile("data.txt");
  int i = 0;
  if (myfile.is_open()){
    //while ( getline (myfile,line) ){
    while(myfile >> var){
      arr.push_back(var);
      i++;
    }
    myfile.close();
  }
  return arr;
}

void printArray(vector<double> a){
  for(int i=0;i<a.size();i++)
    cout << a.at(i) << " ";
  cout << endl;
}

vector<double> inverseVector(vector<double> array){
  int size = array.size()-1;
  for(int i=0; i<size/2; i++)
    swap(array.at(i), array.at(size-i) );
  return array;
}

int main(int argc, char** argv) {
  srand((unsigned)time(0));
  ofstream fs("datos_sort.txt");
  double tStart;
  double insertionTime, serialTime, parallelTime, quickTime, mergeTime;
  int size = getNumberData();
  cout <<"Numero de datos: "<<size<<endl;
  
  vector<double> arr_quick_sort, arr = readFile();
  
  arr_quick_sort = arr;
  
  if (!issorted(arr_quick_sort.begin(), arr_quick_sort.end())) {
    cout << "Quick Sort No esta ordenado"<<endl;
  } else {
    cout << "Quick Sort Ordenado"<<endl;
  }
  
  fs << "#numdatos serialTime parallelTime speedup efficiencia #Hilos" << endl;
    
  tStart = omp_get_wtime();
      quick_sort(arr_quick_sort.begin(), arr_quick_sort.end());
  serialTime = (double)(omp_get_wtime() - tStart);
  
  cout << "Tiempo serial que demoro el Quick Sort con " << size <<" elementos fue: " << serialTime << endl;
  
  //printArray(arr_quick_sort);
  if (!issorted(arr_quick_sort.begin(), arr_quick_sort.end())) {
    cout << "Quick Sort No esta ordenado"<<endl;
  } else {
    cout << "Quick Sort Ordenado"<<endl;
  }

  // Paralelo
  int ns[] = {1, 2, 4, 8, 16, 32};
  for (auto num_threads: ns){
    arr_quick_sort = arr;
  
    int num_thds = num_threads; //omp_get_max_threads()
    
    tStart = omp_get_wtime();
    quick_sort_parallel(arr_quick_sort.begin(), arr_quick_sort.end(), num_thds);
    parallelTime = (double)(omp_get_wtime() - tStart);
    
    cout << "Tiempo paralelo con "<<num_thds <<" hilos que demoro el Quick Sort con " << size <<" elementos fue: " << parallelTime << endl;
    cout << "Speed UP: "<< serialTime/(parallelTime) << endl;
    cout << "Eficiencia: "<< serialTime/(parallelTime*num_threads) << endl;
    
    fs << size<<" "<< serialTime << " " << parallelTime << " " << serialTime/parallelTime << " " << serialTime/parallelTime/num_thds<< " " << num_thds <<endl;
    
    //printArray(arr_quick_sort);
    if (!issorted(arr_quick_sort.begin(), arr_quick_sort.end())) {
      cout << "Quick Sort No esta ordenado"<<endl;
    } else {
      cout << "Quick Sort Ordenado"<<endl;
    }
  }
  return 0;
}
