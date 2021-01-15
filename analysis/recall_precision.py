import matplotlib.pyplot as plt
import numpy as np
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("dataset")
parser.add_argument("directory")
parser.add_argument('-save', action='store_true')
parser.add_argument("topk")
args = parser.parse_args()
directory = args.directory
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

def read_hnsw(file_name, max_time):
	record = []
	with open(file_name, "r") as f:
		while True:
			line = f.readline()
			if not line:
				break
			if line.startswith("R" + compare_by + "@"):
				values = line.split()[1].split(",")
				if float(values[0]) <= max_time:
					record.append((float(values[1]), float(values[2])))
	return get_pareto(record)

def read_flash(file_name, max_time):
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


def get_data(file_name, max_time):
	if "hnsw" in file_name:
		return read_hnsw(file_name, max_time)
	else:
		return read_flash(file_name, max_time)
	
titlefontsize = 22
axisfontsize = 18
labelfontsize = 12

methods = ["flinng", "flash", "hnsw"]
colors = ["#264478","#F08406", "#232f3e"]

markers = ["s"]
times = ["20"]
linestyles = ["--"]

# for more linestyles see here: https://matplotlib.org/3.1.1/gallery/lines_bars_and_markers/linestyles.html


for c, method in zip(colors, methods):
	file_name = directory + f"/{method}_{dataset}.txt"
	for ls, mark, t in list(zip(linestyles, markers, times)): 
		try:
			D = get_data(file_name, int(t))
			plt.plot([x for (x, _) in D], [y for (_, y) in D], color = c, linestyle = ls, marker = mark, label = method.upper()+" ("+t+"ms)", alpha = 0.8)
		except:
			pass

plt.legend(fontsize = labelfontsize)
plt.title(f"{dataset}: Top-{compare_by} Recall".title(),fontsize = titlefontsize)
plt.xlabel("Recall",fontsize = axisfontsize)
plt.ylabel("Precision",fontsize = axisfontsize)

if save:
	plt.savefig(f"{dataset}-{compare_by}.png", bbox_inches='tight')
else:
	plt.show()


