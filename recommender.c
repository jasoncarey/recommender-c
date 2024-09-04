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

typedef struct {
  int user_id;
  int song_id;
  double rating;
} Rating;

/**
 * Loads a CSV file with user, song, and rating data
 * Returns a Rating array with the data
 */
Rating* load_csv_data(const char* filename, int* num_ratings, int* max_user_id, int* max_song_id) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(1);
    }

    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    int user_id, song_id;
    double rating;
    int count = 0;
    int capacity = 1000000;
    Rating* ratings = malloc(capacity * sizeof(Rating));

    if (!ratings) {
        perror("Failed to allocate memory for ratings");
        exit(1);
    }

    *max_user_id = 0;
    *max_song_id = 0;

    while ((read = getline(&line, &len, file)) != -1) {
        if (sscanf(line, "%d,%d,%lf", &user_id, &song_id, &rating) == 3) {
            if (count >= capacity) {
                capacity *= 2;
                Rating* new_ratings = realloc(ratings, capacity * sizeof(Rating));
                if (!new_ratings) {
                    free(ratings);
                    perror("Failed to reallocate memory for ratings");
                    exit(1);
                }
                ratings = new_ratings;
            }

            ratings[count].user_id = user_id;
            ratings[count].song_id = song_id;
            ratings[count].rating = rating;

            if (user_id > *max_user_id) *max_user_id = user_id;
            if (song_id > *max_song_id) *max_song_id = song_id;

            count++;
        } else {
            printf("Skipping invalid line: %s", line);
        }
    }

    fclose(file);
    if (line) {
        free(line);
    }
    *num_ratings = count;

    printf("Loaded %d ratings. Max user ID: %d, Max song ID: %d\n\n", *num_ratings, *max_user_id, *max_song_id);

    return ratings;
}

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
double ** create_sparse_matrix(int num_users, int num_songs);
void fill_sparse_matrix(double** matrix, Rating* ratings, int num_ratings, int num_users, int num_items);
int main() {
  int num_ratings;
  int max_user_id, max_song_id;

  Rating* ratings = load_csv_data("data/songsDataset.csv", &num_ratings, &max_user_id, &max_song_id);

  int num_users = max_user_id + 1;
  int num_items = max_song_id + 1;

  printf("num_users: %d, num_items: %d\n", num_users, num_items);

  // // Hyperparameters
  int num_factors = 10;
  int epochs = 1;
  double learning_rate = 0.01;
  double lambda = 0.1;

  double **P = create_matrix(num_users, num_factors);
  double **Q = create_matrix(num_items, num_factors);
  initialize_matrix_pq(P, num_users, num_factors);
  initialize_matrix_pq(Q, num_items, num_factors);

  double** R_train = create_sparse_matrix(num_users, num_items);
  // fill_sparse_matrix(R_train, ratings, num_ratings, num_users, num_items);

  // train_model((double **)R_train, P, Q, num_users, num_items, num_factors,
  //             epochs, learning_rate, lambda);


  // recommend_items(P, Q, 0, num_items, num_factors, 5);
  // recommend_items(P, Q, 1, num_items, num_factors, 5);

  // free_matrix(P, num_users);
  // free_matrix(Q, num_items);
  // free_matrix(R_train, num_users);

  return 0;
}

// Implementation logic

double** create_sparse_matrix(int num_users, int num_items) {
    double** matrix = (double**)malloc(num_users * sizeof(double*));
    if (!matrix) {
        perror("Failed to allocate memory for user rows");
        exit(1);
    }

    for (int i = 0; i < num_users; i++) {
        matrix[i] = (double*)calloc(num_items, sizeof(double));  // calloc to initialize to 0
        if (!matrix[i]) {
            perror("Failed to allocate memory for song columns");
            exit(1);
        }
    }
    return matrix;
}

void fill_sparse_matrix(double** matrix, Rating* ratings, int num_ratings, int num_users, int num_items) {
    for (int i = 0; i < num_ratings; i++) {
        printf("Processing rating %d: user_id=%d, song_id=%d, rating=%f\n", i, ratings[i].user_id, ratings[i].song_id, ratings[i].rating);
        int user_id = ratings[i].user_id;
        int song_id = ratings[i].song_id;
        double rating = ratings[i].rating;

        // Add boundary checks
        if (user_id >= 0 && user_id < num_users && song_id >= 0 && song_id < num_items) {
            matrix[user_id][song_id] = rating;
        } else {
            printf("Error: user_id %d or song_id %d is out of bounds\n", user_id, song_id);
        }
    }
}

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