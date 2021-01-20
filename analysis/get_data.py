num_queries = 10000
import traceback

def _read_hnsw(file_name):
	record = []
	with open(file_name, "r") as f:
		while True:
			line = f.readline()
			if not line:
				break
			values = line.split()[1].split(",")
			record.append((line.split()[0], float(values[0]), float(values[1]), float(values[2])))
	return record

def _read_flash(file_name):
	record = []
	with open(file_name, "r") as f:
		while True:
			line = f.readline()
			if line == "":
				break
			if line.startswith("STAT"):
				while True:
					line = f.readline()
					if line.startswith("Queried"):
						time = float(line.strip().split(" ")[-1][:-3])
					if line.startswith("R") and "k" not in line:
						record.append((line.split()[0], time / num_queries, float(line.split()[2]),  float(line.split()[2])/int(line.split()[0].split("@")[1])*int(line.split()[0].split("@")[0][1:])))
					if line == "":
						break
	return record

def _get_raw_data(file_name):   
	if "hnsw" in file_name or "falconn" in file_name:
		return _read_hnsw(file_name)
	else:
		return _read_flash(file_name)

methods = ["flinng", "flash", "hnsw", "falconn"]
colors = {"flinng":"#264478","flash":"#F08406", "hnsw":"#232f3e", "falconn":"#900603"}

def get_all_data(dataset):
	data = []
	for method in methods:
		file_name = "../" + method + "_" + dataset + ".txt"
		try:
			data.append((method, _get_raw_data(file_name)))
		except:
			# print(method, "failed in get data on", dataset)
			# traceback.print_exc()
			pass
	return data

def get_all_data_colored(dataset):
	return [(method, data, colors[method]) for method, data in get_all_data(dataset)]