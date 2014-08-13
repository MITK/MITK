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


#ifndef IMAGETOITK_H_HEADER_INCLUDED_C1C2FCD2
#define IMAGETOITK_H_HEADER_INCLUDED_C1C2FCD2

#include "mitkImage.h"
#include "mitkImageDataItem.h"
#include "mitkImageWriteAccessor.h"

#include <itkImage.h>
#include <itkImageSource.h>


namespace mitk
{

/**
 * Create itk::ImageSource for mitk::Image
 * \ingroup Adaptor
 *
 * \warning 2D MITK images will get a 2D identity matrix in ITK
 * \todo Get clear about how to handle directed ITK 2D images in ITK
 */
template <class TOutputImage>
class ImageToItk : public itk::ImageSource<TOutputImage>
{
protected:
  mitk::Image::Pointer m_MitkImage;
  mitk::ImageDataItem::Pointer m_ImageDataItem;

public:
  typedef ImageToItk  Self;
  typedef itk::ImageSource<TOutputImage>  Superclass;
  typedef itk::SmartPointer<Self>  Pointer;
  typedef itk::SmartPointer<const Self>  ConstPointer;

  /** Method for creation through the object factory. */
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /** Superclass typedefs. */
  typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

  /** Some convenient typedefs. */
  typedef mitk::Image                    InputImageType;
  typedef InputImageType::Pointer        InputImagePointer;
  typedef InputImageType::ConstPointer   InputImageConstPointer;
  typedef SlicedData::RegionType         InputImageRegionType;
  typedef typename TOutputImage::SizeType         SizeType;
  typedef typename TOutputImage::IndexType        IndexType;
  typedef typename TOutputImage::RegionType       RegionType;
  typedef typename TOutputImage::PixelType        PixelType;
  typedef typename TOutputImage::InternalPixelType InternalPixelType;
  typedef typename TOutputImage::PixelContainer PixelContainer;

  virtual void SetInput(mitk::Image *input);
  virtual void SetInput(const mitk::Image* input);
  //virtual void SetInput(unsigned int index, mitk::Image * image);
  //virtual void SetInput(unsigned int index, const mitk::Image * image);

  virtual void UpdateOutputInformation();

  itkGetMacro( Channel, int );
  itkSetMacro( Channel, int );

  itkSetMacro( CopyMemFlag, bool );
  itkGetMacro( CopyMemFlag, bool );
  itkBooleanMacro( CopyMemFlag );

  itkSetMacro( Options, int );
  itkGetMacro( Options, int );

protected:
  using itk::ProcessObject::SetInput;
  mitk::Image * GetInput(void);
  const mitk::Image* GetInput() const;

  ImageToItk()
  : m_CopyMemFlag(false)
  , m_Channel(0)
  , m_Options(mitk::ImageAccessorBase::DefaultBehavior)
  {
  }

  virtual ~ImageToItk()
  {
  }

  void PrintSelf(std::ostream& os, itk::Indent indent) const;


  virtual void GenerateData();

  virtual void GenerateOutputInformation();

private:
  bool m_CopyMemFlag;
  int m_Channel;
  int m_Options;

  bool m_ConstInput;

  //ImageToItk(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  void CheckInput(const mitk::Image* image) const;
};

/**
 * @brief Convert a MITK image to an ITK image.
 *
 * This method creates a itk::Image representation for the given MITK
 * image, referencing the MITK image memory. If the provided template
 * arguments do not match the type of the MITK image, an exception is thrown.
 *
 * The MITK image is locked for read/write access as long as the returned
 * itk::Image object exists. See ImageToItkImage(const mitk::Image*) for
 * read-only access.
 *
 * @tparam TPixel The pixel type of the ITK image
 * @tparam VDimension The image dimension of the ITK image
 * @param mitkImage The MITK image which is to be converted to an ITK image
 * @return An ITK image representation for the given MITK image
 * @throws mitk::Exception if the pixel type or dimension does not match
 *         the MITK image type or if the MITK image is already locked for
 *         read or write access.
 *
 * @sa ImageToItkImage(const mitk::Image*)
 * @sa CastToItkImage
 *
 * @ingroup Adaptor
 */
template<typename TPixel, unsigned int VDimension>
typename ImageTypeTrait<TPixel, VDimension>::ImageType::Pointer ImageToItkImage(mitk::Image* mitkImage)
{
  typedef typename ImageTypeTrait<TPixel, VDimension>::ImageType ImageType;
  typedef mitk::ImageToItk<ImageType> ImageToItkType;
  itk::SmartPointer<ImageToItkType> imagetoitk = ImageToItkType::New();
  imagetoitk->SetInput(mitkImage);
  imagetoitk->Update();
  return imagetoitk->GetOutput();
}

/**
 * @brief Convert a MITK image to an ITK image.
 *
 * This method creates a itk::Image representation for the given MITK
 * image, referencing the MITK image memory. If the provided template
 * arguments do not match the type of the MITK image, an exception is thrown.
 *
 * The MITK image is locked for read access as long as the returned
 * itk::Image object exists. See ImageToItkImage(mitk::Image*) for
 * read and write access.
 *
 * @tparam TPixel The pixel type of the ITK image
 * @tparam VDimension The image dimension of the ITK image
 * @param mitkImage The MITK image which is to be converted to an ITK image
 * @return An ITK image representation for the given MITK image
 * @throws mitk::Exception if the pixel type or dimension does not match
 *         the MITK image type or if the MITK image is already locked for
 *         write access.
 *
 * @sa ImageToItkImage(mitk::Image*)
 * @sa CastToItkImage
 *
 * @ingroup Adaptor
 */
template<typename TPixel, unsigned int VDimension>
typename ImageTypeTrait<TPixel, VDimension>::ImageType::ConstPointer ImageToItkImage(const mitk::Image* mitkImage)
{
  typedef typename ImageTypeTrait<TPixel, VDimension>::ImageType ImageType;
  typedef mitk::ImageToItk<ImageType> ImageToItkType;
  itk::SmartPointer<ImageToItkType> imagetoitk = ImageToItkType::New();
  imagetoitk->SetInput(mitkImage);
  imagetoitk->Update();
  return imagetoitk->GetOutput();
}

} // end namespace mitk



#ifndef ITK_MANUAL_INSTANTIATION
#include "mitkImageToItk.txx"
#endif

#endif // IMAGETOITK_H_HEADER_INCLUDED_C1C2FCD2

