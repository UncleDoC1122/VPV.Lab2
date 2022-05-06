// *******************************************************************
// ������������ ���������� ���������� ������ ���������� ���������
// ������� sin(x)/x
// integralThreadLock - ������ ���������, ����� ����������� 
//     � ��������� ����� �������������� � ���������� ����� � ����������� ������ ������ 
// integralThreadStLock - ������ ��������� ��� ��������������, ����� ������ 
//     � ������ ����� ��� ������������ ���� ��������� ����� � ���������� � �����������
// integralThreadStNoLock - ������ ��������� ��� ��������������, ����� ������ 
//     � ������ ����� ��� ������������ ���� ��������� ����� � ���������� � �����������
// integralOpenMP - ������ ��������� � ����������� ����� ���������� openMP ���������� 
//     #pragma omp parallel for private(x) reduction(+:sum)
// nvnulstu@gmail.com - ������ 2021
// *******************************************************************
#pragma once
#include "main.h"
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <thread>
#include <mutex>
#include <omp.h>


using namespace std;

// ������������ �������� ������� sin(x)/x �� ��������� 0..1 ����� ������� ���
// f(x) = sin(x) / x = 1 - x^2/3! + x^4/5! - x^6/7! + ... + (-1)^n * x^(2n+1) / (2n+1)! ...
// �������������(x) = F(x) = x - x^3 / (3*3!) + x^5 / (5*5!) -x^7/(7*7!) + ... + (-1)^n * x^(2n+1) / ((2n+1)*(2n+1)!) + ... 
// ��������(0, 1) = F(1) - F(0) = 1 - 1/(3*3!) + 1 / (5*5!) -1/(7*7!) + ... + (-1)^n / ((2n+1)*(2n+1)!)
double calcEtalon () {
	double	sum = 1.0, // ����� ������ ���������� ����
			fact = 1.0; // ��������� ���� double, ����� �������� ������������, ��������� uint64 �������� ������ 20!
	int sign = 1;
	for (int n = 1; n <= 20; n++) {  
		fact *= 2 * n * (2*n + 1);
		sign *= -1;
		sum += sign / ((2 * n + 1) * fact);
	}
	return sum;
}

// ���������� ��������� ����� ���������� thread 
double globSum; // ���������� ����� ����� ��������������� �� ���� �������
double arrSum[MAX_THREAD]; // c���� �� ��������� ������� 

mutex mu; // ������� ��� ���������� �������� � globSum
// ������� ������������ ����� ���������������,  � ������� wRect 
// ����� ����������� � ���������� � �����������
void fuSumLock(int nRect, int thRect, double wRect) {
   double x, sum = 0.0;
   thRect += nRect;
   // ���� ������������ ����� ������� ���������������
   do {
      x = nRect * wRect + 0.5 * wRect;
      sum += sin(x) / x;
   } while (++nRect < thRect);
   mu.lock();
   globSum = globSum + sum;
   mu.unlock();
};

// ������� ������������ ����� ���������������,  � ������� wRect 
// ����� ������� �� ������ pResult
void fuSumNoLock(int nRect, int thRect, double wRect, double* pResult) {
   double x, sum = 0.0;
   thRect += nRect;
   // ���� ������������ ����� ������� ���������������
   do {
      x = nRect * wRect + 0.5 * wRect;
      sum += sin(x) / x;
   } while (++nRect < thRect);
   *pResult = sum;
};

// �������� � ���������� nTreads ������� � ���������� �� ��� allRect ���������������
// ������ ����� ������������ ���� ��������� � ���������� ����� globSum
double integralThreadLock(int nThreads, int allRect) {	
    double wRect = 1.0 / (double)allRect; // ������ �������������� ���������� ������
    // ����� ��������������� ��� ������ ������
    int thRect = (allRect - 1) / nThreads + 1; 
    // ������������� ����� ������� � ������, ����� �� ������, ��� allRect
    nThreads = (allRect - 1) / thRect + 1; 
    int restRect = allRect - (nThreads - 1) * thRect;
    if (restRect == 0) restRect = thRect; // ����� allRect ������ nThreads
    globSum = 0.0; 
    // ������� nThreads �������
    thread * threads = new thread[nThreads];
    for (int n = 0; n < nThreads; n++)
       threads[n] = thread(fuSumLock, n * thRect, n < nThreads - 1 ? thRect : restRect, wRect);
    for (int n = 0; n < nThreads; n++) 
       threads[n].join();
    return globSum * wRect; // ������� ������� ���� ������� ���������������
}

// ������������� ��� ��������� nTreads ������� � ���������� �� ��� allRect ���������������
thread threadsStatic[MAX_THREAD];

double integralThreadStNoLock(int nThreads, int allRect) {
    double wRect = 1.0 / (double)allRect; // ������ �������������� ���������� ������
// ����� ��������������� ��� ������ ������
    int thRect = (allRect - 1) / nThreads + 1;
    // ������������� ����� ������� � ������, ����� �� ������, ��� allRect
    nThreads = (allRect - 1) / thRect + 1;
    int restRect = allRect - (nThreads - 1) * thRect;
    if (restRect == 0) restRect = thRect; // ����� allRect ������ nThreads
    globSum = 0.0;
    // ������� nThreads �������
    for (int n = 0; n < nThreads; n++)
       threadsStatic[n] = thread(fuSumNoLock, n * thRect, 
          n < nThreads - 1 ? thRect : restRect, wRect, arrSum + n); 
    for (int n = 0; n < nThreads; n++) 
        threadsStatic[n].join();
    globSum = 0.0;
    for (int n = 0; n < nThreads; n++) 
       globSum += arrSum[n];
    return globSum * wRect; // ������� ������� ���� ������� ���������������
}

double integralThreadStLock(int nThreads, int allRect) {
	double wRect = 1.0 / (double)allRect; // ������ �������������� ���������� ������
// ����� ��������������� ��� ������ ������
	int thRect = (allRect - 1) / nThreads + 1;
	// ������������� ����� ������� � ������, ����� �� ������, ��� allRect
	nThreads = (allRect - 1) / thRect + 1;
	int restRect = allRect - (nThreads - 1) * thRect;
	if (restRect == 0) restRect = thRect; // ����� allRect ������ nThreads
	globSum = 0.0;
	// ������� nThreads �������
	for (int n = 0; n < nThreads; n++)
		threadsStatic[n] = thread(fuSumLock, n * thRect, n < nThreads - 1 ? thRect : restRect, wRect);
	for (int n = 0; n < nThreads; n++) 
        threadsStatic[n].join();
	return globSum * wRect; // ������� ������� ���� ������� ���������������
}

// ���������� ��������� ����� ���������� openMP 
//   numTreads �������� � ���������� ��������� 0..1 �� numSubIntervals �������������
double integralOpenMP(int numTreads, int numSubIntervals) {
	int i;
	double x;
	double sum = 0.0;
	double wRect = 1.0 / (double)numSubIntervals;
	omp_set_num_threads(numTreads);
#pragma omp parallel for private(x) reduction(+:sum)   
	for (i = 0; i < numSubIntervals; i++)
	{	x = i * wRect + wRect / 2;
		sum += sin(x)/x;
	}
	return sum * wRect;
}

