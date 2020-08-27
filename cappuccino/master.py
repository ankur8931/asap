import timeit
import os

batch_size = [16, 32, 64]
gamma = [0.6, 0.7, 0.8, 0.9, 0.99]
training_steps = [2000, 4000, 6000, 8000, 10000]
test_file = "small"
f= open("expt2.txt","w+")

for b in batch_size:
    for g in gamma:
        start = timeit.default_timer()
        os.system("python3.6 nasim/agents/dqn_agent.py --batch_size "+str(b)+" --gamma "+str(g)+" "+test_file)
        stop = timeit.default_timer()
        f.write("Batch Size:"+str(b)+", Gamma:"+str(g)+", Running Time:"+str(stop-start)+"(s) \n")
