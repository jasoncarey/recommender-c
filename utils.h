#ifndef UTILS_H
#define UTILS_H

typedef struct {
    int user_id;
    int song_id;
    double rating;
} Rating;

// Function declarations related to data loading and matrix creation
Rating* load_csv_data(const char* filename, int* num_ratings, int* max_user_id, int* max_song_id);
double **create_matrix(int rows, int cols);
void free_matrix(double **matrix, int rows);

#endif // UTILS_H