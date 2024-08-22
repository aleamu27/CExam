#ifndef konte_h23_oppgave2
#define konte_h23_oppgave2

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

bool isAbundantNumber(int n);
bool isDeficientNumber(int n);
bool isFibonacci(int n);
bool isSquareNumber(int n);
bool isOdd(int n);
bool isPerfectNumber(int n);
bool isPrime(int n);

typedef struct {
    int  number;
    bool Fibonacci;
    bool PrimeNumber;
    bool SquareNumber;
    bool PerfectNumber;
    bool AbundantNumber;
    bool DeficientNumber;
    bool OddNumber;
} numbStruct;

#endif
