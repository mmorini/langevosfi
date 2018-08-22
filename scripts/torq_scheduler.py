#!/usr/bin/python
from __future__ import print_function
import numpy as np
import os, sys
import argparse
from collections import OrderedDict

def get_mutator_classes():
  parentdir = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
  sys.path.append(parentdir)
  from mutators import VALID_MUTATOR_CLASSES
  return VALID_MUTATOR_CLASSES

def get_opts_dict(**kwargs):
  defaultopts = OrderedDict()
  defaultopts['num_agents'] = 30
  defaultopts['num_memes']  = 30
  defaultopts['num_lexes']  = 30
  defaultopts['num_steps']  = 50000000000
  defaultopts['report_every'] = 100000
  defaultopts['init_meme_prob'] = 'UNIFORM'
  defaultopts['terminator_class'] = 'ComprehensionPlateauTerminator'
  defaultopts['terminator_opts'] = 'min_comprehension_mult=5,plateau_percentage=0.99,plateau_time=5'
  defaultopts.update(**kwargs)
  return defaultopts

class QueueAllocator(object):
  # This class is used to spread out jobs evenly between the different queues
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

def printcmd(queueobj, runopts, full_output_dir):
  nm = "run_{num_agents}_{num_memes}_{num_lexes}_mr_{mutation_scale}_{mutator_class}_{init_meme_prob}_{temperature}.txt".format(**runopts)
  full_nm = full_output_dir + nm
  exists = False
  if os.path.exists(full_nm):
    if args.verbose >= 1:
      print('# %s already exists'%full_nm)
    exists = True
  if not exists or args.force:
    queue = queueobj.get_queue()
    runcmdopts = " ".join("--{k}={v}".format(k=k,v=v) for k,v in runopts.items())
    print("echo OMP_NUM_THREADS=1 python run.py {runcmdopts} --logfile {logfile} --mkdir | qsub -q {queue} -N {nm} -o /dev/null -e /dev/null -d {CODEDIR}".format(nm=nm, logfile=full_nm, queue=queue, CODEDIR=CODEDIR, runcmdopts=runcmdopts))

# Followig prevents errors being throw when output is piped and cut off
from signal import signal, SIGPIPE, SIG_DFL
signal(SIGPIPE,SIG_DFL) 
                
CODEDIR = '~/langevosfi/'
DATADIR = '~/ldata/'

RUNTYPES = ['demo','mutator_heatmaps', 'scaling']
parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("runtype", choices=RUNTYPES, help='Which parameter values to loop over')
parser.add_argument("outputdir", type=str, help='Output directory (within %s)'%DATADIR, default=1)
parser.add_argument("--runsubtype", type=str, default='all', help='Runtype choice')
parser.add_argument('--queue', type=str, help='Which queue', default=None)
parser.add_argument('--force', action='store_true', help='Force creation of directories/overwritting', default=False)
parser.add_argument('--verbose', type=int, help='Verbosity level', default=1)
args = parser.parse_args()

OUTPUTDIR = args.outputdir
if OUTPUTDIR[-1] != os.path.sep:
  OUTPUTDIR += os.path.sep
FULL_OUTPUT_DIR = DATADIR + OUTPUTDIR 
VALID_MUTATOR_CLASSES = get_mutator_classes()

print("./cluster_rsync.sh") 

USE_MUTATORS = ['ProbabilityMover', 'ProbitSingleUniform','AdditiveSingleGaussianClip', 'ArcsSingleClip', 'AdditiveSingleClipExppGaussian']

if args.runtype == 'demo':
  queueobj = QueueAllocator(force_queue=args.queue)
  for m in VALID_MUTATOR_CLASSES:
        printcmd(queueobj, get_opts_dict(mutator_class=m.__name__, temperature=1e-5, mutation_scale=0.1, num_agents=10, num_memes=100, num_lexes=100, num_steps=1000000), full_output_dir=FULL_OUTPUT_DIR)


elif args.runtype == 'mutator_heatmaps':
  queueobj = QueueAllocator(force_queue=args.queue, total_count=5*5*len(VALID_MUTATOR_CLASSES))
  for m in VALID_MUTATOR_CLASSES:
    for ms in 10.**np.arange(-3,2):
      for t in 10.**np.arange(-8, -3):
        printcmd(queueobj, get_opts_dict(mutator_class=m.__name__, mutation_scale=ms, temperature=t, num_agents=10, num_memes=100, num_lexes=100, num_steps=10000000), full_output_dir=FULL_OUTPUT_DIR)

elif args.runtype == 'scaling':
  queueobj = QueueAllocator(force_queue=args.queue)
  baseopts = {}
  baseopts['mutation_scale'] = 0.01
  baseopts['temperature'] = 1e-7

  Nvals1 = list(map(int, 10.**np.linspace(1,3,7,endpoint=True)))
  Nvals2 = list(map(int, 10.**np.linspace(1,2,7,endpoint=True)))
  Nvals = sorted(list(set(Nvals1 + Nvals2)))

  for init_meme_prob in ['UNIFORM','RANDOM']:
    baseopts['init_meme_prob'] = init_meme_prob
    for m in USE_MUTATORS:
      if args.runsubtype in ['all', 'lexes']:
	for N in Nvals:
	  printcmd(queueobj, get_opts_dict(mutator_class=m, num_lexes=N, **baseopts), full_output_dir=FULL_OUTPUT_DIR+'lexes/')
      if args.runsubtype in ['all', 'memes']:
	for N in Nvals:
	  printcmd(queueobj, get_opts_dict(mutator_class=m, num_memes=N, **baseopts), full_output_dir=FULL_OUTPUT_DIR+'memes/')
      if args.runsubtype in ['all', 'agents']:
	for N in Nvals:
	  printcmd(queueobj, get_opts_dict(mutator_class=m, num_agents=N, **baseopts), full_output_dir=FULL_OUTPUT_DIR+'agents/')
      if args.runsubtype in ['all', 'lexesmemesagents']:
	for N in Nvals:
	 printcmd(queueobj, get_opts_dict(mutator_class=m, num_agents=N, num_lexes=N, num_memes=N, **baseopts), full_output_dir=FULL_OUTPUT_DIR+'lexesmemesagents/')

else:
  raise Exception('Unknown runtype %s'%args.runtype)

