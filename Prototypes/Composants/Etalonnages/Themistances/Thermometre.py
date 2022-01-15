import os
from math import*

Resistance = float(input("Quelle est la valeur de la resistance en Kilo Ohm >"))
Temperature = 39.077 * exp(-(Resistance - 5.08) / 9.802) + 1.294
print(round(Temperature,1), " °C")
os.system("PAUSE")