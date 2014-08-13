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


#ifndef MITKIMAGECAST_H_HEADER_INCLUDED
#define MITKIMAGECAST_H_HEADER_INCLUDED

#include <MitkCoreExports.h>
#include <mitkImage.h>

#include <itkImage.h>
#include <itkVectorImage.h>

namespace mitk
{

#ifndef DOXYGEN_SKIP
  template < typename TPixel, unsigned int VImageDimension, class ItkOutputImageType >
  void _CastToItkImage2Access(const itk::Image<TPixel, VImageDimension>* itkInputImage, itk::SmartPointer<ItkOutputImageType>& itkOutputImage);

  template < typename TPixel, unsigned int VImageDimension, class ItkOutputImageType >
  void _CastToItkImage2Access(const itk::VectorImage<TPixel, VImageDimension>* itkInputImage, itk::SmartPointer<ItkOutputImageType>& itkOutputImage);
#endif //DOXYGEN_SKIP

 /**
  * @brief Cast an mitk::Image to an itk::Image with a specific type.
  *
  * This method casts the pixel types and copies the image memory if necessary. If
  * the requested pixel type is equal to the pixel type of the MITK image, the
  * memory is just referenced.
  *
  * Usually, you want the ITK image to reference the same image data as the MITK image
  * without any casting. In this case use mitk::ImageToItkImage, which generate less
  * code and is more efficient.
  *
  * @param mitkImage The MITK image to be cast to an ITK image
  * @param itkOutputImage You don't have to initialize the itk::Image<..>::Pointer.
  *
  * @sa mitk::ImageToItkImage
  *
  * @ingroup Adaptor
  */
 template <typename ItkOutputImageType> extern void MITK_CORE_EXPORT CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<ItkOutputImageType>& itkOutputImage);

  /**
   * @brief Cast an mitk::Image to an itk::VectorImage with a specific type.
   *
   * You don't have to initialize the itk::VectorImage<..>::Pointer.
   * @ingroup Adaptor
   */
  template <typename TPixelType, unsigned int VImageDimension>
  extern void MITK_CORE_EXPORT CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<itk::VectorImage<TPixelType,VImageDimension> >& itkOutputImage);

  /**
   * @brief Cast an itk::Image (with a specific type) to an mitk::Image.
   *
   * CastToMitkImage does not cast pixel types etc., just image data
   * @ingroup Adaptor
   * \sa mitkITKImageImport
   */
  template <typename ItkOutputImageType>
  void CastToMitkImage(const itk::SmartPointer<ItkOutputImageType>& itkimage, itk::SmartPointer<mitk::Image>& mitkoutputimage)
  {
    CastToMitkImage(itkimage.GetPointer(), mitkoutputimage);
  }

  /**
   * @brief Cast an itk::Image (with a specific type) to an mitk::Image.
   *
   * CastToMitkImage does not cast pixel types etc., just image data
   * @ingroup Adaptor
   * \sa mitkITKImageImport
   */
  template <typename ItkOutputImageType>
  void CastToMitkImage(const ItkOutputImageType* itkimage, itk::SmartPointer<mitk::Image>& mitkoutputimage)
  {
    if(mitkoutputimage.IsNull())
    {
      mitkoutputimage = mitk::Image::New();
    }
    mitkoutputimage->InitializeByItk(itkimage);
    mitkoutputimage->SetChannel(itkimage->GetBufferPointer());
  }
}

#endif // of MITKIMAGECAST_H_HEADER_INCLUDED
