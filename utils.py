# Miscellaneous utility functions
from __future__ import print_function
import numpy as np

# Draw N random integers from 0 to highval-1 (inclusive)
def rand_ints(highval, N):
    return (np.random.random(N)*highval).astype('int')

# Function to log to file and print to stdout
def log(msg, filename):
    print(msg)
    if filename is not None:
        with open(filename, "a") as f:
            f.write(msg + "\n")

# Probit perturbation function
from scipy.special import erf, ndtri
def normcdf(x):
    return (1.0 + erf(x / np.sqrt(2.0))) / 2.0
def probitperturb(x,p):
    return normcdf(ndtri(x) + p)
