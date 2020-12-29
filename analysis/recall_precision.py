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
size_map = {"url": 2376130, "webspam": 340000, "genomes": 117219, "proteomes": 116373, "promethion": 3696341}
num_points = size_map[dataset]
print(dataset, maximum_time_per_query, compare_by)

def parse_file(file_name, line_hint, size_func):   
        record = {}
        sizes = {}
        with open(file_name, "r") as f:
                while True:
                        line = f.readline()
                        if line == "":
                                break
                        if line.split(" ")[0] == line_hint:
                                start = line
                                while True:
                                        line = f.readline()
                                        if line.startswith("T@k"):
                                                break
                                        if line.startswith("Queried"):
                                                time = float(line.strip().split(" ")[-1][:-3])
                                                if time > num_queries * maximum_time_per_query:
                                                        break
                                        if line.startswith(f"R{compare_by}@") and not line.startswith(f"R{compare_by}@k"):
                                                r = float(line.split()[2])
                                                p = int(compare_by) / int(line.split()[0][2 + len(compare_by):])
                                                if p not in record:
                                                        record[p] = 0
                                                if r > record[p]:
                                                        record[p] = r
                                                        sizes[p] = size_func(start)
                                        if line == "":
                                                break

        small_sizes = {key:val for key, val in sizes.items()} 
        small_record = {key:val for key, val in record.items()}   
        fudge_factor = 0.05                                
        with open(file_name, "r") as f:
                while True:
                        line = f.readline()
                        if line == "":
                                break
                        if line.split(" ")[0] == line_hint:
                                start = line
                                while True:
                                        line = f.readline()
                                        if line.startswith("T@k"):
                                                break
                                        if line.startswith("Queried"):
                                                time = float(line.strip().split(" ")[-1][:-3])
                                                if time > num_queries * maximum_time_per_query:
                                                        break
                                        if line.startswith(f"R{compare_by}@") and not line.startswith(f"R{compare_by}@k"):
                                                r = float(line.split()[2])
                                                p = int(compare_by) / int(line.split()[0][2 + len(compare_by):])
                                                if r >= record[p] * (1 - fudge_factor):
                                                        if size_func(start) < small_sizes[p] :
                                                                small_sizes[p] = size_func(start)
                                                                small_record[p] = r
                                        if line == "":
                                                break
        
        return record, sizes, small_record, small_sizes


def flinng_size_calc(line):
        label, r, b, hash_range, reps = line.split()
        return 8 * int(r) * int(reps) * num_points / 1e9 / 4

def flash_size_calc(line):
        label, reservoire_size, hash_range, reps = line.split()
        return 8 * (2**int(hash_range)) * int(reps) * int(reservoire_size) / 1e9

group_record, group_sizes, group_small_record, group_small_sizes = parse_file(args.my_file, "STATS_GROUPS:", flinng_size_calc)
normal_record, normal_sizes, normal_small_record, normal_small_sizes = parse_file(args.their_file, "STATS_NORMAL:", flash_size_calc)

if not save:
        def get_average(sizes):
                return sum(sizes.values()) / len(sizes.values())
        print(get_average(group_sizes), get_average(group_small_sizes))
        print(get_average(normal_sizes), get_average(normal_small_sizes))
        print(get_average({key:(group_record[key] - normal_record[key]) for key in normal_record.keys()}))
        print(get_average({key:(group_small_record[key] - normal_small_record[key]) for key in normal_record.keys()}))

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
