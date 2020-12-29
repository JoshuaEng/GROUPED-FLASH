import argparse

# Instantiate the parser
parser = argparse.ArgumentParser(description='Compare FLASH and FLINNG')
parser.add_argument("compare_by")
parser.add_argument("their_file")
parser.add_argument("my_file")
parser.add_argument("lower_bound")
parser.add_argument('-save', action='store_true')
args = parser.parse_args()
save = args.save
compare_by = args.compare_by
num_queries = 10000
dataset = args.my_file.split("_")[1]
lower_bound = float(args.lower_bound)

print(dataset, compare_by)

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
        return record

group_record = parse_file(args.my_file, "STATS_GROUPS:")
normal_record = parse_file(args.their_file, "STATS_NORMAL:")


import matplotlib.pyplot as plt
import math
plt.scatter([y for (x,y, z) in group_record if y > lower_bound], [math.log10(10000 / x * 1000) for (x,y, z) in group_record if y > lower_bound], marker='x', label=f'FLINNG' )
plt.scatter([y for (x,y, z) in normal_record if x != 0  and y > lower_bound], [math.log10(10000 / x * 1000) for (x,y, z) in normal_record if x != 0  and y > lower_bound], marker='s', label='FLASH')
plt.legend(loc='upper right')
plt.xlabel(compare_by)
plt.ylabel('Queries per second (log 10)')
plt.title((dataset).title())
if save:
        valid = compare_by.replace("@", "at")
        plt.savefig(f"/home/jae4/FLINNG-Results/trecall/{dataset}-{valid}-{int(lower_bound*100)}.png", bbox_inches='tight')
else:
        plt.show()
