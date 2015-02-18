# -*- coding: utf-8 -*-
"""
Created on Sat Feb 07 23:42:41 2015

@author: Seb
"""

import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate    import RectBivariateSpline
from scipy.optimize       import minimize
from reflectanceError     import ReflectanceError
from mpl_toolkits.mplot3d import Axes3D
from matplotlib           import cm

from setup import data


#%% load data

dataFolder = "data/output/"

trainingParameters, trainingReflectances, testParameters, testReflectances = \
    data.perfect(dataFolder)


BVFs = np.unique(trainingParameters[:,0])
Vss  = np.unique(trainingParameters[:,1])


#%% build optimization function from rectangular reflectance grid

reflectanceGrid3D  = np.reshape(trainingReflectances, (len(BVFs), len(Vss), trainingReflectances.shape[1]))
functionToMinimize = ReflectanceError(BVFs, Vss, reflectanceGrid3D)


#%% do optimization

absErrors = np.zeros_like(testParameters)


for idx, (testParameter, testReflectance) in enumerate(zip(testParameters, testReflectances)):
    functionToMinimize.setReflectanceToMatch(testReflectance)
    minimization = minimize(functionToMinimize.f, [np.median(BVFs), np.median(Vss)], method="Nelder-Mead")
    # interpolation extrapolates with constant values. We just crop it to the bounds
    clippedX= np.clip(minimization.x, [min(BVFs), min(Vss)], [max(BVFs), max(Vss)])

    absErrors[idx,:] = np.abs(clippedX - testParameter)


#%% test

print("error distribution BVF, Volume fraction")
print("median: " + str(np.median(absErrors, axis=0)))
print("lower quartile: " + str(np.percentile(absErrors, 25, axis=0)))
print("higher quartile: " + str(np.percentile(absErrors, 75, axis=0)))



#%% test routines, put these in a unit test asap!
if __name__ == "__main__":

    rbs = RectBivariateSpline(BVFs,
                              Vss,
                              np.reshape(trainingReflectances[:,0],(100,100)))



    rbsFrom3DGrid = RectBivariateSpline(BVFs,
                          Vss,
                          reflectanceGrid3D[:,:,0])

    print("check if reflectance grid is build correctly. Reflectance at " + \
        "(0.04258016, 0.29442218): " + str(rbs( 0.04258016 , 0.29442218, dx=0)) + "; expected " + str(654.222))

    print("check if reflectance grid is build correctly. Reflectance at " + \
        "(0.1, 0.60): " + str(rbs( 0.1 , 0.6, dx=0)) + "; expected " + str(818.945))

    print("check if reflectance grid is build correctly. Reflectance at " + \
        "(0.01, 0.60): " + str(rbs(0.01 , 0.6,  dx=0)) + "; expected " + str(1120.118))


    print("check if reflectance grid is build correctly. Reflectance at " + \
        "(0.0112, 0.041): " + str(rbsFrom3DGrid(0.0112, 0.041, dx=0)) + "; expected " + str(rbs(0.0112, 0.041, dx=0)))

    reflectanceError = ReflectanceError(BVFs, Vss, reflectanceGrid3D)

    # reflectance at point (0.062, 0.149)
    exampleReflectance = [ 312.09769118,  327.67866117,  336.20970583,  343.10626114,
        344.77202411,  322.68062559,  280.01722363,  265.45441892,
        292.59760112,  294.58676191,  256.88475134,  296.10442177,
        388.44056814,  422.99479049,  425.6602738 ,  426.88353542,
        427.09604971,  413.01675144,  424.50008822,  423.78125335,
        422.71219033,  421.51283858,  421.32106797]

    reflectanceError.setReflectanceToMatch(exampleReflectance)

    print("check if reflectance error value is correctly computed. Expected: 0, actual: " +
        str(reflectanceError.f([6.191087469731736126e-02, 1.149252955326078018e-01])))

    print("check if derivate of reflectance error value is correctly computed. Expected: close to [0,0], actual: " +
        str(reflectanceError.df([6.191087469731736126e-02, 1.149252955326078018e-01])))

    # reflectance at point (0.01, 0.6)
    exampleReflectance2 = [1.120118468811987896e+03,    1.131318509608477598e+03,
                           1.137543168078165081e+03,    1.141159082538952589e+03,
                           1.139712500764461311e+03,    1.120915174518419235e+03,
                           1.081868496849364647e+03,    1.064766763752600582e+03,
                           1.084980179526470238e+03,    1.081898683840950525e+03,
                           1.038063983415024040e+03,    1.073340106470938281e+03,
                           1.117816020857113244e+03,    1.122940442828165033e+03,
                           1.116782763468411758e+03,    1.108377361568318520e+03,
                           1.104745713217586172e+03,    1.098370113333694690e+03,
                           1.093525893516002043e+03,    1.089713462981120756e+03,
                           1.085745981406935471e+03,    1.087101893762737973e+03,
                           1.083097058199290814e+03]

    reflectanceError.setReflectanceToMatch(exampleReflectance2)

    print("check if reflectance error value is correctly computed (second one). Expected: 0, actual: " +
        str(reflectanceError.f([0.01, 0.6])))

    print("check if derivate of reflectance error value is correctly computed (second one). Expected: close to [0,0], actual: " +
        str(reflectanceError.df([0.01, 0.6])))


    functionToMinimize.setReflectanceToMatch(trainingReflectances[0,:])
    absError = minimize(functionToMinimize.f, [0.05, 0.3], method="Nelder-Mead")
    #absError = minimize(functionToMinimize.f, [0.05, 0.3], method="BFGS", jac=functionToMinimize.df)

    print(" function evaluation yields: " + str(absError.x) + " (unclipped) with absolute error: " + str(functionToMinimize.f(absError.x)))
    print(" expected evaluation result: " + str([0.01, 0.04]) + " real result error: " + str(functionToMinimize.f([0.01, 0.04])))




    # check if grid interpolation looks good.
    #%%
    grid_x, grid_y = np.mgrid[min(BVFs):max(BVFs):100j, min(Vss):max(Vss):100j]
    grid_z = rbs.ev(grid_x, grid_y)

    fig = plt.figure(0)
    ax = fig.gca(projection='3d')
    surf = ax.plot_surface(grid_x, grid_y, grid_z, cmap=cm.jet, linewidth=1, antialiased=True)
    ax.scatter(testParameters[:,0], testParameters[:,1], testReflectances[:,0])
    ax.set_zlim3d(np.min(grid_z), np.max(grid_z))
    fig.colorbar(surf)

    plt.show()
  #  plt.imshow(grid_z.T, extent=(0.04,0.6,0.01,0.1))

