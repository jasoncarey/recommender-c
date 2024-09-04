#include "model.h"
#include "util.h"
#include "sparse_matrix.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

void initialize_matrix_pq(double **matrix, int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            matrix[i][j] = ((double)rand() / RAND_MAX) * 0.1;
        }
    }
}

double calculate_mean(Rating *ratings, int num_ratings)
{
    double sum = 0.0;
    for (int i = 0; i < num_ratings; i++)
    {
        sum += ratings[i].rating;
    }
    return sum / num_ratings;
}

double calculate_stddev(Rating *ratings, int num_ratings, double mean)
{
    double sum = 0.0;
    for (int i = 0; i < num_ratings; i++)
    {
        sum += pow(ratings[i].rating - mean, 2);
    }
    return sqrt(sum / num_ratings);
}

void normalize_ratings(Rating *ratings, int num_ratings, double mean, double stddev)
{
    for (int i = 0; i < num_ratings; i++)
    {
        ratings[i].rating = (ratings[i].rating - mean) / stddev;
    }
}

double denormalize_prediction(double normalized_prediction, double mean, double stddev)
{
    return normalized_prediction * stddev + mean;
}

void calculate_rating_frequencies(Rating *ratings, int num_ratings, double *freq)
{
    // Initialize frequency counts to zero
    for (int i = 1; i <= 5; i++)
    {
        freq[i] = 0.0;
    }

    // Count frequency of each rating
    for (int i = 0; i < num_ratings; i++)
    {
        int rating_value = (int)ratings[i].rating;

        if (rating_value >= 1 && rating_value <= 5)
        {
            freq[rating_value]++;
            printf("Counting rating: %d, Current frequency for %d: %f\n", rating_value, rating_value, freq[rating_value]);
        }
        else
        {
            // printf("Unexpected rating value: %d\n", rating_value);
        }
    }

    // Normalize frequencies
    for (int i = 1; i <= 5; i++)
    {
        freq[i] /= num_ratings;
        printf("Frequency for rating %d: %f\n", i, freq[i]);
    }
}

void calculate_rating_frequencies_from_raw(const char *filename, double *freq)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Failed to open file for frequency calculation");
        exit(1);
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int user_id, song_id;
    double rating;
    int count = 0;

    // Initialize frequency array to zero
    for (int i = 1; i <= 5; i++)
    {
        freq[i] = 0;
    }

    while ((read = getline(&line, &len, file)) != -1)
    {
        if (sscanf(line, "%d,%d,%lf", &user_id, &song_id, &rating) == 3)
        {
            int rounded_rating = (int)round(rating);

            if (rounded_rating >= 1 && rounded_rating <= 5)
            {
                freq[rounded_rating]++;
                count++;
            }
        }
    }

    fclose(file);
    if (line)
    {
        free(line);
    }

    // Convert counts to frequency percentages
    for (int i = 1; i <= 5; i++)
    {
        freq[i] /= count;
    }
}

void assign_weights(double *weights, double *freq)
{
    for (int i = 1; i <= 5; i++)
    {
        if (freq[i] > 0)
        {
            weights[i] = 1.0 / freq[i]; // Inverse of frequency
        }
        else
        {
            weights[i] = 0.0; // Set weight to zero if the frequency is zero
            printf("Warning: Frequency for rating %d is zero, setting weight to 0\n", i);
        }
    }
}

double calculate_weighted_sse(SparseMatrix *R, double **P, double **Q, int num_users, int num_items, int num_factors, double *weights, double mean, double stddev) {
    double sse = 0.0;
    for (int i = 0; i < num_users; i++) {
        RatingNode *node = R->users[i].ratings;
        while (node) {
            double prediction = 0.0;
            for (int k = 0; k < num_factors; k++) {
                prediction += P[i][k] * Q[node->song_id][k];
            }

            double error = node->rating - prediction;

            // Assuming node->original_rating is the original integer rating before normalization
            int original_rating = (int)round((node->rating * stddev) + mean);

            if (original_rating >= 1 && original_rating <= 5) {
                sse += weights[original_rating] * error * error; // Apply weight based on original rating
            } else {
                // Handle unexpected cases
                printf("Warning: Rating %d out of expected range 1-5\n", original_rating);
            }
            node = node->next;
        }
    }
    // printf("Weighted SSE: %f\n", sse);
    return sse;
}

double calculate_sse(SparseMatrix *R, double **P, double **Q, int num_users, int num_items, int num_factors)
{
    double sse = 0.0;
    int count = 0;

    for (int i = 0; i < num_users; i++)
    {
        RatingNode *node = R->users[i].ratings;
        while (node)
        {
            double prediction = 0.0;
            for (int k = 0; k < num_factors; k++)
            {
                prediction += P[i][k] * Q[node->song_id][k];
            }

            double error = node->rating - prediction;
            sse += error * error;

            // Debugging output for first few items
            // if (count < 10) {
            //     printf("User %d, Item %d: Actual = %f, Predicted = %f, Error = %f\n", i, node->song_id, node->rating, prediction, error);
            // }
            count++;
            node = node->next;
        }
    }
    return sse;
}

void train_model(SparseMatrix *R, double **P, double **Q, int num_users,
                 int num_items, int num_ratings, int num_factors, int epochs,
                 double learning_rate, double lambda, double *weights, double mean, double stddev)
{
    for (int i = 1; i <= 5; i++)
    {
        printf("Weight for rating %d: %f\n", i, weights[i]);
    }
    printf("\n");
    for (int epoch = 0; epoch < epochs; epoch++)
    {
        for (int i = 0; i < num_users; i++)
        {
            RatingNode *node = R->users[i].ratings;
            while (node)
            {
                double prediction = 0.0;
                for (int k = 0; k < num_factors; k++)
                {
                    prediction += P[i][k] * Q[node->song_id][k];
                }
                double error = node->rating - prediction;
                for (int k = 0; k < num_factors; k++)
                {
                    double p_ik = P[i][k];
                    double q_jk = Q[node->song_id][k];

                    P[i][k] += learning_rate * (error * q_jk - lambda * p_ik);
                    Q[node->song_id][k] += learning_rate * (error * p_ik - lambda * q_jk);
                }
                node = node->next;
            }
        }

        if (epoch % 100 == 0)
        {
            double sse = calculate_weighted_sse(R, P, Q, num_users, num_items, num_factors, weights, mean, stddev);
            printf("Epoch %d: Weighted MSE: %1f\n", epoch, sse / num_ratings);

            // Print some intermediate values
            for (int i = 0; i < num_users && i < 5; i++)
            {
                RatingNode *node = R->users[i].ratings;
                while (node)
                {
                    double prediction = predict_rating(P, Q, i, node->song_id, num_factors);
                    double error = node->rating - prediction;
                    // printf("User %d, Item %d: Actual = %f, Predicted = %f, Error = %f\n", i, node->song_id, node->rating, prediction, error);
                    node = node->next;
                }
            }
        }
    }
}

double predict_rating(double **P, double **Q, int user_id, int item_id,
                      int num_factors)
{
    double prediction = 0.0;
    for (int k = 0; k < num_factors; k++)
    {
        prediction += P[user_id][k] * Q[item_id][k];
    }
    return prediction;
}

void recommend_items(double **P, double **Q, int user_id, int num_items, int num_factors, int k, double mean, double stddev)
{
    ItemRating *items = malloc(num_items * sizeof(ItemRating));
    for (int item_id = 0; item_id < num_items; item_id++)
    {
        double normalized_pred = predict_rating(P, Q, user_id, item_id, num_factors);
        double final_pred = denormalize_prediction(normalized_pred, mean, stddev);

        if (final_pred < 1.0) final_pred = 1.0;
        if (final_pred > 5.0) final_pred = 5.0;

        items[item_id].item_id = item_id;
        items[item_id].rating = final_pred;
    }
    qsort(items, num_items, sizeof(ItemRating), compare_items);
    printf("\nTop %d items for user %d:\n", k, user_id + 1);
    for (int i = 0; i < k; i++)
    {
        printf("Item %d with predicted rating %1f\n", items[i].item_id + 1, items[i].rating);
    }
    free(items);
}

int compare_items(const void *a, const void *b)
{
    double diff = ((ItemRating *)b)->rating - ((ItemRating *)a)->rating;
    return (diff > 0) - (diff < 0); // Returns -1 if b > a, 1 if a > b, 0 if equal
}