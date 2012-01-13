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


#include "MitkExports.h"
#include "mitkImageToImageFilter.h"
#include <vtkSmartPointer.h>

#include <vtkImageReslice.h>
#include <vtkMatrix4x4.h>
#include <vtkImageData.h>
#include <vtkPoints.h>
#include <vtkTransform.h>
#include <vtkAbstractTransform.h>

namespace mitk
{
	/**
	*	\brief ExtractSliceFilter extracts a 2D abitrary oriented slice from a 3D volume.
	*	
	*/
	class MITK_CORE_EXPORT ExtractSliceFilter : public ImageToImageFilter
	{
	public:

		mitkClassMacro(ExtractSliceFilter, ImageToImageFilter);
		itkNewMacro(ExtractSliceFilter);

		int* GetPlaneBounds(){ return this->m_Reslicer->GetOutput()->GetWholeExtent();}

		void SetWorldGeometry(const Geometry2D* geometry ){ this->m_WorldGeometry = geometry; this->Modified(); }

		
		void SetTimeStep( unsigned int timestep){ this->m_TimeStep = timestep; }
		unsigned int GetTimeStep(){ return this->m_TimeStep; }

		/**
		* \brief Set a transform for the reslice axes.
		*
		* This transform is needed if the image volume itself is transformed.
		*/
		void SetResliceTransformByGeometry(const Geometry3D* transform){ this->m_ResliceTransform = transform; }


		void SetInPlaneResampleExtentByGeometry(bool inPlaneResampleExtentByGeometry){ this->m_InPlaneResampleExtentByGeometry = inPlaneResampleExtentByGeometry; }

		bool GetBounds(vtkFloatingPointType bounds[6]);

		mitk::ScalarType* GetOutPutSpacing();

		vtkMatrix4x4* GetResliceAxes(){
			return this->m_Reslicer->GetResliceAxes();
		}

		enum ResliceInterpolation { RESLICE_NEAREST, RESLICE_LINEAR, RESLICE_CUBIC };
		
		void SetInterPolationMode( ExtractSliceFilter::ResliceInterpolation interpolation){ this->m_InterpolationMode = interpolation; }

	protected:
		ExtractSliceFilter();
		virtual ~ExtractSliceFilter();

		virtual void GenerateData();
		virtual void GenerateOutputInformation();
		virtual void GenerateInputRequestedRegion();


		const Geometry2D* m_WorldGeometry;
		vtkSmartPointer<vtkImageReslice> m_Reslicer;

		unsigned int m_TimeStep;

		ResliceInterpolation m_InterpolationMode;

		const Geometry3D* m_ResliceTransform;

		bool m_InPlaneResampleExtentByGeometry;

		mitk::ScalarType* m_OutPutSpacing;

		bool CalculateClippedPlaneBounds( const Geometry3D *boundingGeometry, const PlaneGeometry *planeGeometry, vtkFloatingPointType *bounds );
		bool LineIntersectZero( vtkPoints *points, int p1, int p2, vtkFloatingPointType *bounds );
	};
}

#endif // mitkExtractSliceFilter_h_Included