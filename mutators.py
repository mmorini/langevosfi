from __future__ import print_function
import numpy as np
import scipy.special as spys
import scipy.stats as scps
from utils import probitperturb, rand_ints


class Mutator(object):
    # Base class for mutation operators
    def __init__(self, mutation_scale):
        if mutation_scale is None:
            raise Exception('Must specify mutation_scale')
        if mutation_scale < 0:
            raise Exception('mutation_scale should be >= 0')
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


# New mutaton operators, split by class (DW/AK)

def clip(x):
    # rectifying nonlinearity
    return np.maximum(x,0)

def clip_interval(x):
    # clips to interval [0,1]
    return np.minimum(np.maximum(x,0),1)


# CLASS Ia           % mutators where f is not just a sum, non clipped

class SingleBeta(Mutator):
    # Draw new probability from beta distribution. Set variance to g/1e5 and solve for alpha and beta.
    def _mutate(self, probs):
        lexes = rand_ints(probs.shape[1], probs.shape[0])
        pd = probs[:,lexes]
        a = 1/self.mutation_scale
        b = a/pd - a
        probs = probs.copy()
        probs[:,lexes] = np.random.beta(a, b, None)
        return probs

class VectorDirichlet(Mutator):
    # Draw joint p from a dirichlet distribution with conc. vector alpha defined via probs
    # two params: base measures - underlying probs, and scale s.  As s->inf, variance tends to zero.   
    def _mutate(self, probs):
        return np.random.dirichlet(probs[0,:]*(1000000/self.mutation_scale) + 1e-16, 1)


# CLASS Ib           % mutators where f is not just a sum, clipped

class SingleGamma(Mutator):
    # Draw mutation from a gamma distribution
    def _mutate(self, probs):
        lexes = rand_ints(probs.shape[1], probs.shape[0])
        # set probs[:,lexes] as the mode of the gamma distribution
        alpha = probs[:,lexes] + 1
        probs = probs.copy()
        probs[:,lexes] = clip_interval(self.mutation_scale * scps.gamma.rvs(alpha, scale=1, size=1))
        return probs

class VectorLogMultivariateGamma(Mutator):
    # Associated vector form of gamma distibution
    def _mutate(self, probs):
        U = self.mutation_scale * abs(spys.multigammaln((probs[0,:] + 1 + 1e-16), 1))
        probs = clip_interval(np.reshape(U, (1, U.shape[0])))
        return probs


# CLASS II           % mutators that can increase or decrease probability

class AdditiveSingleClipExppGaussian(Mutator):
    # additive perturbation with standard normal and clipping
    def _mutate(self, probs):
        lexes = rand_ints(probs.shape[1], probs.shape[0])
        probs = probs.copy()
        probs[:,lexes] += clip(self.mutation_scale * (np.exp(probs[:,lexes]) * np.random.randn(probs.shape[0])))
        return probs

class AdditiveVectorClipExppGaussian(Mutator):
    # additive perturbation with uniform(0,var) to cond. prob p(lexe|meme)
    def _mutate(self, probs):
        return clip(probs + self.mutation_scale * (np.exp(probs) * np.random.randn(probs.shape[1])))


class AdditiveSingleAbsExppGaussian(Mutator):
    # additive perturbation with standard normal and clipping
    def _mutate(self, probs):
        lexes = rand_ints(probs.shape[1], probs.shape[0])
        probs = probs.copy()
        probs[:,lexes] += self.mutation_scale * (np.exp(probs[:,lexes]) * np.random.randn(probs.shape[0]))
        return np.abs(probs)

class AdditiveVectorAbsExppGaussian(Mutator):
    # additive perturbation with uniform(0,var) to cond. prob p(lexe|meme)
    def _mutate(self, probs):
        return np.abs(probs + self.mutation_scale * np.exp(probs) * np.random.randn(probs.shape[1]))



# CLASS III           % mutators that only increase probability

class AdditiveSingleExppUniform(Mutator):
    # additive perturbation with standard normal and clipping
    def _mutate(self, probs):
        lexes = rand_ints(probs.shape[1], probs.shape[0])
        probs = probs.copy()
        probs[:,lexes] += self.mutation_scale * (np.exp(probs[:,lexes]) * np.random.rand(probs.shape[0]))
        return probs

class AdditiveVectorExppUniform(Mutator):
    # additive perturbation with uniform(0,var) to cond. prob p(lexe|meme)
    def _mutate(self, probs):
        return probs + self.mutation_scale * (np.exp(probs) * np.random.random(probs.shape))


class ArcsSingleClip(Mutator):
    # Tanmoy's suggested mutation operator
    def _mutate(self,probs):
        lexes = rand_ints(probs.shape[1], probs.shape[0])
        probs = probs.copy()
        probs[:,lexes] = 0.5 * (np.sin(np.arcsin(2*probs[:,lexes]-1) + self.mutation_scale) + 1)
        return clip(probs)

class ArcsVectorClip(Mutator):
    # Tanmoy's suggested mutation operator: vector
    def _mutate(self, probs):
        return clip(0.5 * abs(np.sin(np.arcsin(2*probs-1) + self.mutation_scale) + 1))


class ProbabilityMover(Mutator):
    # Select two lexes for a given meme m, i and j, and move U(0, mutation_scale)*p(i,m) 
    # probability from p(i,m) to p(j,m) 
    def __init__(self, *kargs, **kwargs):
        super(ProbabilityMover, self).__init__(*kargs, **kwargs)
        if self.mutation_scale > 1.0:
            raise Exception('mutation_scale should be <= 1.0')

    def mutate(self, probs):
        from_lexes = rand_ints(probs.shape[1], probs.shape[0])
        to_lexes   = rand_ints(probs.shape[1]-1, probs.shape[0])
        to_lexes  += (to_lexes>=from_lexes).astype('int')
        probs = probs.copy()
        mp = probs[:,from_lexes] * self.mutation_scale
        mp *= np.random.random(mp.shape)
        probs[:,from_lexes] -= mp
        probs[:,to_lexes]   += mp
        return probs


VALID_MUTATOR_CLASSES = [ProbitVectorGaussian, ProbitVectorUniform, AdditiveVectorUniform,
                         ProbitSingleGaussian, ProbitSingleUniform, AdditiveSingleUniform,
                         NoMutation, SingleBeta, VectorDirichlet, SingleGamma, VectorLogMultivariateGamma, AdditiveSingleClipExppGaussian,
                         AdditiveVectorClipExppGaussian, AdditiveSingleAbsExppGaussian,
                         AdditiveVectorAbsExppGaussian, AdditiveSingleExppUniform, AdditiveVectorExppUniform,
                         ArcsSingleClip, ArcsVectorClip, ProbabilityMover]