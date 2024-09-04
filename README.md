# Collaborative Filtering Recommender System

This is a C implementation of a collaborative filtering recommender system.

It expects a CSV file with the following format:

```C
    userID: number;
    songID: number;
    rating: number; (1-5)
```

The program will train a model using the data and then recommend songs for a user.

I trained the model on a dataset of 2,000,000 song ratings from [kaggle](https://www.kaggle.com/datasets/rymnikski/dataset-for-collaborative-filters)

## Usage

Place the CSV file in the `data` directory
Update the CSV file path in `main.c`

Run `make` to compile the program
Run `./recommender` to run the program
