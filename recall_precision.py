# group_record2 = []
group_record = {}
normal_record = {}

import argparse

# Instantiate the parser
parser = argparse.ArgumentParser(description='Compare FLASH and FLINNG')
parser.add_argument("compare_by")
parser.add_argument("their_file")
parser.add_argument("my_file")
parser.add_argument("max_time")
parser.add_argument('-save', action='store_true')
args = parser.parse_args()
save = args.save
compare_by = args.compare_by
maximum_time_per_query = int(args.max_time)

best_r = 0
with open(args.my_file, "r") as f:
        while True:
                line = f.readline()
                if line == "":
                        break
                if line.split(" ")[0] == "STATS_GROUPS:":
                        if int(line.split(" ")[1]) > 5:
                                continue
                        while True:
                                line = f.readline()
                                if line.startswith("Queried"):
                                        time = float(line.split(" ")[-2][:-3])
                                        if time > 10000 * maximum_time_per_query:
                                                break
                                if line.startswith(f"R{compare_by}@") and not line.startswith(f"R{compare_by}@k"):
                                        r = float(line.split()[2])
                                        p = int(compare_by) / int(line.split()[0][2 + len(compare_by):])
                                        if p not in group_record:
                                                group_record[p] = 0
                                        if r > group_record[p]:
                                                group_record[p] = r
                                if line == "":
                                        break
with open(args.their_file, "r") as f:
        while True:
                line = f.readline()
                if line == "":
                        break
                if line.split(" ")[0] == "STATS_NORMAL:":
                        saved = line
                        while True:
                                line = f.readline()
                                if line.startswith("Queried"):
                                        time = float(line.strip().split(" ")[-1][:-3])
                                if line.startswith(f"R{compare_by}@") and not line.startswith(f"R{compare_by}@k"):
                                        r = float(line.split()[2])
                                        p = int(compare_by) / int(line.split()[0][2 + len(compare_by):])
                                        if p not in normal_record:
                                                normal_record[p] = 0
                                        if r > normal_record[p]:
                                                normal_record[p] = r
                                if line == "":
                                        break

import matplotlib.pyplot as plt
import math
plt.scatter(group_record.values(), group_record.keys(), marker='x', label=f'FLINNG' )
plt.scatter(normal_record.values(), normal_record.keys(), marker='s', label=f'FLASH' )
dataset = args.my_file.split("_")[1]
plt.legend(loc='upper right')
plt.xlabel("Recall")
plt.ylabel('Precison')
extra = "" if compare_by=="1" else "s"
plt.title(f"top {compare_by} neighbor{extra} for {dataset}, max query time {maximum_time_per_query} ms".title())
if save:
        plt.savefig(f"/home/jae4/FLINNG-Results/precall/{dataset}-{compare_by}-{maximum_time_per_query}.png", bbox_inches='tight')
else:
        plt.show()
