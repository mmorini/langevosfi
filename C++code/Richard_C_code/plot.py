import numpy as np, matplotlib.pyplot as plt, re, argparse

# Configuration
parser = argparse.ArgumentParser(description='Plot coordination game statistics')

parser.add_argument('infile', help='Data file')
parser.add_argument('--plot', '-p', choices=['coherence', 'grammars', 'pseudos'], default='coherence', help='What to plot [coherence]')
parser.add_argument('--title', '-T', help='Figure title (optional)')
parser.add_argument('--fig', '-F', help='Figure file [display]')

args = parser.parse_args()

# At iterator that yields exactly k lines from a file
def read_block(fh,k):
    for i in xrange(k):
        yield fh.next()

ts = []
sc = []
oc = []

grs = []
pgs = []

# Read data from the file
with file(args.infile) as fh:
    try:
        while True:        
            ln = fh.next().strip()
            if not ln: continue # Strip blank lines
            # Work out what's coming
            what, kvs = ln.split(None,1)
            kvs = dict(re.findall('(\w+)=(\S+)', kvs))
            if what == 'coherence':
                nagents = int(kvs['agents'])
                round = int(kvs['round'])
                ch = np.loadtxt(read_block(fh, nagents))
                ts += [round]
                sc += [np.mean(np.diag(ch))]
                oc += [np.mean( [ch[i,j] for i in xrange(nagents) for j in xrange(nagents) if i != j ] ) ]
            elif what == 'grammar':
                # Read grammar - just assume we only get one set, in the right order, for now
                gr = np.loadtxt(read_block(fh, int(kvs['meanings'])))
                grs += [gr]
            elif what == 'pseudo':
                # Read pseudo-grammar - just assume we only get one set, in the right order, for now
                pg = np.loadtxt(read_block(fh, int(kvs['meanings'])))
                pgs += [pg]
    except StopIteration:
        # This is only here so we don't get an error when we reach EOF
        pass

plt.figure()

if args.plot == 'coherence':
    plt.plot(ts,sc,label='self')
    plt.plot(ts,oc,label='other')
    plt.xlabel('time')
    plt.ylabel('coherence')
    plt.ylim((0.0,1.0))
    plt.legend()
elif args.plot == 'grammars':
    rows = int(np.sqrt(len(grs)))
    cols = len(grs)/rows
    if rows*cols < len(grs): cols+=1
    for n, gr in enumerate(grs):
        plt.subplot(rows, cols, n+1)
        plt.pcolor(gr, vmin=0.0, vmax=1.0, cmap='Greys')
elif args.plot == 'pseudos':
    rows = int(np.sqrt(len(pgs)))
    cols = len(pgs)/rows
    if rows*cols < len(pgs): cols+=1
    for n, pg in enumerate(pgs):
        plt.subplot(rows, cols, n+1)
        plt.pcolor(pg, vmin=0.0, vmax=1.0, cmap='Greys')

if args.title:
    plt.title(args.title)

if args.fig is None:
    plt.show()
else:
    plt.savefig(args.fig)

