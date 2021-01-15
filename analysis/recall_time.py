import argparse
import matplotlib.pyplot as plt
import math

# Instantiate the parser
parser = argparse.ArgumentParser(description='Compare FLASH and FLINNG')
parser = argparse.ArgumentParser()
parser.add_argument("dataset")
parser.add_argument("directory")
parser.add_argument('-save', action='store_true')
parser.add_argument("compare_by")

args = parser.parse_args()
directory = args.directory
save = args.save
dataset = args.dataset
compare_by = args.compare_by

num_queries = 10000

def get_pareto(record):
	record.sort()
	result = [record[0]]
	for i in range(1, len(record)):
		if result[-1][1] < record[i][1]:
			result.append(record[i])
	return result

def read_hnsw(file_name):
	record = []
	with open(file_name, "r") as f:
		while True:
			line = f.readline()
			if not line:
				break
			if line.startswith(compare_by):
				values = line.split()[1].split(",")
				record.append((float(values[0]) * num_queries, float(values[1])))
	return get_pareto(record)

def read_flash(file_name):
	record = []
	with open(file_name, "r") as f:
		while True:
			line = f.readline()
			if line == "":
				break
			if line.startswith("STAT"):
				while True:
					line = f.readline()
					if line.startswith("Queried"):
						time = float(line.strip().split(" ")[-1][:-3])
					if line.startswith(compare_by):
						record.append((time, float(line.split()[2])))
						break
					if line == "":
						break
	return get_pareto(record)

def get_data(file_name):   
	if "hnsw" in file_name:
		return read_hnsw(file_name)
	else:
		return read_flash(file_name)
	
titlefontsize = 22
axisfontsize = 18
labelfontsize = 12

methods = ["flinng", "flash", "hnsw"]
colors = ["#264478","#F08406", "#232f3e"]

mark = "s"
ls = "--"

for c, method in zip(colors, methods):
	try:
		file_name = directory + f"/{method}_{dataset}.txt"
		D = get_data(file_name)
		plt.plot([y for (_, y) in D], [math.log10(num_queries / x * 1000) for (x, _) in D if x != 0], color = c, linestyle = ls, marker = mark, label = method.upper(), alpha = 0.8)
	except:
		pass
		
plt.legend(loc='upper right', fontsize=labelfontsize)
plt.xlabel(compare_by, fontsize=axisfontsize)
plt.ylabel('Queries per second (log 10)', fontsize=axisfontsize)
plt.title((dataset).title(), fontsize=titlefontsize)
if save:
	valid = compare_by.replace("@", "at")
	plt.savefig(f"{dataset}-{valid}.png", bbox_inches='tight')
else:
	plt.show()
