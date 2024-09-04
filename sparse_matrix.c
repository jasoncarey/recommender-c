#include "sparse_matrix.h"
#include <stdlib.h>
#include <stdio.h>

SparseMatrix* create_sparse_matrix(int num_users) {
    SparseMatrix* matrix = (SparseMatrix*)malloc(sizeof(SparseMatrix));
    matrix->users = (UserRatings*)calloc(num_users, sizeof(UserRatings));
    matrix->num_users = num_users;
    return matrix;
}

void add_rating(SparseMatrix* matrix, int user_id, int song_id, double rating) {
    if (user_id >= matrix->num_users) {
        printf("User ID %d out of bounds\n", user_id);
        return;
    }
    UserRatings* user_ratings = &matrix->users[user_id];
    RatingNode* new_node = (RatingNode*)malloc(sizeof(RatingNode));
    new_node->song_id = song_id;
    new_node->rating = rating;
    new_node->next = user_ratings->ratings;
    user_ratings->ratings = new_node;
}

double get_rating(SparseMatrix* matrix, int user_id, int song_id) {
    if (user_id >= matrix->num_users) {
        return 0.0; // Out of bounds, assume no rating
    }
    RatingNode* node = matrix->users[user_id].ratings;
    while (node) {
        if (node->song_id == song_id) {
            return node->rating;
        }
        node = node->next;
    }
    return 0.0; // No rating found
}

void free_sparse_matrix(SparseMatrix* matrix) {
    for (int i = 0; i < matrix->num_users; i++) {
        RatingNode* node = matrix->users[i].ratings;
        while (node) {
            RatingNode* temp = node;
            node = node->next;
            free(temp);
        }
    }
    free(matrix->users);
    free(matrix);
}

void load_ratings_into_sparse_matrix(SparseMatrix* matrix, Rating* ratings, int num_ratings) {
    for (int i = 0; i < num_ratings; i++) {
        add_rating(matrix, ratings[i].user_id, ratings[i].song_id, ratings[i].rating);
    }
}

void fill_sparse_matrix(SparseMatrix* matrix, Rating* ratings, int num_ratings, int num_users, int num_items) {
    for (int i = 0; i < num_ratings; i++) {
        int user_id = ratings[i].user_id;
        int song_id = ratings[i].song_id;
        double rating = ratings[i].rating;

        // Debugging: Print the current indices
        // if (i < 10 || i % 100000 == 0) { // Print first few and then every 100,000 ratings
        //     printf("Processing rating %d: user_id=%d, song_id=%d, rating=%f\n", i, user_id, song_id, rating);
        // }

        // Ensure indices are within bounds
        if (user_id >= 0 && user_id < num_users && song_id >= 0 && song_id < num_items) {
            add_rating(matrix, user_id, song_id, rating);
        } else {
            printf("Error: user_id %d or song_id %d is out of bounds\n", user_id, song_id);
        }
    }

    // Print the matrix for the first 3 users
    printf("R matrix for the first 3 users:\n");
    for (int i = 0; i < 10; i++) { // First 3 users
        printf("User %d: ", i + 1);
        RatingNode* node = matrix->users[i].ratings;
        while (node) {
            printf("(%d, %1.2f) ", node->song_id, node->rating);
            node = node->next;
        }
        printf("\n");
    }
}

int count_total_ratings(SparseMatrix* matrix) {
    int total_ratings = 0;
    
    for (int i = 0; i < matrix->num_users; i++) {
        RatingNode* node = matrix->users[i].ratings;
        while (node) {
            total_ratings++;
            node = node->next;
        }
    }
    
    return total_ratings;
}

int count_users_with_ratings(SparseMatrix* matrix) {
    int user_count = 0;

    for (int i = 0; i < matrix->num_users; i++) {
        if (matrix->users[i].ratings != NULL) {
            user_count++;
        }
    }

    return user_count;
}