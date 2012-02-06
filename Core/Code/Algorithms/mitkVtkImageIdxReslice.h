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

#ifndef mitkVtkImageIdxReslice_h_Included
#define mitkVtkImageIdxReslice_h_Included

#include <vtkImageReslice.h>
#include "MitkExports.h"
#include <cstdlib>

	/** \brief A Filter based on vtkImageReslice that extracts Coordinates in the 3D volume not pixel values.
	 * 
	 */
	class MITK_CORE_EXPORT mitkVtkImageIdxReslice : public vtkImageReslice
	{
	public:
		static mitkVtkImageIdxReslice *New();
		vtkTypeMacro(mitkVtkImageIdxReslice, vtkImageReslice);
		
		//std::vector<unsigned int*> GetPointerShifts(){ return this->pointerShift;}

	protected:
		
		mitkVtkImageIdxReslice();
		virtual ~mitkVtkImageIdxReslice();

		virtual void ThreadedRequestData(vtkInformation *vtkNotUsed(request),
 																		 vtkInformationVector **vtkNotUsed(inputVector),
 																		 vtkInformationVector *vtkNotUsed(outputVector),
 																		 vtkImageData ***inData,
 																		 vtkImageData **outData,
 																		 int outExt[6], int id);
								
		//std::vector<unsigned int*> m_PointerShifts;
	};


#endif //mitkVtkImageIdxReslice_h_Included
