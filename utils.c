#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

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

    printf("Loaded %d ratings. Max user ID: %d, Max song ID: %d\n", *num_ratings, *max_user_id, *max_song_id);

    return ratings;
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