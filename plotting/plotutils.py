import pandas as pd
import numpy as np
import os
import matplotlib.pyplot as plt
import scipy.stats

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


def plot_scatter(xs, ys, loglog=False, regression_line=False):
    plt.scatter(xs, ys, s=100, marker='x')
    if loglog:
        plt.loglog()
        
    if regression_line and len(xs) > 1:
        xs, ys = map(np.array, [xs, ys])
        if loglog:
            xs, ys = map(np.log, [xs, ys])
        slope, intercept = scipy.stats.linregress(xs, ys)[0:2]
        preds = intercept + slope * xs
        
        plotxs, plotpreds = xs, preds
        if loglog:
            plotxs, plotpreds = map(np.exp, [plotxs, plotpreds])
            
        plt.plot(plotxs, plotpreds, '--', label='Slope=%0.2f'%slope)
        plt.legend()
        
        return slope, intercept
        
    else:
        return None