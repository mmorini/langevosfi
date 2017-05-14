#!/usr/bin/python

# Suck coherence measures from the output of the unified code

import numpy as np, re, matplotlib.pyplot as plt, argparse

parser = argparse.ArgumentParser(description='Plot coherence measures')
parser.add_argument('infile', help='Input filename')
args = parser.parse_args()


# Read a sequence of lines of the form M<n> f(M<n>) phi[L0|M<n>] phi[L0|M<n>] ...
# Returns two numpy arrays, one of f[M<n>], one of phi[M<n>,L<m>]
def read_grammar(instream, memes, lexemes):
    freqs = np.empty(memes)
    gram = np.empty((memes,lexemes))
    # This is "pythonic" apparently. Yuck.
    m=0
    for row in instream:
        row = row.split()
        if row[0] != 'M'+str(m):
            raise RuntimeException('Expected M{m} in input'.format(m=m))
        freqs[m] = np.float_(row[1])
        gram[m] = np.array( map(np.float_, row[2:]) )
        m+=1
        if m==memes: return freqs, gram


times = []
scohere = []
ocohere = []

with file(args.infile) as fh:
    
    # Obtain key parameters
    parms = {}
    for row in fh:
        kvs = re.findall(r'(\w+)\s*=\s*([\w.]+)',row)
        if not kvs: break
        for k,v in kvs:
            parms[k] = v
            
    print parms
                
    memes, lexemes, agents = int(parms['nummemes']), int(parms['numlexes']), int(parms['numagents'])
    time = 0
    deltat, maxt = int(parms['printinterval']), int(parms['outer'])
    
    freqs = np.empty( (agents, memes) )
    grammars = np.empty( (agents, memes, lexemes) )
    
    # This is useful for extracting offdiagonal elements of a matrix
    ai, aj = np.indices((agents,agents))
    offdiag = np.where(ai!=aj)
    
    agent = 0

    # Slurp in each grammar    
    for row in fh:

        if re.search('L\d+', row):
            freqs[agent], grammars[agent] = read_grammar(fh, memes, lexemes)
            agent += 1
            if agent == agents:
                        
                # Process a batch of agents
                # Get normalised production and inference frequencies
                produce = grammars / np.sum(grammars, axis=2)[:,:,np.newaxis]
                infer = grammars / np.sum(grammars, axis=1)[:,np.newaxis,:] 
                                
                # This tensor product gives us a matrix coherence[i,j] between agent i,j
                # Specifically: coherenence[i,j] = sum_m sum_l freqs[i,m] * produce[i,m,l] * infer[j,m,l] 
                coherence = np.tensordot( freqs[...,np.newaxis]*produce, infer, axes=( (1,2), (1,2) ) )
                
                times += [np.float_(time) / np.float_(memes*lexemes)]
                scohere += [np.mean(np.diagonal(coherence))] # Mean coherence of a speaker with themselves
                ocohere += [np.mean(coherence[offdiag])] # Mean coherence of speakers with all others
                
                # Advance the clock
                time += deltat
                if time > maxt:
                    time = maxt
                    
                # Reset the agent
                agent = 0

print len(times), times[-1]

plt.plot(times, scohere, label='self')
plt.plot(times, ocohere, label='other')
plt.xlabel('Time (sweeps)')
plt.ylabel('Coherence')
plt.legend()
plt.show()
