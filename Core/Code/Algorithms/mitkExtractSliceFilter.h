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
		\brief ExtractSliceFilter extracts a 2D abitrary oriented slice from a 3D volume.
		
		The filter can reslice in all orthogonal planes such as sagittal, coronal and transversal,
		and is also able to reslice a abitrary oriented oblique plane.
		Curved planes are specified via an AbstractTransformGeometry as the input worldgeometry.

		The convinient workflow is:
			1. Set an image as input.
			2. Set the worldGeometry2D. This defines a grid where the slice is being extracted
			3. And then start the pipeline.
		
		There are a few more properties that can be set to modify the behavior of the slicing.
		The properties are:
			- interpolation mode	either Nearestneighbor, Linear or Cubic.
			- a transform			this is a convinient way to adapt the reslice axis for the case
									that the image is transformed e.g. rotated.
			- time step				the time step in a timesliced volume.
			- resample by geometry	wether the resampling grid corresponds to the specs of the
									worldgeometry or is directly derived from the input image

		By default the properties are set to:
			- interpolation mode	Nearestneighbor.
			- a transform			NULL (No transform is set).
			- time step				0.
			- resample by geometry	false (Corresponds to input image).
	*/
	class MITK_CORE_EXPORT ExtractSliceFilter : public ImageToImageFilter
	{
	public:

		mitkClassMacro(ExtractSliceFilter, ImageToImageFilter);
		itkNewMacro(ExtractSliceFilter);

		int* GetPlaneBounds(){ return this->m_Reslicer->GetOutput()->GetWholeExtent();}

		/** \brief Set the axis where to reslice at.*/
		void SetWorldGeometry(const Geometry2D* geometry ){ this->m_WorldGeometry = geometry; this->Modified(); }

		/** \brief Set the time step in the 4D volume */
		void SetTimeStep( unsigned int timestep){ this->m_TimeStep = timestep; }
		unsigned int GetTimeStep(){ return this->m_TimeStep; }

		/**
		  \brief Set a transform for the reslice axes.
		 
		  This transform is needed if the image volume itself is transformed. (Effects the reslice axis)
		*/
		void SetResliceTransformByGeometry(const Geometry3D* transform){ this->m_ResliceTransform = transform; }

		/** \brief Resampling grid corresponds to: false->image    true->worldgeometry*/
		void SetInPlaneResampleExtentByGeometry(bool inPlaneResampleExtentByGeometry){ this->m_InPlaneResampleExtentByGeometry = inPlaneResampleExtentByGeometry; }

		/** \brief Get the bounding box of the slice [xMin, xMax, yMin, yMax, zMin, zMax]*/
		bool GetBounds(double bounds[6]);

		/** \brief Get the spacing of the slice. returns mitk::ScalarType[2] */
		mitk::ScalarType* GetOutputSpacing();

		/** \brief Get the reslices axis matrix.
			Note: the axis are recalculated when calling SetResliceTransformByGeometry.
		*/
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

		bool m_InPlaneResampleExtentByGeometry;//Resampling grid corresponds to:  false->image    true->worldgeometry

		mitk::ScalarType* m_OutPutSpacing;

		bool CalculateClippedPlaneBounds( const Geometry3D *boundingGeometry, const PlaneGeometry *planeGeometry, vtkFloatingPointType *bounds );
		bool LineIntersectZero( vtkPoints *points, int p1, int p2, vtkFloatingPointType *bounds );
	};
}

#endif // mitkExtractSliceFilter_h_Included