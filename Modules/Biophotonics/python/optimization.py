# -*- coding: utf-8 -*-
"""
Created on Sat Feb 07 23:42:41 2015

@author: Seb
"""

from scipy.interpolate import RectBivariateSpline
import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import minimize
from reflectanceError import ReflectanceError
from mpl_toolkits.mplot3d import Axes3D
from matplotlib import cm


dataFolder = "outputRS/"

# load data
trainingParameters   = np.load(dataFolder + "paramterers2015February0511:02PM.npy")
trainingReflectances = np.load(dataFolder + "reflectances2015February0511:02PM.npy")

BVFs = np.unique(trainingParameters[:,0])
Vss  = np.unique(trainingParameters[:,1])

reflectanceGrid3D = np.reshape(trainingReflectances, (len(Vss), len(BVFs), trainingReflectances.shape[1]), order='F')

functionToMinimize = ReflectanceError(Vss, BVFs, reflectanceGrid3D)

testParameters   = np.load(dataFolder + "2015February1107:43PMparamterersRandom.npy")
testReflectances = np.load(dataFolder + "2015February1107:43PMreflectancesRandom.npy")

absErrors = np.zeros_like(testParameters)

functionToMinimize.setReflectanceToMatch(trainingReflectances[0,:])
absError = minimize(functionToMinimize.f, [0.3, 0.05], method="Nelder-Mead")
#absError = minimize(functionToMinimize.f, [0.04, 0.3], method="BFGS", jac=functionToMinimize.df)

print(" function evaluation yields: " + str(absError.x) + " with absolute error: " + str(functionToMinimize.f(absError.x)))
print(" expected evaluation result: " + str([0.04, 0.01]) + " real result error: " + str(functionToMinimize.f([0.04, 0.01])))


for idx, (testParameter, testReflectance) in enumerate(zip(testParameters, testReflectances)):
    functionToMinimize.setReflectanceToMatch(testReflectance)
    minimization = minimize(functionToMinimize.f, [0.3, 0.05], method="Nelder-Mead")
    # interpolation extrapolates with constant values. Since the minimization function is
    # covex, we can just crop it to the bounds
    # todo: sort out this ugly mess of flipping Vss and BVFs!
    estimatedVs = np.clip(minimization.x[0], min(Vss), max(Vss))
    estimatedBVF= np.clip(minimization.x[1], min(BVFs), max(BVFs))

    absErrors[idx,:] = np.abs([estimatedBVF, estimatedVs] - testParameter)



# plot




print("error distribution BVF, Volume fraction")
print("median: " + str(np.median(absErrors, axis=0)))
print("lower quartile: " + str(np.percentile(absErrors, 25, axis=0)))
print("higher quartile: " + str(np.percentile(absErrors, 75, axis=0)))



if __name__ == "__main__":

    rbs = RectBivariateSpline(Vss,
                              BVFs,
                              np.reshape(trainingReflectances[:,0],(100,100)).T)


    rbsFrom3DGrid = RectBivariateSpline(Vss,
                          BVFs,
                          reflectanceGrid3D[:,:,0])

    print("check if reflectance grid is build correctly. Reflectance at " + \
        "(0.29442218, 0.04258016): " + str(rbs( 0.29442218, 0.04258016 , dx=0)) + "; expected " + str(654.222))

    print("check if reflectance grid is build correctly. Reflectance at " + \
        "(0.60, 0.1): " + str(rbs( 0.6, 0.1 , dx=0)) + "; expected " + str(818.945))

    print("check if reflectance grid is build correctly. Reflectance at " + \
        "(0.60, 0.01): " + str(rbs( 0.6, 0.01 , dx=0)) + "; expected " + str(1120.118))


    print("check if reflectance grid is build correctly. Reflectance at " + \
        "(0.041, 0.0112): " + str(rbsFrom3DGrid(0.041,0.0112, dx=0)) + "; expected " + str(rbs(0.041,0.0112, dx=0)))

    reflectanceError = ReflectanceError(Vss, BVFs, reflectanceGrid3D)

    # reflectance at point (0.149, 0.062)
    exampleReflectance = [ 312.09769118,  327.67866117,  336.20970583,  343.10626114,
        344.77202411,  322.68062559,  280.01722363,  265.45441892,
        292.59760112,  294.58676191,  256.88475134,  296.10442177,
        388.44056814,  422.99479049,  425.6602738 ,  426.88353542,
        427.09604971,  413.01675144,  424.50008822,  423.78125335,
        422.71219033,  421.51283858,  421.32106797]

    reflectanceError.setReflectanceToMatch(exampleReflectance)

    print("check if reflectance error value is correctly computed. Expected: 0, actual: " +
        str(reflectanceError.f([1.149252955326078018e-01,    6.191087469731736126e-02])))

    print("check if derivate of reflectance error value is correctly computed. Expected: close to [0,0], actual: " +
        str(reflectanceError.df([1.149252955326078018e-01,    6.191087469731736126e-02])))

    # reflectance at point (0.6, 0.01)
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
        str(reflectanceError.f([0.6, 0.01])))

    print("check if derivate of reflectance error value is correctly computed (second one). Expected: close to [0,0], actual: " +
        str(reflectanceError.df([0.6, 0.01])))



    # check if grid interpolation looks good.
    #%%
    grid_x, grid_y = np.mgrid[0.04:0.6:100j, 0.01:0.1:100j]
    grid_z = rbs.ev(grid_x, grid_y)

    fig = plt.figure(0)
    ax = fig.gca(projection='3d')
    surf = ax.plot_surface(grid_x, grid_y, grid_z, cmap=cm.jet, linewidth=1, antialiased=True)
    ax.scatter(testParameters[:,1], testParameters[:,0], testReflectances[:,0])
    ax.set_zlim3d(np.min(grid_z), np.max(grid_z))
    fig.colorbar(surf)

    plt.show()
  #  plt.imshow(grid_z.T, extent=(0.04,0.6,0.01,0.1))

