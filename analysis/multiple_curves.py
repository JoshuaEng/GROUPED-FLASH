import matplotlib.pyplot as plt
import numpy as np
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("dataset")
parser.add_argument('-save', action='store_true')
parser.add_argument("topk")
args = parser.parse_args()
save = args.save
dataset = args.dataset
compare_by = args.topk

num_queries = 10000

def get_pareto(record):
	record.sort()
	record = record[::-1]
	result = [record[0]]
	for i in range(1, len(record)):
		if result[-1][1] < record[i][1]:
			result.append(record[i])
	return result

def get_data(file_name, max_time):
	record = []
	with open(file_name, "r") as f:
		while True:
			line = f.readline()
			if line == "":
				break
			if line.startswith("STAT"):
				while True:
					line = f.readline()
					if line.startswith("T@k"):
						break
					if line.startswith("Queried"):
						time = float(line.strip().split(" ")[-1][:-3])
						if time > num_queries * max_time:
							break
					if line.startswith(f"R{compare_by}@") and not line.startswith(f"R{compare_by}@k"):
						r = float(line.split()[2])
						p = r * int(compare_by) / int(line.split()[0][2 + len(compare_by):])
						record.append((r, p))
					if line == "":
						break
	return get_pareto(record)


titlefontsize = 22
axisfontsize = 18
labelfontsize = 12

methods = ["flinng", "flash"]
colors = ["#264478","#F08406"]
files = [f"../my_{dataset}_cached3.txt", f"../their_{dataset}_cached3.txt"]

# markers = ["s","^","+"]
markers = ["s","^"]
times = ["2","20","200"]
# linestyles = ["--", "-.", ":"]
linestyles = ["--", "-."]

if dataset == "yfcc":
	start = 1
	end = 3
else:
	start = 0
	end = 2

# for more linestyles see here: https://matplotlib.org/3.1.1/gallery/lines_bars_and_markers/linestyles.html



for c, method, file_name in zip(colors, methods, files):
	for ls, mark, t in list(zip(linestyles, markers, times[start:end])): 
		D = get_data(file_name, int(t))
		plt.plot([x for (x, _) in D], [y for (_, y) in D], color = c, linestyle = ls, marker = mark, label = method.upper()+" ("+t+"ms)", alpha = 0.8)

plt.legend(fontsize = labelfontsize)
plt.title(f"{dataset}: Top-{compare_by} Recall".title(),fontsize = titlefontsize)
plt.xlabel("Recall",fontsize = axisfontsize)
plt.ylabel("Precision",fontsize = axisfontsize)

if save:
	plt.savefig(f"/home/jae4/FLINNG-Results/precall-mult/{dataset}-{compare_by}.png", bbox_inches='tight')
else:
	plt.show()


