#include "main.h"

void processNumber(int num) {
    numbStruct data;
    data.number = num;
    data.Fibonacci = isFibonacci(num);
    data.PrimeNumber = isPrime(num);
    data.SquareNumber = isSquareNumber(num);
    data.PerfectNumber = isPerfectNumber(num);
    data.AbundantNumber = isAbundantNumber(num);
    data.DeficientNumber = isDeficientNumber(num);
    data.OddNumber = isOdd(num);
    
    


    printf("Processed number? %d\n", num);
    printf("Is Fibonacci? %s\n", data.Fibonacci ? "YES" : "NO");
    printf("Is Prime? %s\n", data.PrimeNumber ? "YES" : "NO");
    printf("Is Square? %s\n", data.SquareNumber ? "YES" : "NO");
    printf("Perfekt? %s\n", data.PerfectNumber ? "YES" : "NO");
    printf("Abundant? %s\n", data.AbundantNumber ? "YES" : "NO");
    printf("Deficient? %s\n", data.DeficientNumber ? "YES" : "NO");
    printf("Odd? %s\n\n", data.OddNumber ? "YES" : "NO");
    
     FILE *file = fopen("output.txt", "ab");
    if (file != NULL) {
        fwrite(&data, sizeof(numbStruct), 1, file);
        fclose(file);
    } else {
        printf("Error opening file for writing.\n");
    }
}


int main() {
    char input[50];
    int nummer;
    char continue_choice;

    printf("Enter a number and press enter. Enter '0' to terminate the program.\n");

    while (1) {
        printf("Enter a number or 0 to quit): ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0; 

        nummer = atoi(input);

        if (nummer == 0) {
            break;
        }

        if (nummer > 0) {
            processNumber(nummer);

            printf("Do you want to add another number? (Y/N): ");
            do {
                continue_choice = getchar();
                while (getchar() != '\n'); // Clear input buffer
            } while (continue_choice != 'Y' && continue_choice != 'y' &&
                     continue_choice != 'N' && continue_choice != 'n');

            if (continue_choice == 'N' || continue_choice == 'n') {
                break;
            }
        } else {
            printf("Please enter a valid positive integer or '0' to exit.\n");
        }
    }

    printf("\nProgram terminated.\n");
    return 0;
}
