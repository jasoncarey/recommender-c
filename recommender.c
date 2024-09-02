#include <math.h>
#include <stdio.h>
#include <stdlib.h>

double data_train[6][6] = {
    {5, 3, 0, 1, 4, 0},
    {4, 0, 0, 1, 0, 2},
    {1, 1, 0, 0, 0, 5},
    {1, 0, 0, 4, 4, 0},
    {0, 1, 5, 4, 0, 0},
    {0, 0, 0, 0, 3, 4}
};

// double data_test[6][6] = {
//     {0, 0, 0, 0, 0, 0}, // Predict missing ratings for User 1
//     {0, 0, 0, 0, 0, 0}, // Predict missing ratings for User 2
//     {0, 0, 0, 0, 0, 0}, // Predict missing ratings for User 3
//     {0, 0, 0, 0, 0, 0}, // Predict missing ratings for User 4
//     {0, 0, 0, 0, 0, 0}, // Predict missing ratings for User 5
//     {0, 0, 0, 0, 0, 0}  // Predict missing ratings for User 6
// };

double data_test[6][6] = {
    {0, 0, 0, 0, 0, 3}, // Maybe we know the rating for User 1, Item 6
    {0, 0, 4, 0, 3, 0}, // Maybe we know the ratings for User 2, Items 3 and 5
    {0, 0, 2, 0, 0, 0}, // Maybe we know the rating for User 3, Item 3
    {0, 5, 0, 0, 0, 0}, // Maybe we know the rating for User 4, Item 2
    {4, 0, 0, 0, 3, 0}, // Maybe we know the ratings for User 5, Items 1 and 5
    {0, 0, 0, 2, 0, 0}  // Maybe we know the rating for User 6, Item 4
};

typedef struct {
  int item_id;
  double rating;
} ItemRating;

void initialize_matrix_pq(double **matrix, int rows, int cols);
void initialize_matrix(double **R, double *data, int num_users, int num_items);
double calculate_sse(double **R, double **P, double **Q, int num_users,
                     int num_items, int num_factors);
void train_model(double **R, double **P, double **Q, int num_users,
                 int num_items, int num_factors, int epochs,
                 double learning_rate, double lambda);
double predict_rating(double **P, double **Q, int user_id, int item_id,
                      int num_factors);
double **create_matrix(int rows, int cols);
void free_matrix(double **matrix, int rows);
void recommend_items(double** P, double** Q, int user_id, int num_items, int num_factors, int k);

int main() {
  int num_users = 6;
  int num_items = 6;

  // Hyperparameters
  int num_factors = 5;
  int epochs = 2000;
  double learning_rate = 0.001;
  double lambda = 0.001;  // regularization parameter

  double **P = create_matrix(num_users, num_factors);
  double **Q = create_matrix(num_items, num_factors);
  double **R_train = create_matrix(num_users, num_items);
  double **R_test = create_matrix(num_users, num_items);

  initialize_matrix_pq(P, num_users, num_factors);
  initialize_matrix_pq(Q, num_items, num_factors);
  initialize_matrix(R_train, (double *)data_train, num_users, num_items);
  initialize_matrix(R_test, (double *)data_test, num_users, num_items);

  train_model((double **)R_train, P, Q, num_users, num_items, num_factors,
              epochs, learning_rate, lambda);

  // for (int i = 0; i < num_users; i++) {
  //   for (int j = 0; j < num_items; j++) {
  //     if (R_test[i][j] > 0) {
  //       double predicted_rating = predict_rating(P, Q, i, j, num_factors);
  //       printf("\nUser %d, Item %d: Predicted Rating: %1f\n", i, j,
  //              predicted_rating);
  //     }
  //   }
  // }

  recommend_items(P, Q, 0, num_items, num_factors, 3);
  recommend_items(P, Q, 1, num_items, num_factors, 3);
  recommend_items(P, Q, 2, num_items, num_factors, 3);
  recommend_items(P, Q, 3, num_items, num_factors, 3);
  recommend_items(P, Q, 4, num_items, num_factors, 3);
  recommend_items(P, Q, 5, num_items, num_factors, 3);

  free_matrix(P, num_users);
  free_matrix(Q, num_items);
  free_matrix(R_train, num_users);
  free_matrix(R_test, num_users);

  return 0;
}

// Implementation logic

// Initializes a matrix with random small values
void initialize_matrix_pq(double **matrix, int rows, int cols) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      matrix[i][j] = ((double)rand() / RAND_MAX) * 0.1;
    }
  }
}

void initialize_matrix(double **R, double *data, int num_users, int num_items) {
    for (int i = 0; i < num_users; i++) {
        for (int j = 0; j < num_items; j++) {
            R[i][j] = data[i * num_items + j];
        }
    }
}

/**
 * MSE ~= SSE = Σ(yi - y)^2
 */
double calculate_sse(double **R, double **P, double **Q, int num_users,
                     int num_items, int num_factors) {
  double sse = 0;
  int n = 0; // number of known items
  for (int i = 0; i < num_users; i++) {
    for (int j = 0; j < num_items; j++) {
      if (R[i][j] > 0) { // Only calculate for known items
        double yi = 0;
        for (int k = 0; k < num_factors; k++) {
          yi +=
              P[i][k] *
              Q[j][k]; // P[i][k] is the user factor, Q[i][k] is the item factor
        }
        double error =
            R[i][j] -
            yi; // R[i][j] is the actual rating, yi is the predicted rating
        sse += error * error;
      }
    }
  }
  return sse;
}

/**
 * Stochastic Gradient Descent
 *  Pik = Pik - learning_rate * (error * Qik - lambda * Pik)
 *  Qij = Qij - learning_rate * (error * Pik - lambda * Qij)
 */
void train_model(double **R, double **P, double **Q, int num_users,
                 int num_items, int num_factors, int epochs,
                 double learning_rate, double lambda) {
  for (int epoch = 0; epoch < epochs; epoch++) {
    for (int i = 0; i < num_users; i++) {
      for (int j = 0; j < num_items; j++) {
        if (R[i][j] > 0) {
          double prediction = 0.0;
          for (int k = 0; k < num_factors; k++) {
            prediction += P[i][k] * Q[j][k];
          }
          double error = R[i][j] - prediction;

          for (int k = 0; k < num_factors; k++) {
            double p_ik = P[i][k];
            double q_jk = Q[j][k];

            P[i][k] += learning_rate * (error * q_jk - lambda * p_ik);
            Q[j][k] += learning_rate * (error * p_ik - lambda * q_jk);
          }
        }
      }
    }

    if (epoch % 100 == 0) {
      double sse = calculate_sse(R, P, Q, num_users, num_items, num_factors);
      printf("Epoch %d: SSE: %1f\n", epoch, sse);
    }
  }
}

/**
 * Predicts the rating for a given user and item
 */
double predict_rating(double **P, double **Q, int user_id, int item_id,
                      int num_factors) {
  double prediction = 0.0;
  for (int k = 0; k < num_factors; k++) {
    prediction += P[user_id][k] * Q[item_id][k];
  }

  // Clip prediction to the range [1, 5]
  if (prediction > 5.0) {
    prediction = 5.0;
  }
  if (prediction < 1.0) {
    prediction = 1.0;
  }
  return prediction;
}

/**
 * Compares two items based on their ratings, used for quicksort
 */
int compare_items(const void *a, const void *b) {
    double diff = ((ItemRating *)b)->rating - ((ItemRating *)a)->rating;
    return (diff > 0) - (diff < 0);  // Returns -1 if b > a, 1 if a > b, 0 if equal
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

double **create_matrix(int rows, int cols) {
  double **matrix = (double **)malloc(rows * sizeof(double *));
  if (matrix == NULL) {
    printf("Failed to allocate memory for matrix\n");
    exit(1);
  }

  for (int i = 0; i < rows; i++) {
    matrix[i] = (double *)malloc(cols * sizeof(double));
    if (matrix[i] == NULL) {
      printf("Failed to allocate memory for row %d\n", i);
      exit(1);
    }
  }
  return matrix;
}

void free_matrix(double **matrix, int rows) {
  for (int i = 0; i < rows; i++) {
    free(matrix[i]);
  }
  free(matrix);
}