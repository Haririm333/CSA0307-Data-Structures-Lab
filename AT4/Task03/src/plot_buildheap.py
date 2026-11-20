import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

def plot_task3():
    df = pd.read_csv('Task03/output/buildheap_bench.csv')
    
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5.5))
    
    n_vals = df['N']
    
    # 1. Total Comparisons vs N
    ax1.plot(n_vals, df['BuildHeap_Comps'], 'o-', color='#2ca02c', linewidth=2.2, label='Bottom-Up BuildHeap (Linear ~2N)')
    ax1.plot(n_vals, df['Successive_Comps'], 's--', color='#d62728', linewidth=2.2, label='Successive Inserts (~N log2 N)')
    
    # Ideal theoretical curves
    ax1.plot(n_vals, 2 * n_vals, ':', color='#2ca02c', alpha=0.7, label='Theory: 2N')
    ax1.plot(n_vals, n_vals * np.log2(n_vals), ':', color='#d62728', alpha=0.7, label='Theory: N log2 N')
    
    ax1.set_xscale('log')
    ax1.set_yscale('log')
    ax1.set_xlabel('Array Size N (Log Scale)', fontsize=11)
    ax1.set_ylabel('Total Comparisons (Log Scale)', fontsize=11)
    ax1.set_title('Heap Construction: Total Comparisons Comparison', fontsize=12, fontweight='bold')
    ax1.grid(True, linestyle='--', alpha=0.6, which='both')
    ax1.legend(fontsize=9)
    
    # 2. Ratio of Comparisons / N
    ax2.plot(n_vals, df['BuildHeap_RatioCompsN'], 'o-', color='#2ca02c', linewidth=2.2, label='BuildHeap: Comps / N (Constant ~ 2.0)')
    ax2.plot(n_vals, df['Successive_RatioCompsN'], 's--', color='#d62728', linewidth=2.2, label='Successive: Comps / N (Grows as log2 N)')
    
    ax2.set_xscale('log')
    ax2.set_xlabel('Array Size N (Log Scale)', fontsize=11)
    ax2.set_ylabel('Comparisons / N', fontsize=11)
    ax2.set_title('Asymptotic Efficiency: Comparisons per Item', fontsize=12, fontweight='bold')
    ax2.grid(True, linestyle='--', alpha=0.6, which='both')
    ax2.legend(fontsize=9)
    
    plt.tight_layout()
    plt.savefig('Task03/output/buildheap_bench.png', dpi=300)
    plt.close()
    print("Graph saved: Task03/output/buildheap_bench.png")

if __name__ == '__main__':
    plot_task3()
