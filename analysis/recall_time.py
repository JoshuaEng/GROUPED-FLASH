import argparse

# Instantiate the parser
parser = argparse.ArgumentParser(description='Compare FLASH and FLINNG')
parser.add_argument("dataset")
parser.add_argument("compare_by")
parser.add_argument('-save', action='store_true')
args = parser.parse_args()
save = args.save
compare_by = args.compare_by
dataset = args.dataset

files = [f"../my_{dataset}_cached3.txt", f"../their_{dataset}_cached3.txt"]
num_queries = 10000

def get_pareto(record):
        record.sort()
        result = [record[0]]
        for i in range(1, len(record)):
                if result[-1][1] < record[i][1]:
                        result.append(record[i])
        return result

def parse_file(file_name, line_hint):   
        record = []
        with open(file_name, "r") as f:
                while True:
                        line = f.readline()
                        if line == "":
                                break
                        if line.split(" ")[0] == line_hint:
                                saved = line
                                while True:
                                        line = f.readline()
                                        if line.startswith("Queried"):
                                                time = float(line.strip().split(" ")[-1][:-3])
                                        if line.startswith(compare_by):
                                                r = float(line.split()[2])
                                                split = saved.strip().split()[1:]
                                                record.append((time, r, saved.strip()))
                                                break
                                        if line == "":
                                                break
        return get_pareto(record)

group_record = parse_file(files[0], "STATS_GROUPS:")
normal_record = parse_file(files[1], "STATS_NORMAL:")
        
colors = ["#264478","#F08406"]

# markers = ["s","^","+"]
# times = ["2","20","200"]
linestyles = ["--", "-.", ":"]
titlefontsize = 22
axisfontsize = 18
labelfontsize = 12

import matplotlib.pyplot as plt
import math
plt.plot([y for (x,y, z) in group_record], [math.log10(10000 / x * 1000) for (x,y, z) in group_record], label=f'FLINNG', color=colors[0], linestyle = linestyles[0], marker = "s" )
plt.plot([y for (x,y, z) in normal_record if x != 0], [math.log10(10000 / x * 1000) for (x,y, z) in normal_record if x != 0], label='FLASH', color=colors[1], linestyle = linestyles[0], marker = "s")
plt.legend(loc='upper right', fontsize=labelfontsize)
plt.xlabel(compare_by, fontsize=axisfontsize)
plt.ylabel('Queries per second (log 10)', fontsize=axisfontsize)
plt.title((dataset).title(), fontsize=titlefontsize)
if save:
        valid = compare_by.replace("@", "at")
        plt.savefig(f"/home/jae4/FLINNG-Results/trecall/{dataset}-{valid}.png", bbox_inches='tight')
else:
        plt.show()
