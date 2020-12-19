def save_graph(path, look_for, my_file, their_file):
        # R, B, REPS
        mins = [float("inf"), float("inf"), float("inf")]
        maxes = [0, 0, 0]

        group_record = []
        normal_record = []

        best_r = 0
        with open(my_file, "r") as f:
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
        with open(their_file, "r") as f:
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
                                        if line.startswith(look_for):
                                                r = float(line.split()[2])
                                                normal_record.append((time, r, saved.strip()))
                                                break
                                        if line == "":
                                                break

        index = 1
        names = ['R', 'B', 'REPS']
        lower_bound = 0.2
        num_points = 2386130
        show_size = False
        show_params = False

        import matplotlib.pyplot as plt
        import math
        plt.scatter([y for (x,y, z) in group_record if y > lower_bound], [math.log10(10000 / x * 1000) for (x,y, z) in group_record if y > lower_bound], marker='x', label=f'FLINNG' )
        plt.scatter([y for (x,y, z) in normal_record if x != 0  and y > lower_bound], [math.log10(10000 / x * 1000) for (x,y, z) in normal_record if x != 0  and y > lower_bound], marker='s', label='FLASH')

        if show_size:
                for record in normal_record:
                        if (record[1] > lower_bound):
                                label, reservoire_size, hash_range, reps = record[2].split()
                                plt.annotate(str((2**int(hash_range)) * int(reps) * int(reservoire_size) / 1000000000) + "GB", [record[1], math.log10(10000 / record[0] * 1000)])
                for record in group_record:
                        if (record[1] > lower_bound):
                                label, r, b, hash_range, reps = record[2].split()
                                plt.annotate(str(int(r) * int(reps) * num_points / 1000000000 / 4) + "GB", [record[1], math.log10(10000 / record[0] * 1000)])

        if show_params:
                for record in normal_record:
                        if (record[1] > lower_bound):
                                label, reservoire_size, hash_range, reps = record[2].split()
                                plt.annotate(str((reservoire_size, hash_range, reps)), [record[1], math.log10(10000 / record[0] * 1000)])
                for record in group_record:
                        if (record[1] > lower_bound):
                                label, r, b, hash_range, reps = record[2].split()
                                plt.annotate(str((r, b, hash_range, reps)), [record[1], math.log10(10000 / record[0] * 1000)])

        dataset = my_file.split("_")[1]
        plt.legend(loc='upper left')
        start, end = look_for.split("@")
        lookup1 = {"T": f"Top {end} recall in first {end} returned points", "S": f"Similarity of {end} returned points", "R": f"Recall of top 1 at {end}"}
        plt.xlabel(lookup1[start])
        plt.ylabel('Queries per second (log 10)')
        plt.title((dataset).title())
        if start == "R":
                plt.savefig(f"{path}{(dataset).title()}-1-at-{end}.png", bbox_inches='tight')
        if start == "T":
                plt.savefig(f"{path}{(dataset).title()}-{end}-at-{end}.png", bbox_inches='tight')
        if start == "S":
                plt.savefig(f"{path}{(dataset).title()}-sim-at-{end}.png", bbox_inches='tight')
        plt.close()

algos = ["gplus", "webspam", "url"]
things = ["R@10", "R@100", "T@10", "T@100"]
for algo in algos:
        their_file = f"their_{algo}_cached.txt"
        my_file = f"my_{algo}_cached.txt"
        for thing in things:
                save_graph("../FLINNG-Results/", thing, my_file, their_file)

