#ifndef SPARSE_MATRIX_H
#define SPARSE_MATRIX_H

#include "utils.h" // Include the utils.h file for the Rating structure

typedef struct RatingNode {
  int song_id;
  double rating;
  struct RatingNode *next;
} RatingNode;

typedef struct {
  int user_id;
  RatingNode *ratings;
} UserRatings;

typedef struct {
  UserRatings *users;
  int num_users;
} SparseMatrix;

SparseMatrix *create_sparse_matrix(int num_users);
void add_rating(SparseMatrix *matrix, int user_id, int song_id, double rating);
double get_rating(SparseMatrix *matrix, int user_id, int song_id);
void free_sparse_matrix(SparseMatrix *matrix);
void load_ratings_into_sparse_matrix(SparseMatrix *matrix, Rating *ratings,
                                     int num_ratings);
void fill_sparse_matrix(SparseMatrix *matrix, Rating *ratings, int num_ratings,
                        int num_users, int num_items);
int count_total_ratings(SparseMatrix *matrix);
int count_users_with_ratings(SparseMatrix *matrix);

#endif // SPARSE_MATRIX_H