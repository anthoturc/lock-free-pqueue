import matplotlib.pyplot as plt

dataf = "run-11813.dat"
x = []  # num threads
y1 = [] # stl pq times
y2 = [] # lock-free pq times

with open(dataf, 'r') as f:
	lines = f.read().splitlines()

	for line in lines:
		data = line.split()
		x.append(int(data[0]))
		y1.append(float(data[1]))
		y2.append(float(data[2]))

plt.yscale('log',basey=2) 
plt.xlabel("Number of Threads")
plt.ylabel("log(Time (s))")
plt.plot(x, y1, marker='x', label="STL Priority Queue (coarse grain locking)")
plt.plot(x, y2, marker='D', label="Lock-free Priority Queue")
plt.legend()
plt.savefig("results.png")



