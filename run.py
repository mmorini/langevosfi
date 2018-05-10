# Run model from command line

from __future__ import print_function
import argparse
import numpy as np
import time
import os

import mutators
import terminators
import model
from utils import log

class StoreDictKeypair(argparse.Action):
     def __call__(self, parser, namespace, values, option_string=None):
         cdict = {}
         for kv in values.split(","):
             k,v = kv.split("=")
             cdict[k] = v
         getattr(namespace, self.dest).update(cdict)

mutatorclassnames = list(map(lambda x: x.__name__, mutators.VALID_MUTATOR_CLASSES))
terminatorclassnames = list(map(lambda x: x.__name__, terminators.VALID_TERMINATOR_CLASSES))

parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("--num_agents", type=int, help="Number of agents", default=10)
parser.add_argument("--num_memes" , type=int, help="Number of memes", default=10)
parser.add_argument("--num_lexes" , type=int, help="Number of lexes", default=10)
parser.add_argument("--num_steps" , type=int, help="Number of steps", default=1000000)
parser.add_argument("--init_meme_prob", type=str, choices=['UNIFORM','RANDOM'],
                    help="How to initialize meme probabilities", default='UNIFORM')
parser.add_argument("--mutator_class", type=str, choices=mutatorclassnames, help="Mutation operator", default=mutatorclassnames[0])
parser.add_argument("--mutation_scale" , type=float, help="Mutation scale", default=0.1)
parser.add_argument("--temperature", type=float, help="Temperature for acceptance step", default=0)
parser.add_argument("--terminator_class", type=str, choices=[None,]+terminatorclassnames, help="Early termination operator", default=None)
parser.add_argument("--terminator_opts", action=StoreDictKeypair, default={}, metavar="KEY1=VAL1,KEY2=VAL2...")
parser.add_argument("--report_every", type=int, help="How often to log stats", default=100000)
parser.add_argument("--report_level", type=int, help="Level of detail to include in reports", default=3)
parser.add_argument("--logfile", type=str, help="Output file to log to")
parser.add_argument("--mkdir", action='store_true', default=False, help="Create logfile directory if it doesn't exist")

args = parser.parse_args()

# Generate probability of memes (this will be fixed)
if args.init_meme_prob == 'UNIFORM':
  meme_probs = np.ones(args.num_memes) / args.num_memes
elif args.init_meme_prob == 'RANDOM':
  meme_probs = np.random.random(args.num_memes)
  meme_probs /= meme_probs.sum()

grammars = model.init_grammars(meme_probs,
                               num_agents = args.num_agents,
                               num_memes  = args.num_memes,
                               num_lexes  = args.num_lexes)

terminator = None
if args.terminator_class is not None:
  try:
    terminator = getattr(terminators, args.terminator_class)(**args.terminator_opts)
  except:
    print("Could not initialize terminator class %s, please check your terminator_opts" 
           % args.terminator_class)
    raise

if args.logfile:
  logdir = os.path.dirname(args.logfile)
  if not os.path.exists(logdir):
    if args.mkdir:
      print("# Making directory %s" % logdir)
      os.makedirs(logdir)
    else:
      raise Exception("Directory %s doesn't exist" % logdir)

model.run_simulation(grammars,
                     meme_probs = meme_probs,
                     num_agents = args.num_agents,
                     num_memes  = args.num_memes,
                     num_lexes  = args.num_lexes,
                     num_steps  = args.num_steps,
                     mutator_class = getattr(mutators, args.mutator_class),
                     mutation_scale = args.mutation_scale,
                     temperature = args.temperature,
                     early_terminator=terminator,
                     report_every = args.report_every,
                     report_level = args.report_level,
                     logfile=args.logfile)
