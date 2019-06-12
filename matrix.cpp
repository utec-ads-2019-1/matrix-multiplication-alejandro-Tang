#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <iostream>

using namespace std;

struct requiredData {
  int numThreads;
  int threadNumber;
  long ** matrixA;
  long ** matrixB;
  long ** matrixC;
  int rows;
  int colsA;
  int colsB;
};

struct requiredData * newDataPackage(int nT, int tN, long ** mA, long ** mB, long ** mC,  int r, int cA, int cB){
  auto data = new requiredData;
  data->numThreads = nT;
  data->threadNumber = tN;
  data->matrixA = mA;
  data->matrixB = mB;
  data->matrixC = mC;
  data->rows = r;
  data->colsA = cA;
  data->colsB = cB;
  return data;
}

void * multiply(void * x){
  struct requiredData * data;
  data = (struct requiredData*) x;
  int NUM_THREADS = data->numThreads;

  float floatLow = data->threadNumber * (data->rows /(float) NUM_THREADS);
  int intLow = (int) floatLow;
  float floatUp = (data->threadNumber + 1) * (data->rows/(float) NUM_THREADS);
  int intUp = (int) floatUp;
  if(floatLow - intLow >= 0.5){
    intLow = intLow + 1;
  }
  if(floatUp - intUp >= 0.5){
    intUp += 1;
  }

  long ** matrixA = data->matrixA;
  long ** matrixB = data->matrixB;
  long ** matrixC = data->matrixC;

  for(int i = intLow; i < intUp; i++){
    for(int j = 0; j < data->colsB; j++){
      int count = 0;
      for(int k = 0; k < data->colsA; k++){
        count += *(matrixA[i] + k) * *(matrixB[k] + j);
      }
      *(matrixC[i] +j) = count;
      }
  }

  printf("Fin de thread %d: fila %d a %d\n",data->threadNumber, intLow, intUp);
}

long **  allocateMatrix(int rows, int cols){
  long ** matrix = (long **) malloc(rows * sizeof(long *)); //allocate array of pointers to integers (each integer pointer will point to a row)
  matrix[0] = (long *) malloc(rows * cols * sizeof(long)); //allocate memory for all the elements
  for (int i = 1; i < rows; i++){
    matrix[i] = matrix[0] + (i*cols); // the allocated memory of the second malloc is divided among the pointers of the first malloc
  }
  return matrix;
}

//Se utilizaron dos matrices llenas de 1s porque así se podía comprobar más
//fácilmente que estuviesen dando la respuesta correcta, pero se implementó
//también una que la llena con números aleatorios que también funciona

void fillWithOnes(long ** matrix, int rows, int cols){
  for(int row = 0; row < rows; row++){
    for(int col = 0; col < cols; col++){
      *(matrix[row] + col) = 1; 
    }
  }
}

void fillWithRandom(long ** matrix, int rows, int cols, int bound){
  for(int row = 0; row < rows; row++){
    for(int col = 0; col < cols; col++){
      *(matrix[row] + col) = rand() % bound; 
    }
  }
}

void printMatrix(long ** matrix, int rows, int cols){
  for(int row = 0; row < rows; row++){
    for(int col = 0; col < cols; col++){
      printf("%ld ", *(matrix[row]+col));
    }
    printf("\n");
  }
}


int main(){
  int rowsA, colsA, rowsB, colsB;	

  cout << "Ingrese el número de filas para la matriz A: ";
  cin >> rowsA;  

  cout << "Ingrese el número de columnas para la matriz A: ";
  cin >> colsA;

  cout << "Ingrese el número de filas para la matriz B: ";
  cin >> rowsB;

  cout << "Ingrese el número de columnas para la matriz B: ";
  cin >> colsB;

  if(colsA == rowsB){
    cout << "Estas matrices sí se pueden multiplicar (" << colsA <<")\n";

    int NUM_THREADS;

    cout << "Ingrese el número de hilos: ";
    cin >> NUM_THREADS;
    
    struct timeval start;
    struct timeval end;
    long timeSecs;
    double timeMicroSecs;
    double finalTime;

    pthread_t threadArr[NUM_THREADS];
    requiredData * passedData[NUM_THREADS];
    long ** matrixA = allocateMatrix(rowsA, colsA);
    long ** matrixB = allocateMatrix(rowsB, colsB);
    long ** matrixC = allocateMatrix(rowsA, colsB);
    fillWithOnes(matrixA, rowsA, colsA);
    fillWithOnes(matrixB, rowsB, colsB);

    gettimeofday(&start, 0);
    
    for(long it = 0; it < NUM_THREADS; it++){
      passedData[it] = newDataPackage(NUM_THREADS, it, matrixA, matrixB, matrixC, rowsA, colsA, colsB);
      pthread_create(&threadArr[it], NULL, multiply, (void*) passedData[it]);
    }

    for(long cit = 0; cit < NUM_THREADS; cit++){
      pthread_join(threadArr[cit], NULL);
    }

    gettimeofday(&end, 0);
    timeSecs = (end.tv_sec - start.tv_sec);    
    timeMicroSecs = (end.tv_usec - start.tv_usec)*1e-6;

    finalTime = (double) timeSecs + timeMicroSecs;

    /*printf("Matrix C\n");
    printMatrix(matrixC, rowsA, colsB);*/
    cout << "Número de hilos: " << NUM_THREADS << ". Tiempo de ejecución: " << finalTime << " secs\n";

  }else{
    printf("Estas matrices no se pueden multiplicar (%d y %d)\n", colsA, rowsB); 
  }
}
