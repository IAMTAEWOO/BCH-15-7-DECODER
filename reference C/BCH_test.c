#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define N 15
#define K 7
#define T 2
#define MAX_TEST 10000
#define M_PI 3.14159265358979323846

#define DEBUG_MODE 1   // 1: 디버그 출력 / 0: SNR BER 측정

int alpha_to[16];
int index_of[16];


// ---------------- PRINT ----------------
void print_vector(const char* name, int* v, int len) {
    printf("%s: ", name);
    for (int i = 0; i < len; i++)
        printf("%d ", v[i]);
    printf("\n");
}

void print_syndrome(int* S) {
    printf("Syndrome: ");
    for (int i = 1; i <= 2 * T; i++)
        printf("S%d=%d ", i, S[i]);
    printf("\n");
}

void print_lambda(int* Lambda) {
    printf("Lambda(x): ");
    for (int i = 0; i <= T; i++)
        printf("%d ", Lambda[i]);
    printf("\n");
}

void print_errors(int* error_pos, int num_errors) {
    printf("Error positions: ");
    for (int i = 0; i < num_errors; i++)
        printf("%d ", error_pos[i]);
    printf("\n");
}


// ---------------- TXT LOAD ----------------
int load_test_vectors(const char* filename, int data[][N]) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        printf("File open error\n");
        return 0;
    }

    int count = 0;

    while (1) {
        int ret = 0;

        for (int i = 0; i < N; i++) {
            ret += fscanf(fp, "%d", &data[count][i]);
        }

        if (ret != N) break;

        count++;
        if (count >= MAX_TEST) break;
    }

    fclose(fp);
    return count;
}


// ---------------- AWGN ----------------
double gaussian_noise(double sigma) {
    double u1 = (double)rand() / RAND_MAX;
    double u2 = (double)rand() / RAND_MAX;

    return sigma * sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
}

void add_awgn_noise(int* data, double snr_db) {
    double snr = pow(10.0, snr_db / 10.0);
    double sigma = sqrt(1.0 / (2.0 * snr));

    for (int i = 0; i < N; i++) {
        double tx = data[i] ? -1.0 : 1.0;
        double rx = tx + gaussian_noise(sigma);

        data[i] = (rx < 0) ? 1 : 0;
    }
}


// ---------------- GF INIT ----------------
void init_gf() {
    int i, mask = 1;

    alpha_to[4] = 0;
    for (i = 0; i < 4; i++) {
        alpha_to[i] = mask;
        index_of[alpha_to[i]] = i;

        if ((1 << i) & 0x13)
            alpha_to[4] ^= mask;

        mask <<= 1;
    }

    index_of[alpha_to[4]] = 4;
    mask >>= 1;

    for (i = 5; i < 15; i++) {
        if (alpha_to[i - 1] >= mask)
            alpha_to[i] = alpha_to[4] ^ ((alpha_to[i - 1] ^ mask) << 1);
        else
            alpha_to[i] = alpha_to[i - 1] << 1;

        index_of[alpha_to[i]] = i;
    }

    index_of[0] = -1;
}


// ---------------- SYNDROME ----------------
void compute_syndrome(int* recv, int* S) {
    for (int i = 1; i <= 2 * T; i++) {
        S[i] = 0;

        for (int j = 0; j < N; j++) {
            if (recv[j])
                S[i] ^= alpha_to[(i * j) % 15];
        }

        S[i] = index_of[S[i]];
    }
}


// ---------------- BERLEKAMP-MASSEY ----------------
void berlekamp_massey(int* S, int* Lambda) {
    int B[5] = { 1,0,0,0,0 };
    int C[5] = { 1,0,0,0,0 };
    int T_poly[5];

    int L = 0, m = 1, b = 1;

    for (int n = 0; n < 2 * T; n++) {
        int d = 0;

        for (int i = 0; i <= L; i++) {
            if (C[i] != 0 && S[n + 1 - i] != -1)
                d ^= alpha_to[(index_of[C[i]] + S[n + 1 - i]) % 15];
        }

        if (d == 0) {
            m++;
        }
        else {
            for (int i = 0; i < 5; i++)
                T_poly[i] = C[i];

            int coef = index_of[d] - index_of[b];
            if (coef < 0) coef += 15;

            for (int i = 0; i < 5; i++) {
                if (B[i] != 0) {
                    int idx = (index_of[B[i]] + coef) % 15;
                    C[i + m] ^= alpha_to[idx];
                }
            }

            if (2 * L <= n) {
                L = n + 1 - L;

                for (int i = 0; i < 5; i++)
                    B[i] = T_poly[i];

                b = d;
                m = 1;
            }
            else {
                m++;
            }
        }
    }

    for (int i = 0; i <= T; i++)
        Lambda[i] = C[i];
}


// ---------------- CHIEN SEARCH ----------------
void chien_search(int* Lambda, int* error_pos, int* num_errors) {
    *num_errors = 0;

    for (int i = 0; i < N; i++) {
        int sum = 1;

        for (int j = 1; j <= T; j++) {
            if (Lambda[j] != 0) {
                int idx = (index_of[Lambda[j]] + j * i) % 15;
                sum ^= alpha_to[idx];
            }
        }

        if (sum == 0) {
            error_pos[*num_errors] = i;
            (*num_errors)++;
        }
    }
}


// ---------------- ERROR CORRECTION ----------------
void correct_errors(int* recv, int* error_pos, int num_errors) {
    for (int i = 0; i < num_errors; i++)
        recv[error_pos[i]] ^= 1;
}


// ---------------- DEBUG DECODER ----------------
void bch_decode_debug(int* recv) {
    int S[5] = { 0 };
    int Lambda[5] = { 0 };
    int error_pos[5];
    int num_errors;

    print_vector("Received", recv, N);

    compute_syndrome(recv, S);
    print_syndrome(S);

    berlekamp_massey(S, Lambda);
    print_lambda(Lambda);

    chien_search(Lambda, error_pos, &num_errors);
    print_errors(error_pos, num_errors);

    correct_errors(recv, error_pos, num_errors);

    print_vector("Corrected", recv, N);
    printf("--------------------------------------------------\n");
}


// ---------------- NORMAL DECODER ----------------
void bch_decode(int* recv) {
    int S[5] = { 0 };
    int Lambda[5] = { 0 };
    int error_pos[5];
    int num_errors;

    compute_syndrome(recv, S);
    berlekamp_massey(S, Lambda);
    chien_search(Lambda, error_pos, &num_errors);
    correct_errors(recv, error_pos, num_errors);
}


// ---------------- MAIN ----------------
int main() {
    int test_vectors[MAX_TEST][N];
    int num_tests;

    init_gf();
    srand((unsigned int)time(NULL));

    num_tests = load_test_vectors("input.txt", test_vectors);

    if (num_tests == 0) {
        printf("No test vectors loaded\n");
        return 0;
    }

#if DEBUG_MODE

    printf("===== DEBUG MODE =====\n\n");

    for (int t = 0; t < 5; t++) {

        int original[N];
        int noisy[N];

        for (int i = 0; i < N; i++) {
            original[i] = test_vectors[t][i];
            noisy[i] = test_vectors[t][i];
        }

        print_vector("Original", original, N);

        add_awgn_noise(noisy, 3.0);

        print_vector("Noisy", noisy, N);

        bch_decode_debug(noisy);

        int error = 0;
        for (int i = 0; i < N; i++) {
            if (original[i] != noisy[i])
                error++;
        }

        printf("Final Error Count: %d\n\n", error);
    }

#else

    printf("===== SNR BER MODE =====\n");

    for (float snr_db = 0; snr_db <= 10; snr_db += 1.0) {

        int total_error = 0;

        for (int t = 0; t < num_tests; t++) {

            int original[N];
            int noisy[N];

            for (int i = 0; i < N; i++) {
                original[i] = test_vectors[t][i];
                noisy[i] = test_vectors[t][i];
            }

            add_awgn_noise(noisy, snr_db);
            bch_decode(noisy);

            for (int i = 0; i < N; i++) {
                if (original[i] != noisy[i])
                    total_error++;
            }
        }

        float ber = (float)total_error / (num_tests * N);

        printf("SNR = %.1f dB, BER = %e\n", snr_db, ber);
    }

#endif

    return 0;
}
