import matplotlib
import numpy as np
import argparse
from get_data import get_all_data_colored

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

if save:
	matplotlib.use('agg')
import matplotlib.pyplot as plt

def get_pareto(record):
	record.sort(key = lambda x: x[2])
	record = record[::-1]
	result = [record[0]]
	for i in range(1, len(record)):
		if result[-1][3] < record[i][3]:
			result.append(record[i])
	return result
	
titlefontsize = 22
axisfontsize = 18
labelfontsize = 12
markers = ["s"]
times = [20]
linestyles = ["--"]

all_data = get_all_data_colored(dataset)
for method, data, c in all_data:
	for ls, mark, t in list(zip(linestyles, markers, times)): 
		try:
			filtered = get_pareto([d for d in data if d[1] < t and d[0].startswith("R" + compare_by + "@")])
			plt.plot([d[2] for d in filtered], [d[3] for d in filtered], color = c, linestyle = ls, marker = mark, label = method.upper()+" ("+str(t)+"ms)", alpha = 0.8)
		except:
			print(method, "failed in recall precision on", dataset)
			pass

plt.legend(fontsize = labelfontsize)
plt.title(f"{dataset}: Top-{compare_by} Recall".title(),fontsize = titlefontsize)
plt.xlabel("Recall",fontsize = axisfontsize)
plt.ylabel("Precision",fontsize = axisfontsize)

if save:
	plt.savefig(f"{dataset}-{compare_by}.png", bbox_inches='tight')
else:
	plt.show()


