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

#if(_MSC_VER==1200)
#include <itkFixedCenterOfRotationAffineTransform.h>
#endif

#include <itkImage.h>
#include <itkImageSource.h>
#include "mitkImage.h"
#include "mitkImageDataItem.h"
#include "mitkImageWriteAccessor.h"

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
class ImageToItk : public itk::ImageSource< TOutputImage >
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
  itkNewMacro(Self);

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

  virtual void SetInput(mitk::Image *input);
  virtual void SetInput(unsigned int index, mitk::Image * image);

  virtual void UpdateOutputInformation();

  itkGetMacro( Channel, int );
  itkSetMacro( Channel, int );

  itkSetMacro( CopyMemFlag, bool );
  itkGetMacro( CopyMemFlag, bool );
  itkBooleanMacro( CopyMemFlag );

protected:
  using itk::ProcessObject::SetInput;
  mitk::Image * GetInput(void);
  mitk::Image * GetInput(unsigned int idx);

  ImageToItk(): m_CopyMemFlag(false), m_Channel(0)
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

  //ImageToItk(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};


} // end namespace mitk



#ifndef ITK_MANUAL_INSTANTIATION
#include "mitkImageToItk.txx"
#endif

#endif // IMAGETOITK_H_HEADER_INCLUDED_C1C2FCD2

