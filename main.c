#include "sparse_matrix.h"
#include "model.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>

int main() {
    int num_ratings;
    int max_user_id, max_song_id;

    Rating* ratings = load_csv_data("data/songsDataset.csv", &num_ratings, &max_user_id, &max_song_id);

    double mean = calculate_mean(ratings, num_ratings);
    double stddev = calculate_stddev(ratings, num_ratings, mean);
    normalize_ratings(ratings, num_ratings, mean, stddev);

    int num_users = max_user_id + 1;
    SparseMatrix* R_train = create_sparse_matrix(num_users);
    fill_sparse_matrix(R_train, ratings, num_ratings, num_users, max_song_id + 1);

    int total_ratings = count_total_ratings(R_train);

    double freq[6] = {0};
    calculate_rating_frequencies_from_raw("data/songsDataset.csv", freq);
    double weights[6] = {0};
    assign_weights(weights, freq);

    // Hyperparameters
    int num_factors = 10;
    int epochs = 501;
    double learning_rate = 0.008;
    double lambda = 0.3;

    printf("Running with %d factors, %d epochs, %f learning rate, %f lambda\n", num_factors, epochs, learning_rate, lambda);

    double **P = create_matrix(num_users, num_factors);
    double **Q = create_matrix(max_song_id + 1, num_factors);
    initialize_matrix_pq(P, num_users, num_factors);
    initialize_matrix_pq(Q, max_song_id + 1, num_factors);

    train_model(R_train, P, Q, num_users, max_song_id + 1, total_ratings, num_factors,
                epochs, learning_rate, lambda, weights, mean, stddev);

    recommend_items(P, Q, 0, max_song_id + 1, num_factors, 5, mean, stddev);
    recommend_items(P, Q, 1, max_song_id + 1, num_factors, 5, mean, stddev);
    recommend_items(P, Q, 10, max_song_id + 1, num_factors, 5, mean, stddev);
    recommend_items(P, Q, 499, max_song_id + 1, num_factors, 5, mean, stddev);
    recommend_items(P, Q, 1000, max_song_id + 1, num_factors, 5, mean, stddev);
    recommend_items(P, Q, 79999, max_song_id + 1, num_factors, 5, mean, stddev);

    free_matrix(P, num_users);
    free_matrix(Q, max_song_id + 1);
    free_sparse_matrix(R_train);
    free(ratings);

    return 0;
}