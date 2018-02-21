# Includes the core code to implement the language evolution model

from __future__ import print_function
import time
import numpy as np
import matplotlib.pyplot as plt
from utils import rand_ints, log
# from utils import log
from mutators import ProbitVectorGaussian
from config import git_strings
git_strings.append("@(#)model.py: $Id$")

# ********** Helper functions ***********
def sanity_check_grammars(meme_probs, grammars):
    for g in grammars:
        assert(np.all( g >= 0))
        assert(np.isclose( g.sum(), 1))
        assert(np.allclose( g.sum(axis=1), meme_probs ))

# Normalize non-negative matrix to be a joint probability matrix with 
# fixed marginals meme_probs
def normalize(meme_probs, p):
    r = p * (meme_probs/p.sum(axis=1))[:,None]
    return r

# Function to initalize random grammars. Meme marginals are fixed to meme_probs
def init_grammars(meme_probs, num_agents, num_memes, num_lexes):
    # Initialize agents with random grammars
    grammars = []
    for i in range(num_agents):
        p = np.random.random((num_memes, num_lexes))
        p = normalize(meme_probs, p)
        grammars.append( p )
    return grammars

# Functions for measuring comprehension (i.e., 1-probability of error) between 
# two grammars. Each grammar is an [num_memes X num_lexes] numpy array
def get_comprehension(speaker_grammar, listener_grammar):
    # Takes in two grammar arrays, returns a number indicating comprehension
    listener_meme_given_lexe = listener_grammar/(listener_grammar.sum(axis=0) + 1e-16)
    
    # Comprehension is given by 
    #    Trace( speaker_grammar.dot(listener_meme_given_lexe^T) )
    # In fact, this is equal to 
    #    sum( speaker_grammar * listener_meme_given_lexe )
    # where * is element-wise multiplication.
    # We use this latter expression, its faster
    return (speaker_grammar*listener_meme_given_lexe).sum()

def get_comprehension_multiple_speakers(speaker_grammars, listener_grammar):
    # Same as calculate_comprehension, but for a list of speaker grammars 
    #  (faster that calling get_comprehension many times, since
    #   listener_meme_given_lexe is only computed once)
    listener_meme_given_lexe = listener_grammar/(listener_grammar.sum(axis=0) + 1e-16)
    comprehension_vals = []
    for sg in speaker_grammars:
        comprehension_vals.append( (sg*listener_meme_given_lexe).sum() )
    return comprehension_vals

def get_comprehension_matrix(grammars):
    # Calculate matrix M of comprehension rates between each pair of agents
    # M[a,b] is comprehension when a is speaker and b is listener
    num_agents = len(grammars)
    comprehension_matrix = np.zeros((num_agents,num_agents))
    for i in range(num_agents):
        comprehension_matrix[:,i] += get_comprehension_multiple_speakers(grammars, grammars[i])
    return comprehension_matrix


# Information theoretic statistics - calculated at the current timepoint and between
# the current and previous timepoints
#  TODO change so these stats can be calculated at arbitrary step delay
def information_moment_ordern(ps, n):
    # calculate nth moment of information for distribution ps.  Entropy is first moment of information
    return np.sum((p*((-np.log2(p))**n)) for p in ps if not np.isclose(p, 0))

def nth_moment_information_population(ps, n):
    # Accept as input a mean (population) grammar. Calculate the nth moment of information of the 
    # population probability distribution over lexes for each given meme
    nth_moment_population = np.zeros(ps.shape[0])
    for i in range(ps.shape[0]):
        nth_moment_population[i] += information_moment_ordern(ps[i,:],n)
    return nth_moment_population

def relative_information_pq(ps, qs):
    # calculate divergence between rows in ps and qs (conditional probabilities for lexes given meme)
    Z = np.zeros(ps.shape[0])
    for i in range(ps.shape[0]):
        a = np.asarray(ps[i,:])
        b = np.asarray(qs[i,:])
        Z[i] += np.sum(np.where(a != 0, a * np.log(a / b), 0))
    return Z

def js_divergence(ps, qs):
    # calculate symmetrized divergence. sqrt(JSD) is a metric
    M = 0.5*(ps + qs)
    return 0.5 * (relative_information_pq(ps, M) + relative_information_pq(qs, M))


# Calculate statistics on the population of grammars
def get_grammars_stats(grammars_tensor, old_grammars_tensor):
    # grammars_tensor is [num_memes, num_lexes, num_agents] np.array for current population
    # old_grammars_tensor is same, but for populating during last reporting interval

    stats = {}
    grammars =[grammars_tensor[:,:,i] for i in range(grammars_tensor.shape[2])]
    stats['Comprehension'] = get_comprehension_matrix(grammars).mean()

    mean_grammar = grammars_tensor.mean(axis=2)
    stats['GrammarVar'] = np.linalg.norm(grammars_tensor-mean_grammar[:,:,None])

    if old_grammars_tensor is not None:
        stats['AgentGrammarDrift'] = \
          np.linalg.norm(grammars_tensor - old_grammars_tensor)
        stats['MeanGrammarDrift'] = \
          np.linalg.norm(mean_grammar-old_grammars_tensor.mean(axis=2))

        # row normalised population grammars
        mean_grammar /= mean_grammar.sum(axis=1)[:,np.newaxis]
        mean_old_grammar = old_grammars_tensor.mean(axis=2)
        mean_old_grammar /= mean_old_grammar.sum(axis=1)[:,np.newaxis]

        # entropy of each meme for current timepoint p(l | M = mi) 
        stats['Entropy'] = nth_moment_information_population(mean_grammar,1)
        stats['2nd_moment_information'] = nth_moment_information_population(mean_grammar,2)
        stats['3rd_moment_information'] = nth_moment_information_population(mean_grammar,3)

        stats['KL_divergence'] = relative_information_pq(mean_grammar, mean_old_grammar)
        stats['JS_divergence'] = js_divergence(mean_grammar, mean_old_grammar) 

    else:
        stats['AgentGrammarDrift'] = np.nan
        stats['MeanGrammarDrift'] = np.nan


    return stats



# ****************** Main simulation function *********************************
def run_simulation(
    grammars,       # List of num_agents numpy arrays, 
                    #   each having shape [num_memes x num_steps]
    meme_probs,     # Numpy array with (fixed) meme probabilities
    num_agents,     # Number of agents 
    num_memes,      # Number of memes
    num_lexes,      # Number of lexes
    num_steps,      # Num_steps
    mutator_class=ProbitVectorGaussian, # Mutation operator class
    mutation_scale=None,  # Scale of mutations to be used by mutation operator
    report_every=10000,   # How often to print stats
    logfile=None,         # Filename to save stats. None means stdout only
    temperature=0         # Temperature (used to decide acceptance)
    ):   
    
    start_time = time.time()

    return_data = []  # Save stats in here and return from function

    # Cache random numbers, so we don't have to call this every time (faster)
    speakers    = rand_ints(num_agents, num_steps)
    listeners   = rand_ints(num_agents, num_steps)
    mutatememes = np.random.choice(num_memes, size=num_steps, p=meme_probs)
    acceptanceprobs = np.random.random(num_steps)

    acceptedsteps = 0   # num. steps accepted in the last report_every interval
    takensteps    = 0   # num. steps taken in the last report_every interval
                        #  (doesn't count skipped steps w/ speaker == listener)
    old_grammars_tensor = None  # Saved grammars from last reporting interval

    log("Step Comprehension AcceptanceRate GrammarVar AgentGrammarDrift MeanGrammarDrift Time", logfile)
    logstring = "%(Step)d %(Comprehension)0.8f %(AcceptanceRate)0.4f " + \
                "%(GrammarVar)0.4f %(AgentGrammarDrift)0.4f " + \
                "%(MeanGrammarDrift)0.4f %(Time)ds"
    # Step is iteration number of the simulation
    # Comprehension is average comprehension
    # AcceptanceRate is proportion of steps accepted in last reporting interval
    # GrammarVar is variance of grammars around mean grammar
    # AgentGrammarDrift is L2 norm of [\Delta grammar] (over report interval),
    #    for all agents
    # MeanGrammarDrift is L2 norm of \Delta meangrammar (over report interval)
    # Time is total elapsed time in seconds

    mutation_op = mutator_class(mutation_scale=mutation_scale)

    for step in range(num_steps):
        if step % report_every == (report_every-1):  # Print logs

            # TODO: Possibly remove to increase performance. However, this 
            # function should already be relatively fast
            sanity_check_grammars(meme_probs, grammars)  

            stats = {}  # Dictionary storing current stats
            stats['Step'] = step
            stats['Time'] = time.time() - start_time
            stats['AcceptanceRate'] = acceptedsteps/float(takensteps)

            grammars_tensor = np.stack(grammars, axis=2)
            # Calculate various stats for current grammar population and add to stats dict
            stats.update(get_grammars_stats(grammars_tensor, old_grammars_tensor))

            log(logstring % stats, logfile)
            return_data.append(stats)

            acceptedsteps = 0
            takensteps    = 0
            old_grammars_tensor = grammars_tensor


        speaker  = speakers[step]
        listener = listeners[step]
        if speaker == listener:   # Skip steps where speaker == listener
            continue
         
        current_meme = mutatememes[step]  # Meme to mutate/communicate

        # Note that mutation_op.mutate requires a 2d numpy array, having
        # shape [anything x num_lexes]. Here we pass in a [1 x num_lexes] array
        current_meme_lexe_probs = grammars[speaker][[current_meme,],:]
        new_probs = mutation_op.mutate(current_meme_lexe_probs)

        # Calculate change in comprehension
        listener_grammar = grammars[listener]

        # Calculate conditional probabilities of current memes, given all lexes
        listener_meme_given_lexe = (listener_grammar[current_meme,:]/(listener_grammar.sum(axis=0) + 1e-16))
        
        # TODO: double check that this gives correct change in comprehension
        change_in_comprehension = (new_probs - current_meme_lexe_probs).dot(listener_meme_given_lexe)
        
        # TODO: here is where we can insert model A, by doing something like
        # np.sum( (new_probs - current_meme_lexe_probs) * 
        #           (random_interaction_vector) * listener_meme_given_lexe)

        if change_in_comprehension >= 0:
            accept_move = True
        elif temperature > 0:
            accept_move = acceptanceprobs[step] <= np.exp(change_in_comprehension/temperature)
        else:
            accept_move = False

        takensteps += 1
        if accept_move:
            acceptedsteps += 1
            grammars[speaker][current_meme,:] = new_probs