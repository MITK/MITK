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

#ifndef mitkVtkImageMapReslice_h_Included
#define mitkVtkImageMapReslice_h_Included

#include <vtkImageReslice.h>
#include "MitkExports.h"

	/** \brief A Filter based on vtkImageReslice that extracts Coordinates in the 3D volume not pixel values.
	 * 
	 */
	class MITK_CORE_EXPORT mitkVtkImageMapReslice : public vtkImageReslice
	{
	public:

		static mitkVtkImageMapReslice *New();
		vtkTypeMacro(mitkVtkImageMapReslice, vtkImageReslice);

		virtual void SetOutputExtent(int ex[]);
		virtual void SetOutputExtent(int xmin, int xmax,int ymin,int ymax,int zmin,int zmax);

		virtual unsigned int* GetMap(){ return this->m_Map;}

		
	protected:
		
		mitkVtkImageMapReslice();
		virtual ~mitkVtkImageMapReslice();

		virtual void ThreadedRequestData(vtkInformation *vtkNotUsed(request),
 																		 vtkInformationVector **vtkNotUsed(inputVector),
 																		 vtkInformationVector *vtkNotUsed(outputVector),
 																		 vtkImageData ***inData,
 																		 vtkImageData **outData,
 																		 int outExt[6], int id);

		unsigned int* m_Map;
	};


#endif //mitkVtkImageMapReslice_h_Included
