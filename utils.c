#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        fclose(file);
        exit(1);
    }

    *max_user_id = 0;
    *max_song_id = 0;

    // Skip header line if there is one
    if ((read = getline(&line, &len, file)) != -1) {
        if (strstr(line, "userID") != NULL || strstr(line, "songID") != NULL || strstr(line, "rating") != NULL) {
            printf("Skipping header line: %s", line);
        } else {
            // If no header, reset file pointer to the start of the file
            fseek(file, 0, SEEK_SET);
        }
    }

    while ((read = getline(&line, &len, file)) != -1) {
        // Trim any trailing newline or carriage return characters
        line[strcspn(line, "\r\n")] = 0;

        // Attempt to parse the line
        if (sscanf(line, "%d,%d,%lf", &user_id, &song_id, &rating) == 3) {
            // Check if rating is within valid bounds
            if (rating < 1 || rating > 5) {
                printf("Skipping out-of-bounds rating: %f on line: %s\n", rating, line);
                continue;
            }

            // Reallocate memory if capacity is exceeded
            if (count >= capacity) {
                capacity *= 2;
                Rating* new_ratings = realloc(ratings, capacity * sizeof(Rating));
                if (!new_ratings) {
                    free(ratings);
                    fclose(file);
                    perror("Failed to reallocate memory for ratings");
                    exit(1);
                }
                ratings = new_ratings;
            }

            // Assign parsed values to the ratings array
            ratings[count].user_id = user_id;
            ratings[count].song_id = song_id;
            ratings[count].rating = rating;

            // Update max user and song IDs
            if (user_id > *max_user_id) *max_user_id = user_id;
            if (song_id > *max_song_id) *max_song_id = song_id;

            count++;
        } else {
            //printf("Skipping invalid line: %s\n", line);
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