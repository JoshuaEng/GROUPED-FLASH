# group_record2 = []
group_record = []
normal_record = []

import argparse

# R, B, REPS
mins = [float("inf"), float("inf"), float("inf")]
maxes = [0, 0, 0]

# Instantiate the parser
parser = argparse.ArgumentParser(description='Compare FLASH and FLINNG')
parser.add_argument("compare_by")
parser.add_argument("their_file")
parser.add_argument("my_file")
args = parser.parse_args()

look_for = args.compare_by

best_r = 0
with open(args.my_file, "r") as f:
        while True:
                line = f.readline()
                if line == "":
                        break
                if line.split(" ")[0] == "STATS_GROUPS:":
                        if int(line.split(" ")[1]) > 5:
                                continue
                        saved = line
                        while True:
                                line = f.readline()
                                if line.startswith("Queried"):
                                        time = float(line.split(" ")[-2][:-3])
                                if line.startswith(look_for):
                                        r = float(line.split()[2])
                                        if r > best_r:
                                                best_r = r
                                                real = saved
                                        split = saved.strip().split()[1:]
                                        for i in range(3):
                                                maxes[i] = max(maxes[i], int(split[i]))
                                                mins[i] = min(mins[i], int(split[i]))
                                        group_record.append((time, r, saved.strip()))
                                        break
                                if line == "":
                                        break
with open(args.their_file, "r") as f:
        while True:
                line = f.readline()
                if line == "":
                        break
                if line.split(" ")[0] == "STATS_NORMAL:":
                        while True:
                                line = f.readline()
                                if line.startswith("Queried"):
                                        time = float(line.strip().split(" ")[-1][:-3])
                                if line.startswith(look_for):
                                        r = float(line.split()[2])
                                        normal_record.append((time, r))
                                        break
                                if line == "":
                                        break




import matplotlib.pyplot as plt
size = 200
best_group = [-1] * size
best_normal = [-1] * size
lower_bound = 40
for x,y,z in group_record:
        position = int(y * size)
        value = 10000 / x * 1000
        if position > lower_bound:
                if best_group[position] < value:
                        best_group[position] = value

for x,y in normal_record:
        position = int(y * size)
        if x != 0:
                value = 10000 / x * 1000
                if position > lower_bound:
                        if best_normal[position] < value:
                                best_normal[position] = value
best_group = [(i / size, val) for i, val in enumerate(best_group) if val != -1]
best_normal = [(i / size, val) for i, val in enumerate(best_normal) if val != -1]
plt.plot([x for x,y in best_group], [y for x,y in best_group])
plt.plot([x for x,y in best_normal], [y for x,y in best_normal])
plt.legend(loc='upper left')
plt.xlabel(look_for)
plt.ylabel('Queries per second')
plt.show()