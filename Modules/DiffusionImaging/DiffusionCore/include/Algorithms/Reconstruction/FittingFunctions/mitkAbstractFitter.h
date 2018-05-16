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

#ifndef _MITK_AbstractFitter_H
#define _MITK_AbstractFitter_H

#include <vnl/vnl_least_squares_function.h>
#include <mitkDiffusionPropertyHelper.h>
#include <itkVectorImage.h>
#include <mitkTensorImage.h>

namespace mitk {

class AbstractFitter: public vnl_least_squares_function
{

public:
  typedef mitk::DiffusionPropertyHelper::GradientDirectionType            GradientDirectionType;
  typedef mitk::DiffusionPropertyHelper::GradientDirectionsContainerType  GradientContainerType;
  typedef itk::VectorImage< unsigned short, 3 >                           DiffusionImageType;
  typedef TensorImage::PixelType                                          TensorType;

  AbstractFitter(unsigned int number_of_parameters, unsigned int number_of_measurements) :
    vnl_least_squares_function(number_of_parameters, number_of_measurements, no_gradient)
  {

  }

  GradientContainerType::Pointer gradientDirections;
  DiffusionImageType::PixelType measurements;
  std::vector<double> bValues;
  double S0;
  std::vector<int>  weightedIndices;

  void set_S0(double val)
  {
    S0 = val;
  }

  void set_measurements(const DiffusionImageType::PixelType& m)
  {
    measurements = m;
  }

  void set_bvalues(std::vector<double>& x)
  {
    bValues = x;
  }

  void set_weightedIndices(std::vector<int>& x)
  {
    weightedIndices = x;
  }

  void set_gradient_directions(const GradientContainerType::Pointer& directions)
  {
    gradientDirections = directions;
  }

  static void Sph2Cart(vnl_vector_fixed<double,3>& dir, const double &theta, const double &phi)
  {
    dir[0] = std::sin(theta)*std::cos(phi);
    dir[1] = std::sin(theta)*std::sin(phi);
    dir[2] = std::cos(theta);
  }

  static void Cart2Sph(const vnl_vector_fixed<double,3>& dir, double &theta, double &phi)
  {
    theta = std::acos( dir[2] );

    // phi goes from 0.0 (+x axis) and wraps at 2 * PI
    // theta goes from 0.0 (+z axis) and wraps at PI

    // if x and y are 0.0 or very close, return phi == 0
    if (fabs(dir[0]) + fabs(dir[1]) < 1E-9)
    {
      phi = 0.0;
    }
    else {
      // ie, if ( x == 0 && y == 0 ) == false
      if (dir[1] == 0.0)
      {
        if (dir[0] > 0.0)
          phi = 0.0;
        else
          phi = itk::Math::pi;
      }
      else if (dir[0] == 0.0)
      {
        // avoid div by zero
        if (dir[1] > 0) {
          phi = itk::Math::pi / 2.0;
        }
        else {
          phi = 1.5 * itk::Math::pi;
        }
      }
      else if (dir[0] > 0.0 && dir[1] > 0.0) // first quadrant
        phi = atan(dir[1] / dir[0]);
      else if (dir[0] < 0.0 && dir[1] > 0.0) // second quadrant
        phi = itk::Math::pi + atan(dir[1] / dir[0]);
      else if (dir[0] < 0.0 && dir[1] < 0.0) // third quadrant
        phi =  itk::Math::pi + atan(dir[1] / dir[0]);
      else // fourth quadrant
        phi = 2.0 * itk::Math::pi + atan(dir[1] / dir[0]);
    }
  }

};
}

#endif
