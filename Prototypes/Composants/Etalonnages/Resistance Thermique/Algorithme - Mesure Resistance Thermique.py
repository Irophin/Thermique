import os
import math
from random import *
import matplotlib.pyplot as plt

# Declaration des variables

Temps_mesure = 600

AireC = 0.01
TempC = [40]
TempF = [15]
dTemp= [25]
Moyenne_dTemp = 0
Energie = 0
Puissance = 0
Flux_Themique = 0
Epaisseur = 0.02
Lambda = 0
ResistanceT = 0
Dissipation = 1.1

Tension = 12
Intensite = 2


for i in range(1,Temps_mesure):

	if TempC[-1] < TempC[0] :
		TempC.append(TempC[-1]+uniform(-0.1,0.3))
	else:
		TempC.append(TempC[-1]-uniform(0.005,0.01))

	if TempF[-1] < TempF[0] :
		TempF.append(TempF[-1]+uniform(-0.01,00.3))
	else:
		TempF.append(TempF[-1]-uniform(-0.01,00.3))

	dTemp.append(TempC[-1] - TempF[-1])

	if TempC[i] < TempC[0] :
		Energie += Tension * Intensite

Puissance = Energie/Temps_mesure - Dissipation 

Temps = list(range(Temps_mesure))

Moyenne_dTemp =round( sum(dTemp) / len(dTemp) , 2)

Lambda = (Puissance * Epaisseur)/(AireC * Moyenne_dTemp)
ResistanceT = Epaisseur / Lambda

print("\nAffichage des resultats de cette simulation :")
print("\tPour un asservissement à 10°C et 40°C et une durée de 10 min\n")
print(">Moyenne de la difference de temperature : " , Moyenne_dTemp , "°C [ΔT]")
print(">Energie nécessaire pour maintenir l'asservissement de la resitance : " , round(Energie,2) , " J")
print(">Puissance moyenne " + str(round(Puissance,2)) + " W [P]")
print(">Surface de la plaque chaude : " , AireC , "m² [A]")
print(">Epaisseur du matériau :" , Epaisseur , "m [L]")
print(">Conductivité thermqiue : " , round(Lambda,4) , "W/m×K [λ]")
print(">Resistance thermique : " , round(ResistanceT,4) , "m²×K/W [R]\n")

reponse = input("\nVoulez vous affichez le graphique de temperature?\n\t >>").lower()

if reponse == "oui" :
	plt.plot(Temps , dTemp , c="g" , label="Delta Temperature")
	plt.plot(Temps , TempC , c="r" , label="Temperature Chaude")
	plt.plot(Temps , TempF , c="b" , label="Temperature Froide")
	plt.plot([0,Temps_mesure-1] , [Moyenne_dTemp,Moyenne_dTemp] , c="k" , linestyle="-" , lw=2)
	plt.xlabel('Temps ( en s )')
	plt.ylabel('Temperature ( en degres celsius )')
	plt.title('Variation de temperature en fonction du temps\n')
	plt.legend(loc='upper right')
	plt.show()
	print("\nFermeture du graphique...\n")


os.system("PAUSE")

