import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

asp_file = "benchmark_asp.csv"
std_file = "benchmark_std.csv"

missing = []
if not os.path.exists(asp_file):
    missing.append(asp_file)
if not os.path.exists(std_file):
    missing.append(std_file)

if missing:
    print("Missing required CSV files:")
    for f in missing:
        print(f"  -", f)
    print("\nTo generate them, run the following from your build directory:")
    print("  ./benchmark_asp")
    print("  ./benchmark_std")
else:
    asp = pd.read_csv(asp_file)
    std = pd.read_csv(std_file)

    asp["type"] = "actually_smart_pointer"
    std["type"] = "std::shared_ptr"

    combined_time = pd.concat([asp[["iterations", "time_ms", "type"]],
                               std[["iterations", "time_ms", "type"]]])
    # combined_mem = pd.concat([asp[["iterations", "memory_kb", "type"]],
    #                           std[["iterations", "memory_kb", "type"]]])

    plt.figure(figsize=(10, 6))
    sns.lineplot(data=combined_time, x="iterations", y="time_ms", hue="type", marker="o")
    plt.title("Execution Time Comparison")
    plt.xlabel("Iterations")
    plt.ylabel("Time (ms)")
    plt.xscale("log")
    plt.grid(True)
    plt.tight_layout()
    plt.savefig("benchmark_time.png")
    # plt.show()

    # plt.figure(figsize=(10, 6))
    # sns.lineplot(data=combined_mem, x="iterations", y="memory_kb", hue="type", marker="o")
    # plt.title("Memory Usage Comparison")
    # plt.xlabel("Iterations")
    # plt.ylabel("Memory (KB)")
    # plt.xscale("log")
    # plt.grid(True)
    # plt.tight_layout()
    # plt.show()

