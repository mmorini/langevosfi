from __future__ import print_function
import numpy as np
import model

def _call_run(**pargs):
	args = dict(num_agents=5, num_memes=5,
		num_lexes=5, mutation_scale=0.1, num_steps=300, report_every=100)
	args.update(**pargs)
	if 'meme_probs' not in args:
		meme_probs = np.random.random(args['num_memes'])
		args['meme_probs'] = meme_probs / meme_probs.sum()
	grammars = model.init_grammars(num_agents=args['num_agents'],
		num_lexes=args['num_lexes'], num_memes=args['num_memes'],
		meme_probs=args['meme_probs'])
	model.run_simulation(grammars=grammars, **args)

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

def test_run_simulation_comprehension_stats():
	grammars = [np.eye(10)/10.,]*10
	args = dict( num_agents=10,  num_memes=10, num_lexes=10,
		meme_probs=np.ones(10)/10., num_steps=1, report_every=1,
		mutation_scale=0.1)
	stats, _ = model.run_simulation(grammars=grammars, **args)
	assert(len(stats)==1)
	assert(np.isclose(stats[0]['Comprehension'], 1.))
	assert(np.isclose(stats[0]['GrammarVar'], 0.))
	for i in range(len(grammars)):
	    grammars[i][:] = 0
	    grammars[i][:,0]=0.1
	stats, _ = model.run_simulation(grammars, **args)
	assert(len(stats)==1)
	assert(np.isclose(stats[0]['Comprehension'], 0.1))
	assert(np.isclose(stats[0]['GrammarVar'], 0.))
