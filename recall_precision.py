# group_record2 = []
group_record = []
normal_record = []

import argparse

maximum_time_per_query = 20

# Instantiate the parser
parser = argparse.ArgumentParser(description='Compare FLASH and FLINNG')
parser.add_argument("their_file")
parser.add_argument("my_file")
args = parser.parse_args()

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
                                if line.startswith("R@") and not line.startswith("R@k"):
                                        r = float(line.split()[2])
                                        p = 1.0 / int(line.split()[0][2:])
                                        group_record.append((r, p))
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
                                if line.startswith("R@") and not line.startswith("R@k"):
                                        r = float(line.split()[2])
                                        p = 1.0 / int(line.split()[0][2:])
                                        normal_record.append((r, p))
                                if line == "":
                                        break

import matplotlib.pyplot as plt
import math
plt.scatter([x for x, y in group_record], [y for x, y in group_record], marker='x', label=f'FLINNG' )
plt.scatter([x for x, y in normal_record], [y for x, y in normal_record], marker='s', label=f'FLASH' )
dataset = args.my_file.split("_")[1]
plt.legend(loc='upper left')
plt.xlabel("Recall")
plt.ylabel('Precison')
plt.title((dataset).title())
plt.show()

# index = 1
# names = ['R', 'B', 'REPS']
# lower_bound = 0.2
# num_points = 2386130
# show_size = False
# show_params = False

# import matplotlib.pyplot as plt
# import math
# plt.scatter([y for (x,y, z) in group_record if y > lower_bound], [math.log10(10000 / x * 1000) for (x,y, z) in group_record if y > lower_bound], marker='x', label=f'FLINNG' )
# plt.scatter([y for (x,y, z) in normal_record if x != 0  and y > lower_bound], [math.log10(10000 / x * 1000) for (x,y, z) in normal_record if x != 0  and y > lower_bound], marker='s', label='FLASH')

# if show_size:
#         for record in normal_record:
#                 if (record[1] > lower_bound):
#                         label, reservoire_size, hash_range, reps = record[2].split()
#                         plt.annotate(str((2**int(hash_range)) * int(reps) * int(reservoire_size) / 1000000000) + "GB", [record[1], math.log10(10000 / record[0] * 1000)])
#         for record in group_record:
#                 if (record[1] > lower_bound):
#                         label, r, b, hash_range, reps = record[2].split()
#                         plt.annotate(str(int(r) * int(reps) * num_points / 1000000000 / 4) + "GB", [record[1], math.log10(10000 / record[0] * 1000)])

# if show_params:
#         for record in normal_record:
#                 if (record[1] > lower_bound):
#                         label, reservoire_size, hash_range, reps = record[2].split()
#                         plt.annotate(str((reservoire_size, hash_range, reps)), [record[1], math.log10(10000 / record[0] * 1000)])
#         for record in group_record:
#                 if (record[1] > lower_bound):
#                         label, r, b, hash_range, reps = record[2].split()
#                         plt.annotate(str((r, b, hash_range, reps)), [record[1], math.log10(10000 / record[0] * 1000)])

# dataset = args.my_file.split("_")[1]
# plt.legend(loc='upper left')
# start, end = look_for.split("@")
# lookup = {"T": f"Top {end} recall in first {end} returned points", "S": f"Similarity of {end} returned points", "R": f"Recall of top 1 at {end}"}
# plt.xlabel(lookup[start])
# plt.ylabel('Queries per second (log 10)')
# plt.title((dataset).title())
# plt.show()