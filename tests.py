from __future__ import print_function
import numpy as np
import model

def _call_run(**pargs):
	args = dict(num_agents=5, num_memes=5, 
		num_lexes=5, mutation_scale=0.1, num_steps=300, report_every=100)
	args.update(**pargs)
	print(args)
	meme_probs = np.random.random(args['num_memes'])
	meme_probs /= meme_probs.sum()
	grammars = model.init_grammars(num_agents=args['num_agents'],
		num_lexes=args['num_lexes'], num_memes=args['num_memes'],
		meme_probs=meme_probs)
	model.run_simulation(grammars=grammars, meme_probs=meme_probs, **args)

def test_run_simulation():
	# Square grammars
	_call_run(num_agents=10, num_memes=5, num_lexes=5)

	# Less lexes than memes
	_call_run(num_agents=10, num_memes=5, num_lexes=3)

	# More lexes than memes
	_call_run(num_agents=10, num_memes=3, num_lexes=5)

def test_run_simulation_report_levels():
	_call_run(report_level=0)
	_call_run(report_level=1)
	_call_run(report_level=2)
	_call_run(report_level=3)
	