/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKTARGETPOINTSCALCULATOR_H_HEADER_INCLUDED_
#define MITKTARGETPOINTSCALCULATOR_H_HEADER_INCLUDED_

// mitk headers
#include "mitkCommon.h"
#include "mitkImage.h"
#include "mitkPointSet.h"
#include "mitkSurface.h"

// itk headers
#include "itkObject.h"
#include <itkObjectFactory.h>

namespace mitk
{
  /**
   * @brief This class offers methods to automatically calculate target points inside a (closed) surface.
   */
  class TargetPointsCalculator : public itk::Object
  {
  public:
    mitkClassMacroItkParent(TargetPointsCalculator, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** @brief identifier for target point calculation method */
      enum TargetPointCalculationMethod {
        EvenlyDistributedTargetPoints,
        OneTargetPointInCenterOfGravity
      };

    /** @brief Sets the method for the target point calculation. Default value is EvenlyDistributedTargetPoints. */
    void SetTargetPointCalculationMethod(TargetPointCalculationMethod method);

    /** @brief Sets the inter point distance (in mm), which is a parameter for the evenly distributed target points.
      *         This parameter is only used if the method is set to EvenlyDistributedTargetPoints. Default value is 20.
      */
    void SetInterPointDistance(int d);

    /** @brief Sets the input surface. This parameter must be set before calculation is started. */
    void SetInput(mitk::Surface::Pointer input);

    /** @brief  Calculates the target points.
      * @return Returns true if calculation was successful. False if not, you can get an error message in this case.
      */
    bool DoCalculate();

    /** @return Returns the calculated target points. Returns null if no target points are calculated yet. */
    mitk::PointSet::Pointer GetOutput();

    /** @return Returns the last error message. Returns an empty string if there was no error yet. */
    std::string GetErrorMessage();

  protected:
    TargetPointsCalculator();
    ~TargetPointsCalculator() override;

    typedef itk::Image<unsigned char, 3> ImageType;

    int m_InterPointDistance;

    mitk::PointSet::Pointer m_Output;

    mitk::Surface::Pointer m_Input;

    std::string m_ErrorMessage;

    TargetPointCalculationMethod m_Method;

    mitk::Image::Pointer CreateBinaryImage();

    mitk::PointSet::Pointer CreateTargetPoints(mitk::Image::Pointer binaryImage);

    bool isInside(ImageType::Pointer currentImageAsitkImage, mitk::Point3D p);

    int RoundUpToGatter(int i, int gatter);

    int RoundUpToCentimeters(int i);

    mitk::PointSet::Pointer CreateTargetPointInCOG(mitk::Surface::Pointer surface);
  };
}

#endif
