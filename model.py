# Includes the core code to implement the language evolution model

from __future__ import print_function
import time
import numpy as np
from datetime import datetime
from collections import OrderedDict

from utils import rand_ints, log
from mutators import ProbitVectorGaussian
from config import get_git_id
from stats import get_grammars_stats, format_stat_val


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


# ****************** Main simulation function *********************************
def run_simulation(grammars, meme_probs, num_agents, num_memes, num_lexes, num_steps,
                   mutator_class=ProbitVectorGaussian, mutation_scale=None, temperature=0,
                   report_every=10000, report_level=2, logfile=None):
    """
    Runs language simulation.

    Parameters
    ----------
    grammars : list of numpy arrays, each having shape [num_memes x num_steps]
        The initial grammar for every agent. Should have num_agent list entries
    meme_probs : [num_memes,] numpy array
        Fixed meme probabilities
    num_agents : int
        Number of agents 
    num_memes : int
        Number of memes
    num_lexes : int
        Number of lexes
    num_steps : int
        Num_steps
    mutator_class : class
        Mutation operator class (default is ProbitVectorGaussian)
    mutation_scale : float
        Scale of mutations to be used by mutation operator. If None, then mutation
        operator class default is used
    temperature : float
        Temperature (used to decide acceptance)
    report_every : int
        Compute and print stats every report_every iterations of the simulation
    report_level : int 
        Detail level of stats to compute (lower is faster)
    logfile : str
        Filename where to log stats. None means stdout only

    Returns
    -------
    stats_data : list 
        list of dicts, each dict containing stats for a given iteration
    grammars : list of numpy arrays, each having shape [num_memes x num_steps]
        The final grammar for every agent. Should have num_agent list entries
    """

    # Log GitHub versions and arguments
    if logfile is not None:
        print("# Logging to %s" % logfile)
    args = OrderedDict(
        num_agents = num_agents,
        num_memes  = num_memes,
        num_lexes  = num_lexes,
        num_steps  = num_steps,
        mutator_class  = mutator_class.__name__,
        mutation_scale = mutation_scale,
        temperature = temperature,
        report_every = report_every,
        report_level = report_level,
    )
    argstring = ", ".join(['%s=%s'%(k, str(v)) for k, v in args.items()])
    log("# GitHub version: %s, run at %s UTC" % (get_git_id(), datetime.utcnow().isoformat()), logfile)
    log("# %s" % argstring, logfile)

    start_time = time.time()

    stats_data = []  # Save stats in here and return from function

    # Cache random numbers, so we don't have to call this every time (faster)
    speakers    = rand_ints(num_agents, num_steps)
    listeners   = rand_ints(num_agents-1, num_steps)
    # Use a trick to avoid having the listener ever be equal to the speaker
    listeners  += (listeners>=speakers).astype('int')
    mutatememes = np.random.choice(num_memes, size=num_steps, p=meme_probs)
    acceptanceprobs = np.random.random(num_steps)

    acceptedsteps = 0   # num. steps accepted in the last report_every interval
    old_grammars_tensor = None  # Saved grammars from last reporting interval

    report_columns = ['Step', 'AcceptanceRate', 'Comprehension', 'GrammarVar',
                      'AgentGrammarDrift', 'MeanGrammarDrift', 'Time']
    log(" ".join(report_columns), logfile)

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

            grammars_tensor = np.stack(grammars, axis=2)
            # Calculate various stats for current grammar population and add to stats dict
            stats = OrderedDict()
            stats['Step'] = step
            stats['AcceptanceRate'] = acceptedsteps/float(report_every)
            stats.update( get_grammars_stats(grammars_tensor, old_grammars_tensor, report_level) )
            stats['Time'] = int(time.time() - start_time)

            log(" ".join([format_stat_val(k, stats.get(k,'-')) for k in report_columns]), logfile)

            stats_data.append(stats)
            acceptedsteps = 0
            old_grammars_tensor = grammars_tensor


        speaker  = speakers[step]
        listener = listeners[step]

        assert(speaker != listener)  # TODO possibly remove

        current_meme = mutatememes[step]  # Meme to mutate/communicate

        # Note that mutation_op.mutate requires a 2d numpy array, having
        # shape [anything x num_lexes]. Here we pass in a [1 x num_lexes] array
        current_meme_lexe_probs = grammars[speaker][[current_meme,],:]
        new_probs = mutation_op.mutate(current_meme_lexe_probs)

        # Calculate change in comprehension
        listener_grammar = grammars[listener]

        # Calculate conditional probabilities of current memes, given all lexes
        listener_meme_given_lexe = (listener_grammar[current_meme,:]/(listener_grammar.sum(axis=0) + 1e-16))

        # We compute change in overall comprehension. However, we use a trick here to speed things
        # up: because we only change the probabilities for a single meme, the change in overall
        # comprehension should be equal to the change in comprehension for that one perturbed meme.
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

        if accept_move:
            acceptedsteps += 1
            grammars[speaker][current_meme,:] = new_probs

    return stats_data, grammars
