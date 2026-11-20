import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

def plot_task1():
    # Load comparison summary
    df_summary = pd.read_csv('Task01/output/hash_comparison.csv')
    
    fig, axes = plt.subplots(1, 2, figsize=(15, 6))
    
    # Chart 1: Max Chain Length vs Average Chain Length
    methods = [m.split('(')[0].strip() for m in df_summary['Method']]
    x = np.arange(len(methods))
    width = 0.35
    
    # We plot Max Chain on log scale because Bad Hash has ~80,000 while others have ~130!
    ax1 = axes[0]
    bars1 = ax1.bar(x - width/2, df_summary['MaxChain'], width, label='Max Chain Length', color='#d9534f')
    bars2 = ax1.bar(x + width/2, df_summary['AvgChainNonEmpty'], width, label='Avg Chain (Non-Empty)', color='#0275d8')
    
    ax1.set_ylabel('Chain Length (Log Scale)')
    ax1.set_yscale('log')
    ax1.set_title('Task 1: Chain Length Comparison (Log Scale)')
    ax1.set_xticks(x)
    ax1.set_xticklabels(methods, rotation=25, ha='right', fontsize=9)
    ax1.legend()
    ax1.grid(True, linestyle='--', alpha=0.5, which='both')
    
    # Add values on top of bars
    for bar in bars1:
        height = bar.get_height()
        ax1.annotate(f'{int(height)}',
                    xy=(bar.get_x() + bar.get_width() / 2, height),
                    xytext=(0, 3), textcoords="offset points",
                    ha='center', va='bottom', fontsize=8, fontweight='bold')
    for bar in bars2:
        height = bar.get_height()
        ax1.annotate(f'{height:.1f}',
                    xy=(bar.get_x() + bar.get_width() / 2, height),
                    xytext=(0, 3), textcoords="offset points",
                    ha='center', va='bottom', fontsize=8)

    # Chart 2: Non-Empty vs Empty Buckets
    ax2 = axes[1]
    b1 = ax2.bar(x, df_summary['NonEmptyBuckets'], width*1.2, label='Non-Empty Buckets', color='#5cb85c')
    b2 = ax2.bar(x, df_summary['EmptyBuckets'], width*1.2, bottom=df_summary['NonEmptyBuckets'], label='Empty Buckets', color='#f0ad4e')
    
    ax2.set_ylabel('Number of Buckets')
    ax2.set_title('Bucket Utilization (Active vs Wasted Buckets)')
    ax2.set_xticks(x)
    ax2.set_xticklabels(methods, rotation=25, ha='right', fontsize=9)
    ax2.legend()
    ax2.grid(True, linestyle='--', alpha=0.5)
    
    for i, total in enumerate(df_summary['TableSize']):
        active = df_summary['NonEmptyBuckets'][i]
        pct = (active / total) * 100
        ax2.annotate(f'{pct:.1f}% used',
                    xy=(x[i], active / 2),
                    ha='center', va='center', color='white', fontweight='bold', fontsize=9)

    plt.tight_layout()
    plt.savefig('Task01/output/chain_distribution.png', dpi=300)
    plt.close()
    print("Graph generated: Task01/output/chain_distribution.png")

if __name__ == '__main__':
    plot_task1()
