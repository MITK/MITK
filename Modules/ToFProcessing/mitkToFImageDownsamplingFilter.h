/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkToFImageDownsamplingFilter_h
#define mitkToFImageDownsamplingFilter_h

//MITK includes
#include <mitkImage.h>
#include "mitkImageToImageFilter.h"
#include <MitkToFProcessingExports.h>
// ITK includes
#include "itkImage.h"

namespace mitk
{
  /**
  * @brief Reduces the resolution of a ToF distance image. Although it is meant to be used for ToF distance images, it should work
  * for any 2D or 3D images. The dimensions (in pixels) of the desired image are taken as input parameters, and an image with these
  * specified dimensions is created.
  *
  * @ingroup SurfaceFilters
  * @ingroup ToFProcessing
  */

  class MITKTOFPROCESSING_EXPORT ToFImageDownsamplingFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(ToFImageDownsamplingFilter, ImageToImageFilter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    itkSetMacro(ResampledX,double);
    itkGetMacro(ResampledX,double);

    itkSetMacro(ResampledY,double);
    itkGetMacro(ResampledY,double);

    itkSetMacro(ResampledZ,double);
    itkGetMacro(ResampledZ,double);


  protected:
    /*!
    \brief Standard constructor
    */
    ToFImageDownsamplingFilter();

    /*!
    \brief Standard destructor
    */
    ~ToFImageDownsamplingFilter() override;

     /*!
    \brief Method generating the output of this filter. Called in the updated process of the pipeline.
    This method calls the AccessFixedDimensionByItk class with the ItkImageResampling function below
    */
    void GenerateData() override;

     /*!
    \brief Templated method for ITK image type which performs the resampling with an itk filter.
    \param itkImage is the input to the filter converted to ITK format
    */
    template<typename TPixel, unsigned int VImageDimension>
    void ItkImageResampling( const itk::Image<TPixel,VImageDimension>* itkImage );

    double m_ResampledX;///<  length of x dimension of output image in pixels
    double m_ResampledY;///<  length of y dimension of output image in pixels
    double m_ResampledZ;///<  length of z dimension of output image in pixels (if 2D, default is set to 1)

};
}// end namespace mitk
#endif
