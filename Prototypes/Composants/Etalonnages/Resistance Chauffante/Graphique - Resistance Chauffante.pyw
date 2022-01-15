import matplotlib.pyplot as plt
from math import*

Temperature=[20.5,22.9,32.9,46.1,55.4,70.4,87.1,104.0,121.1,142.7,157.2,169.2,178.7,185.7,190.0,193.8,196.4,198.3,199.8,200.9,202.0,202.7,203.3,204.0,204.4,204.8,205.2,205.7,206.0,206.5,206.9,207.2,207.6,208.0,208.2,208.5,208.9,209.2,209.6,210.0,210.4,210.7,211.0,211.2,211.5,210.8,210.5,210.6,210.7]

Temps= list(range(0,len(Temperature)))
for loop in range(len(Temps)):
	Temps[loop]*=5

Limite_x = [0,Temps[-1]]
Limite_y = [210,210]

plt.figure(0, figsize=(10, 4))
plt.plot(Temps , Temperature , c="r" , label="Temperature")
plt.plot(Limite_x , Limite_y , c="black" , label="Limite - 210°C" , linestyle="--")

plt.xlabel('Temps ( en s )')
plt.ylabel('Temperature ( en degres celsius )')
plt.title('Variation de temperature en fonction du temps\n')
plt.subplots_adjust(right=0.97 , left=0.08)
plt.legend()
plt.grid()
plt.show()