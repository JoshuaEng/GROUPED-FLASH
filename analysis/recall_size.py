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
num_queries = 10000
dataset = args.my_file.split("_")[1]
size_map = {"url": 2376130, "webspam": 330000}
num_points = size_map[dataset]

def parse_file(file_name, line_hint, size_func):   
        data = []
        with open(file_name, "r") as f:
                while True:
                        line = f.readline()
                        if line == "":
                                break
                        if line.split(" ")[0] == line_hint:
                                start = line
                                while True:
                                        line = f.readline()
                                        if line.startswith("Queried"):
                                                time = float(line.strip().split(" ")[-1][:-3])
                                                if time > num_queries * maximum_time_per_query:
                                                        break
                                        if line.startswith(compare_by):
                                                r = float(line.split()[2])
                                                data.append((r, size_func(start)))
                                                break
                                        if line == "":
                                                break

        return data

def flinng_size_calc(line):
        label, r, b, hash_range, reps = line.split()
        return 8 * int(r) * int(reps) * num_points / 1e9 / 4

def flash_size_calc(line):
        label, reservoire_size, hash_range, reps = line.split()
        return 8 * (2**int(hash_range)) * int(reps) * int(reservoire_size) / 1e9

group_size_data = parse_file(args.my_file, "STATS_GROUPS:", flinng_size_calc)
normal_size_data = parse_file(args.their_file, "STATS_NORMAL:", flash_size_calc)


import matplotlib.pyplot as plt
import math
plt.scatter([x for x, y in group_size_data], [y for x, y in group_size_data], marker='x', label=f'FLINNG' )
plt.scatter([x for x, y in normal_size_data], [y for x, y in normal_size_data], marker='s', label=f'FLASH' )
dataset = args.my_file.split("_")[1]
plt.legend(loc='upper right')
plt.xlabel("Recall")
plt.ylabel('Size (GB)')
extra = "" if compare_by=="1" else "s"
plt.title(f"Index size by recall of {compare_by} for {dataset}, max query time {maximum_time_per_query} ms".title())
if save:
        plt.savefig(f"/home/jae4/FLINNG-Results/precall/{dataset}-{compare_by}-{maximum_time_per_query}.png", bbox_inches='tight')
else:
        plt.show()
