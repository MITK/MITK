/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 7704 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef UNSTRUCTURED_GIRD_HISTOGRAM_H_HEADER_INCLUDED
#define UNSTRUCTURED_GIRD_HISTOGRAM_H_HEADER_INCLUDED

#include <itkHistogram.h>
#include "mitkUnstructuredGrid.h"

namespace mitk {

//##Documentation
//## @brief Subclass of itk::Histogram, specialized for a mitk::UnstructuredGrid
//##
class UnstructuredGridHistogram : public itk::Statistics::Histogram<double>
{
public:
	mitkClassMacro(UnstructuredGridHistogram,itk::Statistics::Histogram<double>);

	itkNewMacro(Self);
	
	void Initialize(mitk::UnstructuredGrid*);
	
	
protected:
	UnstructuredGridHistogram();

	virtual ~UnstructuredGridHistogram();
};

} // namespace mitk

#endif /* UNSTRUCTURED_GIRD_HISTOGRAM_H_HEADER_INCLUDED */
