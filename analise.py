import seaborn as sns
import matplotlib.pyplot as plt

with open("saidas/temposseq.txt") as seqout:
    seq = [float(x) for x in seqout.read().splitlines()]

with open("saidas/tempospar.txt") as parout:
    par = [float(x) for x in parout.read().splitlines()]

sns.set_style("whitegrid")
sns.set_palette("Set2")
sns.lineplot(x=range(1, 11), y=seq, label="Sequencial")
sns.lineplot(x=range(1, 11), y=par, label="Paralelo")
plt.title("Tempos de Execução para um mesmo número de threads")
plt.xlabel("Execução")
plt.ylabel("Tempo (s)")
plt.legend()
sns.despine()
plt.savefig("tempos.png")
#plt.show()

