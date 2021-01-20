from get_data import get_all_data
import traceback

dataset = "promethion"
goal = 0.9
precisions = []
topk = "R10"
max_time = 20
for method, data in get_all_data(dataset):
	try:
		best_precision = 0		
		for measure, time, recall, precision in data:
			if measure.split("@")[0] == topk and recall > goal and time < max_time:
				best_precision = max(best_precision, precision)

		if best_precision != 0:
			precisions.append(best_precision)
			print(method, best_precision)	
	except Exception as e:
		traceback.print_exc() 
		pass

precisions.sort()
print(f"At {goal} reall with max time of {max_time}ms, the best method on {dataset} is {precisions[-1] / precisions[-2]} better on precision")