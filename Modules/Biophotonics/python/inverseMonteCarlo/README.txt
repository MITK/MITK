This folder contains the python scripts for the inverse monte carlo tissue estimation using
regression forests and domain adaption.

You can find the needed software to run this code in TechtreePython.txt

Most important scripts are:

To generate data: generate*.py
You will need a working gpumcml on your computer to execute this code.
generateReflectanceSpectra.py - generates a grid of reflectance spectra without any noise
generateRandomSpectra.py - generates randomly drawn reflectance spectra without any noise
generateNoisyRandomSpectra.py - generates randomly drawn reflectance spectra with noise

optimization.py - to test tissue parameter estimation using optimization algorithms

randomForest.py - to test tissue parameter estimation using random forests (domain adaption is not implemented yet)


