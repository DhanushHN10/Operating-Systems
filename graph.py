import matplotlib.pyplot as plt

# Data from your input
data_local = [
    ((64, 4), [67591, 81860, 72967, 58547, 280965]),
    ((64, 128), [13420, 18046, 23542, 9930, 64938]),
    ((128, 1024), [446, 8143, 13368, 603, 22560]),
    ((1024, 128), [658, 1195, 10590, 2810, 15253]),
    ((4096, 1024), [51, 138, 5384, 54, 5627]),
    ((16384, 4096), [35, 38, 267, 27, 367]),
    ((65536, 16384), [21, 14, 100, 16, 151]),
    ((65536, 65536), [21, 14, 100, 16, 151])
]

data_global = [
    ((64, 4), [73412, 78578, 77159, 68602, 297751]),
    ((64, 128), [15093, 17968, 24043, 11481, 68585]),
    ((128, 1024), [1357, 8112, 13063, 1928, 24460]),
    ((1024, 128), [2834, 2233, 9955, 2555, 17577]),
    ((4096, 1024), [78, 140, 1498, 92, 1808]),
    ((16384, 4096), [35, 38, 267, 27, 367]),
    ((65536, 16384), [21, 14, 100, 16, 151]),
    ((65536, 65536), [21, 14, 100, 16, 151])
]

def plot_graph(data, title):
    x_labels = [f"{ps},{fr}" for (ps, fr), _ in data]
    processes = ["P0", "P1", "P2", "P3", "Total"]
    colors = ["blue", "orange", "green", "red", "black"]

    plt.figure(figsize=(12, 6))
    for i in range(5):
        y_values = [entry[1][i] for entry in data]
        plt.plot(x_labels, y_values, marker='o', color=colors[i], label=processes[i], linewidth=2 if i==4 else 1.5)
    
    plt.title(title)
    plt.xlabel("Page Size, Frames")
    plt.ylabel("Page Faults")
    plt.legend()
    plt.grid(True)
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.show()

# Plot LOCAL allocation
plot_graph(data_local, "Page Faults vs Page Size & Frames (LOCAL Allocation)")

# Plot GLOBAL allocation
plot_graph(data_global, "Page Faults vs Page Size & Frames (GLOBAL Allocation)")
