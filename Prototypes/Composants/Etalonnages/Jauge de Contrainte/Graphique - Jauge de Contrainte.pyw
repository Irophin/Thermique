import matplotlib.pyplot as plt
from math import*


Masse = [500,200,100,50,20,10,5,2,1,0]
Amplification = [630164.81,327957.18,225387.28,174526.07,143912.42,133822.93,128740.41,125687.94,124719.21,124071.41]
Contrainte = []
Fonction =[]

for i in Masse:
	Contrainte.append(round(i*10**-3*9.81,4))

for i in Amplification:
	Fonction.append(0.96875E-05*i - 1.1998)


plt.figure(0, figsize=(10, 4))
plt.title("Evolution de la contrainte en fonction de l'amplification")
plt.ylabel("Contrainte en (N)")
plt.xlabel("Amplification")
plt.text(250000,1,r" $P(N) = 0.96875 * 10^{-5}*Amplification-1.1998$", fontsize=10,c="r")
plt.grid()
plt.plot(Amplification,Contrainte,'.')
plt.plot(Amplification,Fonction,'-')

plt.show()