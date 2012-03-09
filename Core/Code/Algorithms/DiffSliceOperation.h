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

#ifndef mitkDiffSliceOperation_h_Included
#define mitkDiffSliceOperation_h_Included

#include "MitkExports.h"
#include "mitkCommon.h"
#include <mitkOperation.h>
//#include "mitkCompressedImageContainer.h"

#include <mitkImage.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>


namespace mitk
{
		class MITK_CORE_EXPORT DiffSliceOperation : public Operation
	{
	public:
		mitkClassMacro(DiffSliceOperation, OperationActor);
		//itkNewMacro(DiffSliceOperation);

		//mitkNewMacro4Param(DiffSliceOperation,mitk::Image,mitk::Image,unsigned int, mitk::Geometry2D);

		DiffSliceOperation();
		DiffSliceOperation( mitk::Image* imageVolume, vtkImageData* slice, unsigned int timestep, AffineGeometryFrame3D* currentWorldGeometry);

		bool IsValid();

		mitk::Image* GetImage(){return this->m_Image;}

		vtkImageData* GetSlice();

		unsigned int GetTimeStep(){return this->m_TimeStep;}

		AffineGeometryFrame3D* GetWorldGeometry(){return this->m_WorldGeometry;}


	protected:
		
		virtual ~DiffSliceOperation();

		void OnImageDeleted();


		//CompressedImageContainer::Pointer m_zlibSliceContainer;
		mitk::Image* m_Image;
		vtkSmartPointer<vtkImageData> m_Slice;
		unsigned int m_TimeStep;
		AffineGeometryFrame3D::Pointer m_WorldGeometry;
		bool m_ImageIsValid;

		unsigned long m_DeleteTag;

	};
}
#endif