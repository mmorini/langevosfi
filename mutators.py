from __future__ import print_function
import numpy as np
from utils import probitperturb, rand_ints
from config import git_strings
git_strings.append("@(#)mutators.py: $Id$")

class Mutator(object):
    # Base class for mutation operators
    def __init__(self, mutation_scale):
        if mutation_scale is None:
            raise Exception('Must specify mutation_scale')
        self.mutation_scale = mutation_scale
        
    def mutate(self, probs):
        # probs is a [_ X num_lexes] numpy array of probabilities to perturb
        saved_sums = probs.sum(axis=1)
        newprobs   = self._mutate(probs)
        new_sums   = newprobs.sum(axis=1)
        return newprobs * (saved_sums / new_sums)
        
    def _mutate(self, probs):
        # probs is a [_ X num_lexes] numpy array of probabilities to perturb
        # Called by mutate. This method does mutation without normalization,
        #   while normalization is handled by the mutate method
        raise NotImplementedError
        
class ProbitVectorGaussian(Mutator):
    # probit perturbation with N(var) to cond. prob p(lexe|meme)
    def _mutate(self, probs):
        return probitperturb(probs, self.mutation_scale * np.random.randn(*probs.shape))
        
class ProbitVectorUniform(Mutator):
    # probit perturbation with uniform(0,var) to cond. prob p(lexe|meme) 
    def _mutate(self, probs):
        return probitperturb(probs, self.mutation_scale * np.random.rand(*probs.shape))
        
class AdditiveVectorUniform(Mutator):
    # additive perturbation with uniform(0,var) to cond. prob p(lexe|meme)
    def _mutate(self, probs):
        return probs + self.mutation_scale * np.random.random(probs.shape)
        
        
class ProbitSingleGaussian(Mutator):
    # probit perturbation with N(var) to single lexe p(lexe|meme) for each meme
    def _mutate(self, probs):
        lexes = rand_ints(probs.shape[1], probs.shape[0])
        probs = probs.copy()
        probs[:,lexes] = probitperturb(probs[:,lexes], self.mutation_scale * np.random.randn(probs.shape[0]))
        return probs

class ProbitSingleUniform(Mutator):
    # probit perturbation with uniform(0, var) to single lexe p(lexe|meme) for each meme
    def _mutate(self, probs):
        lexes = rand_ints(probs.shape[1], probs.shape[0])
        probs = probs.copy()
        probs[:,lexes] = probitperturb(probs[:,lexes], self.mutation_scale * np.random.rand(probs.shape[0]))
        return probs

class AdditiveSingleUniform(Mutator):
    # additive perturbation with uniform(0, var) to single lexe p(lexe|meme) for each meme
    def _mutate(self, probs):
        lexes = rand_ints(probs.shape[1], probs.shape[0])
        probs = probs.copy()
        probs[:,lexes] += self.mutation_scale * np.random.rand(probs.shape[0])
        return probs
    
class NoMutation(Mutator):
    def _mutate(self, probs):
        return probs

VALID_MUTATOR_CLASSES = [ProbitVectorGaussian, ProbitVectorUniform, AdditiveVectorUniform, 
                         ProbitSingleGaussian, ProbitSingleUniform, AdditiveSingleUniform,
                        NoMutation]
