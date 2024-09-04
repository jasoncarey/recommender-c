#ifndef MODEL_H
#define MODEL_H

#include "sparse_matrix.h"

// Define the ItemRating structure here
typedef struct {
    int item_id;
    double rating;
} ItemRating;

// Function declarations related to the model
void initialize_matrix_pq(double **matrix, int rows, int cols);
double calculate_sse(SparseMatrix* R, double **P, double **Q, int num_users,
                     int num_items, int num_factors);
void train_model(SparseMatrix* R, double **P, double **Q, int num_users,
                 int num_items, int num_factors, int epochs,
                 double learning_rate, double lambda);
double predict_rating(double **P, double **Q, int user_id, int item_id,
                      int num_factors);
void recommend_items(double** P, double** Q, int user_id, int num_items, int num_factors, int k);
int compare_items(const void *a, const void *b);  // Declare compare_items

#endif // MODEL_H