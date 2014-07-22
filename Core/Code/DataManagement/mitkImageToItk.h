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

/// Note:
/// We need helper types to check type constness. The ImageToItk class acquires read or
/// write lock depending whether the output type is const or not.
#if __cplusplus > 199711L

#include <type_traits>

#else

namespace std
{

template <typename T, T v>
struct integral_constant
{
  static const T value = v;
  typedef T value_type;
  typedef integral_constant<T, v> type;
};

typedef integral_constant<bool, true> true_type;
typedef integral_constant<bool, false> false_type;

template<typename T, T v>
const T std::integral_constant<T, v>::value;

template<typename T> struct is_const : std::false_type {};
template<typename T> struct is_const<T const> : std::true_type {};

template<typename T> struct remove_const
{
  typedef T type;
};

template<typename T> struct remove_const<T const>
{
  typedef T type;
};

}

#endif


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
class ImageToItk : public itk::ImageSource< typename std::remove_const<TOutputImage>::type >
{
protected:
  mitk::Image::Pointer m_MitkImage;
  mitk::ImageDataItem::Pointer m_ImageDataItem;

public:
  typedef ImageToItk  Self;
  typedef itk::ImageSource<typename std::remove_const<TOutputImage>::type>  Superclass;
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

  virtual void SetInput(mitk::Image *input);
  virtual void SetInput(unsigned int index, mitk::Image * image);

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
  mitk::Image * GetInput(unsigned int idx);

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

  //ImageToItk(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};


} // end namespace mitk



#ifndef ITK_MANUAL_INSTANTIATION
#include "mitkImageToItk.txx"
#endif

#endif // IMAGETOITK_H_HEADER_INCLUDED_C1C2FCD2

