# Includes the core code to implement the language evolution model

from __future__ import print_function
import time
import numpy as np
from utils import rand_ints
from utils import log
from mutators import ProbitVectorGaussian

# ********** Helper functions ***********

# Normalize non-negative matrix to be a joint probability matrix with fixed meme_probs
def normalize(meme_probs, p):
    r = p * (meme_probs/p.sum(axis=1)[:,None])
    return r

# Function to initalize random grammars.  Meme marginals are set to meme_probs
def init_grammars(meme_probs, num_agents, num_memes, num_lexes):
    # Initialize agents with random grammars
    grammars = []
    for i in range(num_agents):
        p = np.random.random((num_memes, num_lexes)).astype('float32')
        p = normalize(meme_probs, p)
        grammars.append( p )
    return grammars


# Functions for measuring comprehension (i.e., 1-probability of error) between two grammars
# Each grammar is an [num_memes X num_lexes] numpy array
def get_comprehension(speaker_grammar, listener_grammar):
    # Takes in two grammar arrays, returns a positive number indicating comprehension
    listener_meme_given_lexe = listener_grammar/(listener_grammar.sum(axis=0) + 1e-16)
    
    # Comprehension is given by Trace( speaker_grammar.dot(listener_meme_given_lexe^T) )
    # Turns out this is equal to sum( speaker_grammar * listener_meme_given_lexe )
    #  where * is entrywise multiplication. This latter expression is faster to compute
    return (speaker_grammar*listener_meme_given_lexe).sum()

def get_comprehension_multiple_speakers(speaker_grammars, listener_grammar):
    # Same as calculate_comprehension, but for a list of speaker grammars 
    #  (faster that calling get_comprehension many times, since listener_meme_given_lexe 
    #   is only computed once)
    listener_meme_given_lexe = listener_grammar/(listener_grammar.sum(axis=0) + 1e-16)
    comprehension_vals = []
    for sg in speaker_grammars:
        comprehension_vals.append( (sg*listener_meme_given_lexe).sum() )
    return comprehension_vals

def get_comprehension_matrix(grammars):
    # Calculate matrix of comprehension rates between each pair of agents
    # TODO: Double check that this doesn't just measure comprehension 'one-way' 
    #    (i.e., each agent is either speaker or receiver). It should do both ways
    num_agents = len(grammars)
    comprehension_matrix = np.zeros((num_agents,num_agents))
    for i in range(num_agents):
        comprehension_matrix[:,i] += get_comprehension_multiple_speakers(grammars, grammars[i])
    return comprehension_matrix


# ****************** Main simulation function *********************************
def run_simulation(grammars,     # List of num_agents numpy arrays, each one having shape [num_memes x num_steps]
                   meme_probs,   # List of meme probabilities, [num_memes] numpy array
                   num_agents,   # Number of agents 
                   num_memes,    # Number of memes
                   num_lexes,    # Number of lexes
                   num_steps,    # Num_steps
                   mutator_class=ProbitVectorGaussian,   # Mutation operator class
                   mutation_scale=None,  # Scale of mutations to be used by mutation operator
                   report_every=10000,   # How often to print stats
                   logfile=None,         # Filename to which to print stats. None for stdout only
                   temperature=0         # Temperature (used to decide acceptance)
                   ):   
    
    
    start_time = time.time()

    return_data = []  # Save stats in here and return from function

    # Cache random numbers, so we don't have to call this every time (faster)
    speakers  = rand_ints(num_agents, num_steps)
    listeners = rand_ints(num_agents, num_steps)
    mutatememes = rand_ints(num_memes, num_steps) # TODO, sample with meme probs
    mutatelexes = rand_ints(num_lexes, num_steps)
    acceptanceprobs = np.random.random(num_steps)

    acceptedsteps = 0   # How many steps were accepted in the last report_every interval
    takensteps    = 0   # How many steps were taken in the last report_every interval
                        #  (this discounts the steps that were skipped when, by chance, speaker == listener
    
    log("Step Comprehension AcceptanceRate Time", logfile)

    mutation_op = mutator_class(mutation_scale=mutation_scale)

    for step in range(num_steps):
        if step % report_every == (report_every-1):  # Print logs
            mean_comprehension = get_comprehension_matrix(grammars).mean() 
            mean_acceptance    = acceptedsteps/takensteps
            log('%d %0.8f %0.4f %ds' % (step, mean_comprehension, mean_acceptance, time.time() - start_time), logfile)
            return_data.append(
                {'step'          : step, 
                 'comprehension' : mean_comprehension, 
                 'acceptance'    : mean_acceptance, 
                 'time': time.time() - start_time})
            acceptedsteps = 0
            takensteps    = 0

        speaker  = speakers[step]
        listener = listeners[step]
        if speaker == listener:   # Skip steps where speaker == listener
            continue
         
        current_meme = mutatememes[step]  # Meme to mutate/communicate

        # Note that mutation_op.mutate requires a 2d numpy array, of shape [anything x num_lexes]
        # In this case, we pass it a [1 x num_lexes] array
        current_meme_lexe_probs = grammars[speaker][[current_meme,],:]
        new_probs = mutation_op.mutate(current_meme_lexe_probs)

        # Calculate change in comprehension
        listener_grammar = grammars[listener]

        # Calculate conditional probabilities  of current memes, given all lexes
        listener_meme_given_lexe = (listener_grammar[current_meme,:]/(listener_grammar.sum(axis=0) + 1e-16))
        
        # TODO: double check that this will give correct change in comprehension
        change_in_comprehension = (new_probs - current_meme_lexe_probs).dot(listener_meme_given_lexe)
        
        # TODO: here is where we can insert model A, by doing something like
        # np.sum( (new_probs - current_meme_lexe_probs) * (random_interaction_vector) * listener_meme_given_lexe)

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


    return return_data
