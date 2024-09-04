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
double calculate_sse(SparseMatrix *R, double **P, double **Q, int num_users,
                     int num_items, int num_factors);
void train_model(SparseMatrix *R, double **P, double **Q, int num_users,
                 int num_items, int num_ratings, int num_factors, int epochs,
                 double learning_rate, double lambda, double *weights,
                 double mean, double stddev);
double predict_rating(double **P, double **Q, int user_id, int item_id,
                      int num_factors);
void recommend_items(double **P, double **Q, int user_id, int num_items,
                     int num_factors, int k, double mean, double stddev);
int compare_items(const void *a, const void *b); // Declare compare_items
double calculate_mean(Rating *ratings, int num_ratings);
double calculate_stddev(Rating *ratings, int num_ratings, double mean);
void normalize_ratings(Rating *ratings, int num_ratings, double mean,
                       double stddev);
double denormalize_prediction(double normalized_prediction, double mean,
                              double stddev);
void calculate_rating_frequencies(Rating *ratings, int num_ratings,
                                  double *freq);
void assign_weights(double *weights, double *freq);
double calculate_weighted_sse(SparseMatrix *R, double **P, double **Q,
                              int num_users, int num_items, int num_factors,
                              double *weights, double mean, double stddev);
void calculate_rating_frequencies_from_raw(const char *filename, double *freq);

#endif // MODEL_H