/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkExtractSliceFilter_h_Included
#define mitkExtractSliceFilter_h_Included


#include "ImageExtractionExports.h"
#include "mitkImageToImageFilter.h"
#include <vtkSmartPointer.h>

#include "vtkImageReslice.h"

namespace mitk
{
	/**
	*	\brief ExtractSliceFilter extracts a 2D abitrary oriented slice from a 3D volume.
	*	
	*/
	class ImageExtraction_EXPORT ExtractSliceFilter : public ImageToImageFilter
	{
	public:

		mitkClassMacro(ExtractSliceFilter, ImageToImageFilter);
		itkNewMacro(ExtractSliceFilter);

		void SetWorldGeometry(const Geometry2D* geometry ){ this->m_WorldGeometry = geometry; };

		void SetTimeStep( unsigned int timestep){ this->m_TimeStep = timestep; }
		unsigned int GetTimeStep(){ return this->m_TimeStep; }

	protected:
		ExtractSliceFilter();
		virtual ~ExtractSliceFilter();

		virtual void GenerateData();

		const Geometry2D* m_WorldGeometry;
		vtkSmartPointer<vtkImageReslice> m_Reslicer;

		unsigned int m_TimeStep;

	};
}

#endif // mitkExtractSliceFilter_h_Included