import pandas as pd
import numpy as np
import os
import matplotlib.pyplot as plt
import scipy.stats

COL_NAMES = {'GrammarVar':'GrammarStdDev'}
scalingtypes = {
    'agents': ('num_agents', '# agents'),
    'memes' : ('num_memes' , '# memes'),
    'lexes' : ('num_lexes' , '# lexes'),
    'lexesmemes': ('num_memes', '# memes,lexes'),
    'lexesmemesagents': ('num_agents', '# memes,lexes,agents')
}


def load_data_dir(DIR):
    data = []
    error_files = []
    for fname in sorted(os.listdir(DIR)):
        if fname.startswith('run'):
            ffname = DIR+'/'+fname
            with open(ffname) as f:
                for l in f.readlines():
                    if l[0]=='#' and 'num_memes' in l:
                        doRead = True
                        break
            if doRead:
                try:
                    d = dict(e.split('=', 1) for e in l[1:].strip().split(', '))
                    d['filename'] = fname
                    d['full_filename'] = ffname
                    df = pd.read_csv(ffname, comment='#', sep=' ', na_values='-')
                    if not len(df):
                        raise

                    if df.Step.value_counts().sort_values().iloc[-1] > 1:
                        raise Exception
                    if df.Step.dtype == '|O':
                        raise Exception

                    data.append((d,df))
                except:
                    error_files.append(ffname)
    return data, error_files


def plot_scatter(xs, ys, loglog=False, regression_line=False, do_plot=True):
    if do_plot:
        plt.scatter(xs, ys, s=100, marker='x')
        if loglog:
            plt.loglog()
        
    if regression_line and len(xs) > 1:
        xs, ys = map(np.array, [xs, ys])
        if loglog:
            xs, ys = map(np.log, [xs, ys])
        slope, intercept = scipy.stats.linregress(xs, ys)[0:2]
        if do_plot:
            preds = intercept + slope * xs
            plotxs, plotpreds = xs, preds
            if loglog:
                plotxs, plotpreds = map(np.exp, [plotxs, plotpreds])

            plt.plot(plotxs, plotpreds, '--', label='$\\alpha=%0.2f$'%slope)
            plt.legend()
        
        return slope, intercept
        
    else:
        return None
    
    
def get_scaling_vals(data, COL, m, pCol, COMPREHENSION_CUTOFF=0.25):
    colvals  = set(float(cfg[COL])      for cfg, _ in data)
    plot_dfs = [ (cfg, df) 
                 for v in sorted(colvals) for (cfg, df) in data
                 if cfg['mutator_class'] == m and float(cfg[COL]) == v and len(df) > 1]
    xs, ys = [],[]
    for cfg, df in plot_dfs:
        steps, vals = df.Step, df[pCol]
        if pCol == 'AcceptanceRate':
            ylabel = 'Steps until ~min Acceptance'
            ixs = np.flatnonzero(vals <= 1.02*vals.min())
            if len(ixs) and ixs[0]>1:
                xs.append(float(cfg[COL]))
                ys.append(steps[ixs[0]])

        elif pCol == 'Comprehension':
            ylabel = 'Steps to %d%% Comprehension' % int(COMPREHENSION_CUTOFF*100)
            use_ix = None
            ixs = np.flatnonzero(df[pCol] >= COMPREHENSION_CUTOFF)
            if len(ixs) and ixs[0]>1:
                xs.append(float(cfg[COL]))
                ys.append(df.Step.iloc[ixs[0]])

        else:
            ylabel = 'max %s reached'%COL_NAMES.get(pCol, pCol)
            y = float(vals.max())
            if y != 0:
                xs.append(float(cfg[COL]))
                ys.append(y)
    return plot_dfs, xs, ys, ylabel


def get_data(basedir, scalingtype):
    COL, desc = scalingtypes[scalingtype]
    data, error_files = load_data_dir(basedir+'/'+scalingtype)
    
    mutators = set(cfg['mutator_class'] for cfg, _ in data)

    ALLOWED_MUTATORS = set(['ArcsSingleClip', 'ProbabilityMover', 'ProbitSingleUniform']) # ,'AdditiveSingleClipExppGaussian'])
    if ALLOWED_MUTATORS is not None:
        mutators = mutators.intersection(ALLOWED_MUTATORS)

    print("Loaded %d files" % len(data))
    if len(error_files):
        print("Errors processing these files:")
        for ffname in error_files:
            print("  %s" % ffname)

    return COL, desc, data, mutators

