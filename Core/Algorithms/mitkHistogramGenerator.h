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

#ifndef HISTOGRAM_GENERATOR_H_HEADER_INCLUDED
#define HISTOGRAM_GENERATOR_H_HEADER_INCLUDED

#include <itkObject.h>
#include <itkHistogram.h>
#include <itkImage.h>
#include "mitkImage.h"

namespace mitk {

//##Documentation
//## @brief Provides an easy way to calculate an itk::Histogram for a mitk::Image
//##
class HistogramGenerator : public itk::Object
{
public:
	mitkClassMacro(HistogramGenerator,itk::Object);

	itkNewMacro(Self);  
	typedef itk::Statistics::Histogram<double> HistogramType;
        
	itkSetMacro(Image,mitk::Image::ConstPointer);
	itkSetMacro(Size,int);
	itkGetConstMacro(Size,int);
	itkGetConstObjectMacro(Histogram,HistogramType);
	
	// TODO: calculate if needed in GetHistogram()
	void ComputeHistogram();
	float GetMaximumFrequency() const;
	static float CalculateMaximumFrequency(const HistogramType* histogram);
protected:
	HistogramGenerator();

	virtual ~HistogramGenerator();
       
	mitk::Image::ConstPointer m_Image;
	int m_Size;
	HistogramType::ConstPointer m_Histogram;
};

} // namespace mitk

#endif /* HISTOGRAM_GENERATOR_H_HEADER_INCLUDED */
