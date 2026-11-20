import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import pandas as pd

def plot_speedup():
    df = pd.read_csv('Task05/output/parallel_speedup.csv')
    
    threads = df['Threads']
    speedup = df['Speedup']
    
    plt.figure(figsize=(9, 6))
    
    # Measured Speedup
    plt.plot(threads, speedup, 'o-', color='#1f77b4', linewidth=2.5, markersize=8, label='Measured Speedup (OpenMP Tasks)')
    
    # Ideal Linear Speedup
    plt.plot(threads, threads, '--', color='#2ca02c', linewidth=2.0, alpha=0.8, label='Ideal Linear Speedup (T1 / p)')
    
    plt.title('Parallel Merge Sort: Speedup vs Thread Count (N = 10,000,000)', fontsize=13, fontweight='bold')
    plt.xlabel('Number of Threads (p)', fontsize=11)
    plt.ylabel('Speedup Factor (T1 / Tp)', fontsize=11)
    plt.xticks(threads)
    plt.grid(True, linestyle='--', alpha=0.6)
    plt.legend(fontsize=11)
    
    for i, (t, s, eff) in enumerate(zip(threads, speedup, df['EfficiencyPercent'])):
        plt.annotate(f'{s:.2f}x\n({eff:.1f}%)',
                     xy=(t, s), xytext=(0, 10), textcoords="offset points",
                     ha='center', va='bottom', fontsize=9, fontweight='bold')
                     
    plt.tight_layout()
    plt.savefig('Task05/output/parallel_speedup.png', dpi=300)
    plt.close()
    print("Graph saved: Task05/output/parallel_speedup.png")

if __name__ == '__main__':
    plot_speedup()
