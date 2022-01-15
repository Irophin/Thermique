import matplotlib.pyplot as plt
from math import*

Resistance=[30.3,30.1,28.6,28,27.1,26.5,25.8,25.1,24.5,23.9,23.5,22.7,22.2,21.7,21.2,20.6,20,19.7,19.2,18.8,18.3,17.9,
	17.5,17.1,16.6,16.3,15.9,15.5,15.1,14.8,14.5,14.1,13.8,13.5,13.2,12.9,12.6,12.3,12.1,11.8,11.5,11.3,11,10.8,10.5,
	10.3,10.1,9.9,9.6,9.4,9.2,9,8.8,8.6,8.5,8.3,8.1,7.9,7.8,7.6,7.4,7.3,7.1,6.9,6.8,6.7,6.6,6.46,6.31,6.19,6.06,5.95,
	5.84,5.72,5.6,5.5,5.38,5.29,5.18,5.07,4.96,4.87,4.76,4.69,4.58,4.48,4.39,4.32,4.23,4.14,4.08,4,3.91,3.85,3.77,3.7,
	3.63,3.56,3.5,3.42,3.36,3.3,3.23]

Graph_Resistance = []
Temperature=list(range(3,len(Resistance)+3))

for loop in range(len(Resistance)):
	Temperature[loop]*=0.5
	Graph_Resistance.append(39.077 * exp(-(Resistance[loop] - 5.08) / 9.802) + 1.294)

plt.figure(0, figsize=(10, 4))
plt.grid()
plt.title("Graphique d'étalonnage")
plt.plot(Resistance, Temperature, '.',label='Points issus de la mesure')
plt.plot(Resistance,Graph_Resistance, 'r-' , label='Résultats obtenus en fonction\n de la valeur de la résistance')
plt.text(14, 25, r" $T(°C) = 39.077 * e^{-\frac{kΩ-5.08}{9.802}}+1.294$", fontsize=10,c="r")
plt.xlabel('Resistance ( en KiloOhm )')
plt.ylabel('Temperature ( en degres celsius )')
plt.title('Variation de la temperature en fonction de la resistance\n')
plt.legend()
plt.show()