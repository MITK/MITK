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

#ifndef HISTOGRAM_GENERATOR_H_HEADER_INCLUDED
#define HISTOGRAM_GENERATOR_H_HEADER_INCLUDED

#include "mitkCommon.h"

namespace mitk {

//##Documentation
//## @brief Provides an easy way to calculate an itk::Histogram for a mitk::Image
//##
class HistogramGenerator : public itk::Object
{
public:
	mitkClassMacro(HistogramGenerator,itk::Object);

	itkNewMacro(Self);  

protected:
	HistogramGenerator();

	virtual ~HistogramGenerator();

};

} // namespace mitk

#endif /* HISTOGRAM_GENERATOR_H_HEADER_INCLUDED */
