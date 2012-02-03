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

#ifndef mitkOverwriteSliceFilter_h_Included
#define mitkOverwriteSliceFilter_h_Included


#include "MitkExports.h"
#include "mitkImageToImageFilter.h"
#include <vtkSmartPointer.h>


#include <vtkImageData.h>


namespace mitk
{
	
	class MITK_CORE_EXPORT OverwriteSliceFilter : public ImageToImageFilter
	{
	public:

		mitkClassMacro(OverwriteSliceFilter, ImageToImageFilter);
		itkNewMacro(OverwriteSliceFilter);

		/** \brief Set the time step in the 4D volume */
		void SetTimeStep( unsigned int timestep){ this->m_TimeStep = timestep; }
		unsigned int GetTimeStep(){ return this->m_TimeStep; }

		void SetInputSlice(mitk::Image* slice){ this->m_Slice = slice; }

		void SetInputMap(vtkImageData* map){ this->m_Map = map; }
		void SetInputMap(mitk::Image* map);

	protected:
		OverwriteSliceFilter();
		virtual ~OverwriteSliceFilter();

		virtual void GenerateData();

		virtual void GenerateInputRequestedRegion();

		mitk::Image* m_Slice;
		vtkSmartPointer<vtkImageData> m_Map;

		unsigned int m_TimeStep;

			
		};
}

#endif // mitkOverwriteSliceFilter_h_Included