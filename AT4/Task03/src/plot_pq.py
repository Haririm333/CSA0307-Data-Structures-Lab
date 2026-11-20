import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import pandas as pd

def plot_task3_pq():
    df = pd.read_csv('Task03/output/pq_comparison.csv')
    
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5.5))
    
    n_vals = df['N']
    
    # 1. Total Time: Heap vs Sorted List
    ax1.plot(n_vals, df['Heap_Total_Ms'], 'o-', color='#1f77b4', linewidth=2.2, label='Binary Heap (O(N log N))')
    ax1.plot(n_vals, df['List_Total_Ms'], 's-', color='#d62728', linewidth=2.2, label='Sorted List PQ (O(N^2))')
    
    ax1.set_xlabel('Queue Size N (Operations: N push + N pop)', fontsize=11)
    ax1.set_ylabel('Total Execution Time (ms)', fontsize=11)
    ax1.set_title('Execution Time: Heap vs Sorted Linked List', fontsize=12, fontweight='bold')
    ax1.grid(True, linestyle='--', alpha=0.6)
    ax1.legend(fontsize=10)
    
    # 2. Speedup of Heap over Sorted List
    ax2.bar(n_vals.astype(str), df['Speedup_HeapVsList'], color='#2ca02c', width=0.5)
    ax2.set_xlabel('Queue Size N', fontsize=11)
    ax2.set_ylabel('Speedup Factor (List Time / Heap Time)', fontsize=11)
    ax2.set_title('Heap Performance Advantage (Speedup Multiplier)', fontsize=12, fontweight='bold')
    ax2.grid(True, linestyle='--', alpha=0.5, axis='y')
    
    for i, v in enumerate(df['Speedup_HeapVsList']):
        ax2.annotate(f'{v:.0f}x', xy=(i, v), xytext=(0, 3), textcoords="offset points",
                     ha='center', va='bottom', fontweight='bold', fontsize=9)
        
    plt.tight_layout()
    plt.savefig('Task03/output/pq_comparison.png', dpi=300)
    plt.close()
    print("Graph saved: Task03/output/pq_comparison.png")

if __name__ == '__main__':
    plot_task3_pq()
