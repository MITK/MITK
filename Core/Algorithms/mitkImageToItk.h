/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef IMAGETOITK_H_HEADER_INCLUDED_C1C2FCD2
#define IMAGETOITK_H_HEADER_INCLUDED_C1C2FCD2

#if(_MSC_VER==1200)
#include <itkFixedCenterOfRotationAffineTransform.h>
#endif

#include <itkImage.h>
#include <itkImageSource.h>
#include "mitkImage.h"
#include "mitkImageDataItem.h"

namespace mitk
{
/**
 * Create itk::ImageSource for mitk::Image
 * \ingroup Adaptor
 */
template <class TOutputImage>
class ITK_EXPORT ImageToItk : public itk::ImageSource< TOutputImage >
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
  
	const mitk::Image * GetInput(void);
	const mitk::Image * GetInput(unsigned int idx);

  virtual void SetInput(const mitk::Image *input);
  virtual void SetInput(unsigned int index, const mitk::Image * image);

  virtual void UpdateOutputInformation();

  itkGetMacro( Channel, int );
  itkSetMacro( Channel, int );

  itkSetMacro( CopyMemFlag, bool );
  itkGetMacro( CopyMemFlag, bool );
  itkBooleanMacro( CopyMemFlag );

protected:
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

