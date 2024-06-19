/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2010-10-15 13:56:51 +0200 (Fr, 15 Okt 2010) $
Version:   $Revision: 18261 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKStandardICPPointRegisterPOINTREGISTER_H_HEADER_INCLUDED_
#define MITKStandardICPPointRegisterPOINTREGISTER_H_HEADER_INCLUDED_

//mitk headers
#include "mitkPointSet.h"

//itk headers
#include <itkMatrix.h>

//vtk headers
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

namespace mitk {
  /** Documentation
    * @brief This class offers static methods for computation of standard ICP registration.
    */
class StandardICPPointRegister
  {

  public:

    /**
     * @brief This method executes the standard iterative closest point algorithm to register a moving pointset X onto a static point set Y.
     *
     * @param MovingSet       This is the moving point set, which is registered to the static point set (StaticSet).
     * @param StaticSet       This is the static point set to which the moving set is registered. The static set must have the same number of points like the moving set.
     * @param Threshold       This is the size of change of the moving set above which the iteration continues.
     * @param max_iterations  Maximum iterations until the algorithm terminates. Default value is 100.
     * @param TransformationR Return value: This parameter is a reference to place where the return value "R-component of the transformation" will be saved.
     * @param TransformationT Return value: This parameter is a reference to place where the return value "t-component of the transformation" will be saved.
     * @param FRE             Return value: Here the FRE of the transformation will be saved.
     * @param n               Return value: Here the number of iterations will be saved.
     * @return                Returns true if the algorithm was completed successfully, false if not.
     */
    static bool StandardICPPointRegisterAlgorithm(  mitk::PointSet::Pointer MovingSet,
                                                    mitk::PointSet::Pointer StaticSet,
                                                    double Threshold,
                                                    itk::Matrix<double,3,3>& TransformationR,
                                                    itk::Vector<double,3>& TransformationT,
                                                    double& FRE,
                                                    int& n,
                                                    std::string& ErrorMessage,
                                                    int max_iterations = 100
                                                    );

    /**
     * @brief This method executes the standard iterative closest point algorithm to register a moving pointset X onto a static point set Y.
     *
     * @param MovingSurface       This is the moving surface, which is registered to the static surface (StaticSet).
     * @param StaticSurface       This is the static surface to which the moving set is registered. The static surface must have the same number of points like the moving set.
     * @param Threshold       This is the size of change of the moving set above which the iteration continues.
     * @param max_iterations  Maximum iterations until the algorithm terminates. Default value is 100.
     * @param TransformationR Return value: This parameter is a reference to place where the return value "R-component of the transformation" will be saved.
     * @param TransformationT Return value: This parameter is a reference to place where the return value "t-component of the transformation" will be saved.
     * @param FRE             Return value: Here the FRE of the transformation will be saved.
     * @param n               Return value: Here the number of iterations will be saved.
     * @return                Returns true if the algorithm was completed successfully, false if not.
     */
    static bool StandardICPPointRegisterAlgorithm(  vtkSmartPointer<vtkPolyData> MovingSurface,
                                                    vtkSmartPointer<vtkPolyData> StaticSurface,
                                                    double Threshold,
                                                    itk::Matrix<double,3,3>& TransformationR,
                                                    itk::Vector<double,3>& TransformationT,
                                                    double& FRE,
                                                    int& n,
                                                    std::string& ErrorMessage,
                                                    int max_iterations = 100
                                                    );

  protected:

    //################### private help methods #####################

    static vtkSmartPointer<vtkPolyData> convertPointSetToVtkPolyData(mitk::PointSet::Pointer PointSet);

  };

} // namespace mitk
#endif
