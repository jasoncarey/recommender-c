#include "sparse_matrix.h"
#include "model.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>

int main() {
    int num_ratings;
    int max_user_id, max_song_id;

    Rating* ratings = load_csv_data("data/songsDataset.csv", &num_ratings, &max_user_id, &max_song_id);

    int num_users = max_user_id + 1;
    SparseMatrix* R_train = create_sparse_matrix(num_users);

    fill_sparse_matrix(R_train, ratings, num_ratings, num_users, max_song_id + 1);
    int total_ratings = count_total_ratings(R_train);
    printf("Total number of ratings in sparse matrix: %d\n", total_ratings);
    int total_users = count_users_with_ratings(R_train);
    printf("Total number of users with ratings: %d\n", total_users);

    //load_ratings_into_sparse_matrix(R_train, ratings, num_ratings);
    int num_factors = 10;
    int epochs = 10;
    double learning_rate = 0.005;
    double lambda = 0.1;

    double **P = create_matrix(num_users, num_factors);
    double **Q = create_matrix(max_song_id + 1, num_factors);
    initialize_matrix_pq(P, num_users, num_factors);
    initialize_matrix_pq(Q, max_song_id + 1, num_factors);

    train_model(R_train, P, Q, num_users, max_song_id + 1, num_factors,
                epochs, learning_rate, lambda);

    recommend_items(P, Q, 0, max_song_id + 1, num_factors, 5);
    recommend_items(P, Q, 1, max_song_id + 1, num_factors, 5);

    free_matrix(P, num_users);
    free_matrix(Q, max_song_id + 1);
    free_sparse_matrix(R_train);
    free(ratings);

    return 0;
}