from __future__ import print_function
import numpy as np
import model

def test_run_simulation():
	basic_params = dict(num_agents=5, num_memes=5, num_lexes=5)
	meme_probs = np.random.random(basic_params['num_memes'])
	meme_probs /= meme_probs.sum()
	basic_params['meme_probs'] = meme_probs

	# Square grammars
	grammars = model.init_grammars(**basic_params)
	model.run_simulation(grammars, mutation_scale=0.1, num_steps=1000, report_every=100, **basic_params)

	# Less lexes than memes
	basic_params['num_lexes'] = 3
	grammars = model.init_grammars(**basic_params)
	model.run_simulation(grammars, mutation_scale=0.1, num_steps=1000, report_every=100, **basic_params)

	# More lexes than memes
	basic_params['num_lexes'] = 7
	grammars = model.init_grammars(**basic_params)
	model.run_simulation(grammars, mutation_scale=0.1, num_steps=1000, report_every=100, **basic_params)
