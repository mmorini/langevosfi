# langevosfi
Language Evolution as seen from the SFI

Python Implementation of Model.  Python3 recommended but not required.

**Included files**
* `run.py` provides a command line interface, try ``python3 run.py --help``  for options.
* `model.py` includes the main code to run the language dynamics model. `run_simulation` is main function to run the simulation
* `mutators.py` includes classes for different mutation operators
* `utils.py` includes miscellaneous utility functions

**Output of `run_simulation`**
Includes several columns:
* Step is iteration number of the model
* Comprehension is average comprehension
* AcceptanceRate is proportion of steps accepted in last reporting interval
* GrammarVar is variance of grammars around mean grammar
* AgentGrammarDrift is L2 norm of ΔGrammar (over report interval),for all agents
* MeanGrammarDrift is L2 norm of Δ\[Mean Grammar\] (over report interval)
* Time is total elapsed time, in seconds

**Example usage**
* Run with standard mutation operator with 100 agents, 50 memes, and 50 lexes  
``python3 run.py --num_steps=100000000 --num_agents=100 --num_memes=50 --num_lexes=50  --report_every=100000  --temperature=0.000001``
* Try with different mutation operator, `AdditiveSingleUniform`, which just adds a number to the probability of a random lexe for a given meme  
``python3 run.py --num_steps=10000000 --num_agents=30 --num_memes=20 --num_lexes=20  --report_every=100000  --temperature=0.000001 --mutator_class=AdditiveSingleUniform``  
Interestingly, this mutation operator shows convergence to perfect comprehension. See `mutators.py` for more mutation operators


