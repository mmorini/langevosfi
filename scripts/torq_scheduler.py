#!/usr/bin/python
from __future__ import print_function
import numpy as np
import os, sys
import argparse

def get_mutator_classes():
  parentdir = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
  sys.path.append(parentdir)
  from mutators import VALID_MUTATOR_CLASSES
  return VALID_MUTATOR_CLASSES

# Followig prevents errors being throw when output is piped and cut off
from signal import signal, SIGPIPE, SIG_DFL
signal(SIGPIPE,SIG_DFL) 
                
CODEDIR = '/home/artemy/langevosfi/'
DATADIR = '/home/artemy/ldata/'
DATAHOST = 'habanero'

parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("--mode", type=int, help='Mode', default=1)
parser.add_argument('--queue', type=str, help='Which queue', default=None)
parser.add_argument('--force', action='store_true', help='Force creation of directories/overwritting', default=False)
parser.add_argument('--verbose', type=int, help='Verbosity level', default=1)
args = parser.parse_args()


print("./cluster_rsync.sh") 

defaultopts = {
                #'num_agents': 100,
                #'num_memes' : 100,
                #'num_lexes' : 100,
                'num_agents': 30,
                'num_memes' : 30,
                'num_lexes' : 30,
                'num_steps' : 500000000,
                'report_every': 100000,
                'extraopts': '--terminator_class ComprehensionPlateauTerminator --terminator_opts min_comprehension_mult=5,plateau_percentage=0.99,plateau_time=5',
}

class QueueAllocator(object):
  def __init__(self, total_count=None, force_queue=None):
    self.counter = 0
    self.force_queue = force_queue
    if total_count is not None:
      self.total_count = total_count
    else:
      self.total_count = 256

  def get_queue(self):
    if self.force_queue is not None:
      return self.force_queue

    self.counter += 1
    if self.counter <= 0.25*self.total_count:
      return 'high'
    elif self.counter <= 0.5*self.total_count:
      return 'medium'
    else:
      return 'batch'

def printcmd(queueobj, **kwargs):
  opts = defaultopts.copy()
  opts.update(kwargs)

  outputdir = opts['outputdir']
  #if not os.path.exists(outputdir):
  #  if not args.force:
  #    raise Exception("Ouput directory (%s) doesn't exist"%outputdir)
  #  else:
  #    print("# Making directory %s" % outputdir)
  #    os.makedirs(outputdir)

  nm = "run_{num_agents}_{num_memes}_{num_lexes}_mr_{mutation_scale}_{mutator_class}_{temperature}.txt".format(**opts)
  full_nm = outputdir + nm
  exists = False
  if os.path.exists(full_nm):
    if args.verbose >= 1:
      print('# %s already exists'%full_nm)
    exists = True
  if not exists or args.force:
    queue = queueobj.get_queue()
    print("echo OMP_NUM_THREADS=1 python run.py --num_agents {num_agents} --num_memes {num_memes} --num_lexes {num_lexes} --report_every={report_every} --num_steps={num_steps} --mutator_class={mutator_class} --mutation_scale={mutation_scale} --temperature={temperature} {extraopts} --logfile {logfile} --mkdir | qsub -q {queue} -N {nm} -o /dev/null -e /dev/null -d {CODEDIR}".format(nm=nm, logfile=full_nm, queue=queue, CODEDIR=CODEDIR, outputhost=DATAHOST, **opts))

if args.mode == 1:
  VALID_MUTATOR_CLASSES = get_mutator_classes()
  queueobj = QueueAllocator(force_queue=args.queue, total_count=5*5*len(VALID_MUTATOR_CLASSES))
  BASE_OUTPUT = DATADIR + 'sfi_v005/'
  for m in VALID_MUTATOR_CLASSES:
    #for ms in 10.**np.arange(-4,3):
    #  for t in 10.**np.arange(-8, 0):
    for ms in 10.**np.arange(-3,2):
      for t in 10.**np.arange(-8, -3):
        # printcmd(args, mutator_class=m.__name__, mutation_scale=ms, temperature=t, num_agents=100, num_memes=10, num_lexes=10, outputdir=BASE_OUTPUT)
        printcmd(queueobj, mutator_class=m.__name__, mutation_scale=ms, temperature=t, num_agents=10, num_memes=100, num_lexes=100, outputdir=BASE_OUTPUT,
                 num_steps=10000000)

elif args.mode == 2 or args.mode ==3:
  queueobj = QueueAllocator(force_queue=args.queue)
  # defaultopts['mutation_scale'] = 0.1
  defaultopts['mutation_scale'] = 0.01
  defaultopts['temperature'] = 1e-7
  #if args.mode == 2:
  #  defaultopts['temperature'] = 1e-6
  #elif args.mode == 3:
  #  defaultopts['temperature'] = 1e-5

  BASE_OUTPUT = DATADIR + 'sfi_scaling006/'
  Nvals1 = list(map(int, 10.**np.linspace(1,3,7,endpoint=True)))
  Nvals2 = list(map(int, 10.**np.linspace(1,2,7,endpoint=True)))
  Nvals = sorted(list(set(Nvals1 + Nvals2)))
 
  for m in ['ProbabilityMover', 'ProbitSingleUniform','AdditiveSingleGaussianClip', 'ArcsSingleClip', 'AdditiveSingleClipExppGaussian']:
    for N in Nvals:
      printcmd(queueobj, mutator_class=m, num_lexes=N, outputdir=BASE_OUTPUT+'lexes/')
    for N in Nvals:
      printcmd(queueobj, mutator_class=m, num_memes=N, outputdir=BASE_OUTPUT+'memes/')
    for N in Nvals:
      printcmd(queueobj, mutator_class=m, num_agents=N, outputdir=BASE_OUTPUT+'agents/')
    for N in Nvals:
      printcmd(queueobj, mutator_class=m, num_agents=N, num_lexes=N, num_memes=N, outputdir=BASE_OUTPUT+'lexesmemesagents/')

