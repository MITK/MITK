/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef mitkEigenToImageTransform_h
#define mitkEigenToImageTransform_h

#include <MitkClassificationCoreExports.h>
#include <mitkImageToImageFilter.h>

// Eigen
#include <Eigen/Dense>

namespace mitk
{
  class MITKCLASSIFICATIONCORE_EXPORT EigenToImageTransform
  {
  public:

    typedef Eigen::VectorXd VectorType;

    mitk::Image::Pointer operator()(const VectorType & matrix, const mitk::Image::Pointer & mask);

    static mitk::Image::Pointer transform(const VectorType & matrix, const mitk::Image::Pointer & mask);

  private:
    typedef itk::Image<unsigned char, 3> UCharImageType;
    typedef itk::Image<double, 3> DoubleImageType;

  };
}

#endif //mitkImageToEigenTransform_h
