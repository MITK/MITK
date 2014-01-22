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

#ifndef MITKIMAGETODIFFUSIONIMAGESOURCE_H
#define MITKIMAGETODIFFUSIONIMAGESOURCE_H

#include "mitkDiffusionImageSource.h"

namespace mitk
{
/**
 * @class ImageToDiffusionImageSource
 * @brief The class requires a 3d+t image, list of gradient directions and a corresponding b-value
 * and reinterprets the input image as a diffusion-weighted image.
 *
 * The filter throws an mitk::Exception if
 *  - no gradient list was set
 *  - no input image was set
 *  - the size of the gradient list and the count of timesteps do not match
 */
template< typename TPixelType >
class ImageToDiffusionImageSource
    : public DiffusionImageSource< TPixelType >
{
public:

  typedef vnl_vector_fixed< double, 3 >      GradientDirectionType;
  typedef vnl_matrix_fixed< double, 3, 3 >      MeasurementFrameType;
  typedef itk::VectorContainer< unsigned int,
    GradientDirectionType >::Pointer          GradientDirectionContainerType;

  mitkClassMacro( ImageToDiffusionImageSource,
                  DiffusionImageSource<TPixelType> )

  itkSimpleNewMacro(Self)

  typedef typename Superclass::OutputType OutputType;
  typedef itk::VectorImage<TPixelType,3>     ImageType;

  /**
   * @brief Set the mitk image ( a 3d+t image ) which is to be reinterpreted as dw image
   * @param mitkImage
   */
  void SetImage( mitk::Image::Pointer mitkImage )
  {
    m_SourceImage = mitkImage;
  }

  /**
   * @brief Set the gradient directions corresponding to the volumes of the input image
   * @param container the container with the gradient directions
   *
   * @note The gradient directions must correspond to the volumes stored in the 3d+t input
   */
  void SetGradientDirections( GradientDirectionContainerType container )
  {
    m_GradientDirections = container;
  }

  /**
   * @brief Set the b-value
   * @param bvalue the b-value associated with the gradient list
   */
  void SetBValue( float bvalue )
  {
    this->m_BValue = bvalue;
  }


protected:
  ImageToDiffusionImageSource();
  virtual ~ImageToDiffusionImageSource(){}

  virtual void GenerateData();

  virtual void GenerateOutputInformation();

  mitk::Image::Pointer m_SourceImage;
  GradientDirectionContainerType m_GradientDirections;

  typename OutputType::Pointer m_OutputCache;
  itk::TimeStamp m_CacheTime;

  float m_BValue;
};


}

#include "mitkImageToDiffusionImageSource.cpp"
#endif // MITKIMAGETODIFFUSIONIMAGESOURCE_H
