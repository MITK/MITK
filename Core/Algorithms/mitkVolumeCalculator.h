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

#ifndef VOLUME_CALCULATOR_H_HEADER_INCLUDED
#define VOLUME_CALCULATOR_H_HEADER_INCLUDED

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkImage.h"

#include "mitkCommon.h"
#include "mitkImage.h"

namespace mitk {

//##Documentation
//## @brief Provides an easy way to calculate the volume of a mitk::Image
//##
class VolumeCalculator : public itk::Object
{
public:
	mitkClassMacro(VolumeCalculator,itk::Object);

	itkNewMacro(Self);  
	itkSetMacro(Image,mitk::Image::ConstPointer);
	itkSetMacro(Threshold,int);
	// TODO: calculate if needed in GetHistogram()
	void ComputeVolume();
	itkGetMacro(Volume,float);
	
protected:
	VolumeCalculator();

	virtual ~VolumeCalculator();
	
	template < typename TPixel, unsigned int VImageDimension > 
        void InternalCompute(itk::Image< TPixel, VImageDimension >* itkImage);
        
	mitk::Image::ConstPointer m_Image;
	int m_Threshold;
	float m_Volume;
};

} // namespace mitk

#endif /* VOLUME_CALCULATOR_H_HEADER_INCLUDED */
