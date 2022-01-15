import matplotlib.pyplot as plt
from math import*

Temperature=[22.60,20.60,14.70,10.40,8.00,6.30,5.10,4.30,3.80,3.30,3.10,3.00,2.90,2.80,2.80,2.80,2.80,2.80,2.90,3.00,3.10,3.10,3.10,3.20,3.20,3.20,3.20,3.10,3.10,3.10,3.10]
Temps= list(range(0,len(Temperature)))
for loop in range(len(Temps)):
	Temps[loop]*=5

Limite_x = [0,Temps[-1]]
Limite_y = [3,3]

plt.figure(0, figsize=(10, 4))
plt.plot(Temps , Temperature , c="r" , label="Temperature")
plt.plot(Limite_x , Limite_y , c="black" , label="Limite - 3°C" , linestyle="--")

plt.xlabel('Temps ( en s )')
plt.ylabel('Temperature ( en degres celsius )')
plt.title('Variation de temperature en fonction du temps\n')
plt.subplots_adjust(right=0.97 , left=0.08)
plt.legend()
plt.grid()
plt.show()