/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#ifndef ITKIMAGETOIMAGEFILTERADAPTER_H_HEADER_INCLUDED_C1E5E869
#define ITKIMAGETOIMAGEFILTERADAPTER_H_HEADER_INCLUDED_C1E5E869

#include "mitkCommon.h"
#include "mitkImageToImageFilter.h"

#include "itkImage.h"
#include "itkImageToImageFilter.h"

namespace mitk {

//##Documentation
//## @brief adapter class, that encapsulates any itk::ImageToImageFilter
//##
//## This filter converts a mitk::Image to an itk::Image, executes the 
//## itkFilter and converts the itk::Image back to a mitk::Image as output.
//## Either call SetSingleFilter() to encapsulate one itk::ImageToImageFilter, 
//## or SetFirstFilter() and SetLastFilter() to encapsulate a whole itk pipeline.
//## @ingroup Process
template <typename TPixel> 
class ItkImageToImageFilterAdapter : public ImageToImageFilter
{
public:
  typedef TPixel PixelType;
  typedef itk::Image<PixelType, 3> ItkImageType;
  typedef itk::ImageToImageFilter<ItkImageType, ItkImageType> ImageToImageFilterType;

	mitkClassMacro(ItkImageToImageFilterAdapter,ImageSource);
	/** Method for creation through the object factory. */
	itkNewMacro(Self);

  itkSetObjectMacro(FirstFilter, ImageToImageFilterType);
  itkGetObjectMacro(FirstFilter, ImageToImageFilterType);
  itkSetObjectMacro(LastFilter, ImageToImageFilterType);
  itkGetObjectMacro(LastFilter, ImageToImageFilterType);
  void SetSingleFilter(typename ImageToImageFilterType::Pointer filter);

  virtual void GenerateOutputInformation();
  virtual void GenerateData();

protected:
	ItkImageToImageFilterAdapter();
	virtual ~ItkImageToImageFilterAdapter();
 
  typename ImageToImageFilterType::Pointer m_FirstFilter;  // Start of Filter Pipeline
  typename ImageToImageFilterType::Pointer m_LastFilter;   // End of Filter Pipeline
};
} // namespace mitk

#include "mitkItkImageToImageFilterAdapter.txx"  // because it is a template

#endif /* ITKIMAGETOIMAGEFILTERADAPTER_H_HEADER_INCLUDED_C1E5E869 */
