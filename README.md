# langevosfi
Language Evolution as seen from the SFI

Python Implementation of Model.  Python3 recommended but not required.

`run.py` provides a command line interface, see
``
python3 run.py --help
``
for options.

Example usage:

* Show convergence with 100 agents, 50 memes, and 50 lexes  
``python3 run.py --num_steps=100000000 --num_agents=100 --num_memes=50 --num_lexes=50  --report_every=100000  --temperature=0.000001``

* Try with different mutation operator, `AdditiveSingleUniform`, which just adds a number to the probability of a random lexe for a given meme  
``python3 run.py --num_steps=10000000 --num_agents=30 --num_memes=20 --num_lexes=20  --report_every=100000  --temperature=0.000001 --mutator_class=AdditiveSingleUniform``  
Interestingly, this mutation operator shows convergence to perfect comprehension. See `mutators.py` for more mutation operators


