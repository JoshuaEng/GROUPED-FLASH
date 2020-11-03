look_for = "T@100"
# group_record2 = []
group_record = []
normal_record = []
# with open("their_gplus.txt", "r") as f:
#         while True:
#                 line = f.readline()
#                 if line == "":
#                         break
#                 if line.split(" ")[0] == "STATS_GROUPS:":
#                         while True:
#                                 line = f.readline()
#                                 if line.startswith("Queried"):
#                                         time = int(line.split(" ")[-2][:-3])
#                                 if line.startswith(look_for):
#                                         r = float(line.split()[2])
#                                         group_record2.append((time, r))
#                                         break
#                                 if line == "":
#                                         break
#                                 if line.startswith("FLASH did not return"):
#                                         continue
best_r = 0
with open("my_gplus.txt", "r") as f:
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
                                        time = int(line.split(" ")[-2][:-3])
                                if line.startswith(look_for):
                                        r = float(line.split()[2])
                                        if r > best_r:
                                                best_r = r
                                                print(r, saved)
                                                real = saved
                                        group_record.append((time, r))
                                        break
                                if line == "":
                                        break
print(real)
with open("their_gplus.txt", "r") as f:
        while True:
                line = f.readline()
                if line == "":
                        break
                if line.split(" ")[0] == "STATS_NORMAL:":
                        while True:
                                line = f.readline()
                                if line.startswith("Queried"):
                                        time = int(line.split(" ")[-2][:-3])
                                if line.startswith(look_for):
                                        r = float(line.split()[2])
                                        normal_record.append((time, r))
                                        break
                                if line == "":
                                        break



# best = [float('inf')] * 20
# ids = [-1] * 20
# for i, (time, r) in enumerate(group_record):
#         val = int((r - 0.6) / 0.2 * 20)
#         if (best[val] > time):
#                 best[val] = time
#                 ids[val] = i

# for i in range(20):
#         if ids[i] != -1:
#                 print(0.6 + i * 0.01, group_vals[ids[i]].strip())

import matplotlib.pyplot as plt
plt.scatter([y for (x,y) in group_record], [10000 / x * 1000 for (x,y) in group_record], c='b', marker='x', label='FLINNG1')
# plt.scatter([y for (x,y) in group_record2], [10000 / x * 1000 for (x,y) in group_record2], c='g', marker='o', label='FLINNG2')
plt.scatter([y for (x,y) in normal_record if x != 0], [10000 / x * 1000 for (x,y) in normal_record if x != 0], c='r', marker='s', label='FLASH')
plt.legend(loc='upper left')
plt.xlabel(look_for)
plt.ylabel('Queries per second')
plt.show()