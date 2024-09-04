#include "model.h"
#include "util.h"
#include "sparse_matrix.h"
#include <stdlib.h>
#include <stdio.h>

void initialize_matrix_pq(double **matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = ((double)rand() / RAND_MAX) * 0.01;
        }
    }
}

double calculate_sse(SparseMatrix* R, double **P, double **Q, int num_users, int num_items, int num_factors) {
    double sse = 0.0;
    int count = 0;

    for (int i = 0; i < num_users; i++) {
        RatingNode* node = R->users[i].ratings;
        while (node) {
            double prediction = 0.0;
            for (int k = 0; k < num_factors; k++) {
                prediction += P[i][k] * Q[node->song_id][k];
            }

            double error = node->rating - prediction;
            sse += error * error;

            // Debugging output for first few items
            if (count < 10) {
                printf("User %d, Item %d: Actual = %f, Predicted = %f, Error = %f\n", i, node->song_id, node->rating, prediction, error);
            }
            count++;
            node = node->next;
        }
    }
    return sse;
}

void train_model(SparseMatrix* R, double **P, double **Q, int num_users,
                 int num_items, int num_factors, int epochs,
                 double learning_rate, double lambda) {
    for (int epoch = 0; epoch < epochs; epoch++) {
        for (int i = 0; i < num_users; i++) {
            RatingNode* node = R->users[i].ratings;
            while (node) {
                double prediction = 0.0;
                for (int k = 0; k < num_factors; k++) {
                    prediction += P[i][k] * Q[node->song_id][k];
                }
                double error = node->rating - prediction;
                for (int k = 0; k < num_factors; k++) {
                    double p_ik = P[i][k];
                    double q_jk = Q[node->song_id][k];

                    P[i][k] += learning_rate * (error * q_jk - lambda * p_ik);
                    Q[node->song_id][k] += learning_rate * (error * p_ik - lambda * q_jk);
                }
                node = node->next;
            }
        }

        if (epoch % 1 == 0) {
            double sse = calculate_sse(R, P, Q, num_users, num_items, num_factors);
            printf("Epoch %d: SSE: %1f\n", epoch, sse);
        }
    }
}

double predict_rating(double **P, double **Q, int user_id, int item_id,
                      int num_factors) {
    double prediction = 0.0;
    for (int k = 0; k < num_factors; k++) {
        prediction += P[user_id][k] * Q[item_id][k];
    }
    // if (prediction > 5.0) prediction = 5.0;
    // if (prediction < 1.0) prediction = 1.0;
    return prediction;
}

void recommend_items(double** P, double** Q, int user_id, int num_items, int num_factors, int k) {
    ItemRating* items = malloc(num_items * sizeof(ItemRating));
    for (int item_id = 0; item_id < num_items; item_id++) {
        items[item_id].item_id = item_id;
        items[item_id].rating = predict_rating(P, Q, user_id, item_id, num_factors);
    }
    qsort(items, num_items, sizeof(ItemRating), compare_items);
    printf("\nTop %d items for user %d:\n", k, user_id + 1);
    for (int i = 0; i < k; i++) {
        printf("Item %d with predicted rating %1f\n", items[i].item_id + 1, items[i].rating);
    }
    free(items);
}

int compare_items(const void *a, const void *b) {
    double diff = ((ItemRating *)b)->rating - ((ItemRating *)a)->rating;
    return (diff > 0) - (diff < 0);  // Returns -1 if b > a, 1 if a > b, 0 if equal
}