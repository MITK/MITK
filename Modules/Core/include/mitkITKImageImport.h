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


#ifndef MITKITKIMAGEIMPORT_H_HEADER_INCLUDED_C1E4861D
#define MITKITKIMAGEIMPORT_H_HEADER_INCLUDED_C1E4861D

#include <MitkCoreExports.h>
#include "mitkImageSource.h"
#include "itkImageToImageFilterDetail.h"

namespace mitk {

/**
* @brief Pipelined import of itk::Image
*
* The image data contained in the itk::Image is referenced,
* not copied.
* The easiest way of use is by the function
* mitk::ImportItkImage
* \code
* mitkImage = mitk::ImportItkImage(itkImage);
* \endcode
* \sa ImportItkImage
* @ingroup Adaptor
*/
template <class TInputImage>
class MITK_EXPORT ITKImageImport : public ImageSource
{
public:

  mitkClassMacro(ITKImageImport,ImageSource)
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  ///  \brief The type of the input image.
  typedef TInputImage InputImageType;
  typedef typename InputImageType::Pointer        InputImagePointer;
  typedef typename InputImageType::ConstPointer   InputImageConstPointer;
  typedef typename InputImageType::RegionType     InputImageRegionType;
  typedef typename InputImageType::PixelType      InputImagePixelType;

  /** ImageDimension constants */
  itkStaticConstMacro(InputImageDimension, unsigned int,
                      TInputImage::ImageDimension);
  itkStaticConstMacro(RegionDimension, unsigned int,
                      mitk::SlicedData::RegionDimension);

  /** \brief Set the input itk::Image of this image importer.  */
  InputImageType * GetInput(void);

  /** \brief Set the input itk::Image of this image importer. */
  void SetInput(const InputImageType*);
  using itk::ProcessObject::SetInput;

  /**
  * \brief Set the Geometry of the result image (optional)
  *
  * The Geometry has to fit the dimension and size of
  * the input image. The Geometry will be cloned, not
  * referenced!
  *
  * Providing the Geometry is optional.
  * The default behavior is to set the geometry by
  * the itk::Image::GetDirection() information.
  */
  void SetGeometry(const BaseGeometry* geometry);

protected:
  ITKImageImport();

  virtual ~ITKImageImport();

  virtual void GenerateOutputInformation();

  virtual void GenerateInputRequestedRegion();

  virtual void GenerateData();

  virtual void SetNthOutput(DataObjectPointerArraySizeType num, itk::DataObject *output);

  /** Typedef for the region copier function object that converts an
   * output region to an input region. */
  typedef itk::ImageToImageFilterDetail::ImageRegionCopier<itkGetStaticConstMacro(InputImageDimension),
  itkGetStaticConstMacro(RegionDimension)> OutputToInputRegionCopierType;

  BaseGeometry::Pointer m_Geometry;
};

/**
* @brief Imports an itk::Image (with a specific type) as an mitk::Image.
* @ingroup Adaptor
*
* Instantiates instance of ITKImageImport.
* mitk::ITKImageImport does not cast pixel types etc., it just imports
* image data. If you get a compile error, try image.GetPointer().
*
* \param update: if \a true, fill mitk::Image, which will execute the
* up-stream pipeline connected to the input itk::Image. Otherwise you
* need to make sure that Update() is called on the mitk::Image before
* its data is being used, e.g., by connecting it to an mitk-pipeline
* and call Update of a downstream filter at some time.
* \sa itk::Image::CastToMitkImage
*/
template <typename ItkOutputImageType>
Image::Pointer ImportItkImage(const itk::SmartPointer<ItkOutputImageType>& itkimage, const BaseGeometry* geometry = NULL, bool update = true);

/**
* @brief Imports an itk::Image (with a specific type) as an mitk::Image.
* @ingroup Adaptor
*
* Instantiates instance of ITKImageImport
* mitk::ITKImageImport does not cast pixel types etc., it just imports
* image data. If you get a compile error, try image.GetPointer().
*
* \param update: if \a true, fill mitk::Image, which will execute the
* up-stream pipeline connected to the input itk::Image. Otherwise you
* need to make sure that Update() is called on the mitk::Image before
* its data is being used, e.g., by connecting it to an mitk-pipeline
* and call Update of a downstream filter at some time.
*
*
* \note If the source (itk image) and the target (mitk image) do not share the same scope, the mitk::GrabItkImageMemory function
*  has to be used instead. Otherwise the image memory managed by the itk image is lost at a scope level change. This affects especially the
* usage in combination with AccessByItk macros as in following example code
*
* \snippet Testing/mitkGrabItkImageMemoryTest.cpp OutOfScopeCall
*
* which calls an ITK-like filter
*
* \snippet Testing/mitkGrabItkImageMemoryTest.cpp ItkThresholdFilter
*
*
* \sa itk::Image::CastToMitkImage
* \sa GrabItkImageMemory
*/
template <typename ItkOutputImageType>
Image::Pointer ImportItkImage(const ItkOutputImageType* itkimage, const BaseGeometry* geometry = NULL, bool update = true);

/**
* @brief Grabs the memory of an itk::Image (with a specific type)
* and puts it into an mitk::Image.
* @ingroup Adaptor
*
* The memory is managed by the mitk::Image after calling this
* function. The itk::Image remains valid until the mitk::Image
* decides to free the memory.
* \param update: if \a true, fill mitk::Image, which will execute the
* up-stream pipeline connected to the input itk::Image. Otherwise you
* need to make sure that Update() is called on the mitk::Image before
* its data is being used, e.g., by connecting it to an mitk-pipeline
* and call Update of a downstream filter at some time.
* \sa ImportItkImage
*/
template <typename ItkOutputImageType>
Image::Pointer GrabItkImageMemory(itk::SmartPointer<ItkOutputImageType>& itkimage, mitk::Image* mitkImage = NULL, const BaseGeometry* geometry = NULL, bool update = true);

/**
* @brief Grabs the memory of an itk::Image (with a specific type)
* and puts it into an mitk::Image.
* @ingroup Adaptor
*
* The memory is managed by the mitk::Image after calling this
* function. The itk::Image remains valid until the mitk::Image
* decides to free the memory.
* \param update: if \a true, fill mitk::Image, which will execute the
* up-stream pipeline connected to the input itk::Image. Otherwise you
* need to make sure that Update() is called on the mitk::Image before
* its data is being used, e.g., by connecting it to an mitk-pipeline
* and call Update of a downstream filter at some time.
* \sa ImportItkImage
*/
template <typename ItkOutputImageType>
Image::Pointer GrabItkImageMemory(ItkOutputImageType* itkimage, mitk::Image* mitkImage = NULL, const BaseGeometry* geometry = NULL, bool update = true);

} // namespace mitk

#ifndef MITK_MANUAL_INSTANTIATION
#include "mitkITKImageImport.txx"
#endif

#endif /* MITKITKIMAGEIMPORT_H_HEADER_INCLUDED_C1E4861D */
