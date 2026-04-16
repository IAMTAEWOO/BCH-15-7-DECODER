#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 15
#define K 7
#define NUM_TEST 10000

// BCH(15,7) generator polynomial
// g(x) = x^8 + x^7 + x^6 + x^4 + 1
int g[9] = { 1,1,1,0,1,0,0,0,1 };


// ---------------- ENCODER ----------------
void bch_encode(int* data, int* codeword) {
    int temp[N] = { 0 };

    // data copy
    for (int i = 0; i < K; i++)
        temp[i] = data[i];

    // polynomial division
    for (int i = 0; i < K; i++) {
        if (temp[i] == 1) {
            for (int j = 0; j < 9; j++) {
                temp[i + j] ^= g[j];
            }
        }
    }

    // systematic codeword
    for (int i = 0; i < K; i++)
        codeword[i] = data[i];

    for (int i = 0; i < 8; i++)
        codeword[K + i] = temp[K + i];
}

#if 0
// ---------------- MAIN ----------------
int main() {
    FILE* fp = fopen("input.txt", "w");

    if (!fp) {
        printf("File open error\n");
        return 0;
    }

    srand((unsigned int)time(NULL));

    for (int t = 0; t < NUM_TEST; t++) {

        int data[K];
        int codeword[N];

        // random data 생성
        for (int i = 0; i < K; i++) {
            data[i] = rand() % 2;
        }

        // encode
        bch_encode(data, codeword);

        // 파일에 저장
        for (int i = 0; i < N; i++) {
            fprintf(fp, "%d ", codeword[i]);
        }
        fprintf(fp, "\n");
    }

    fclose(fp);

    printf("input.txt generated with %d codewords\n", NUM_TEST);

    return 0;
}
#endif
