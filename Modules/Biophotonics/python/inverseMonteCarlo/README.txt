This folder contains the python scripts for the inverse monte carlo tissue estimation using
regression forests and domain adaption.

You can find the needed software to run this code in TechtreePython.txt

Most important scripts are:

To generate data use the generateAllSpectra.py script and activate what is needed.
You will need a working gpumcml on your computer to execute this code.
The general functions to generate Monte Carlo simulation can be found in the generate/ subfolder.
perfectGrid.py - generates a grid of reflectance spectra without any noise
randomUniform.py - generates randomly drawn reflectance spectra without any noise
randomNonUniform.py - generates randomly drawn reflectance spectra with noise

optimization.py - to test tissue parameter estimation using optimization algorithms

randomForest.py - to test tissue parameter estimation using random forests (domain adaption is not implemented yet)


