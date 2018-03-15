import pandas as pd
import matplotlib.pyplot as plt
def plot_stats(stats):
    df = pd.DataFrame.from_dict(stats)
    fig, axes = plt.subplots(2, 2, figsize=(8, 8));
    df.plot('Step','Comprehension', ax=axes[0,0])
    df.plot('Step','AcceptanceRate', ax=axes[0,1])
    df.plot('Step','GrammarVar', ax=axes[1,0])
    df.plot('Step','MeanGrammarDrift', ax=axes[1,1])
    axes[0,0].set_ylim([0,1])
    axes[0,1].set_ylim([0,1])
    axes[1,0].set_ylim([0,axes[1,0].get_ylim()[1]])
    axes[1,1].set_ylim([0,axes[1,1].get_ylim()[1]])
    plt.tight_layout()
