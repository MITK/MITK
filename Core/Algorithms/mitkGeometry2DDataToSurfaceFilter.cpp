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


#include "mitkGeometry2DDataToSurfaceFilter.h"
#include "mitkSurface.h"
#include "mitkGeometry2DData.h"
#include "mitkPlaneGeometry.h"
#include "mitkAbstractTransformGeometry.h"
#include <vtkPolyData.h>
#include <vtkPlaneSource.h>
#include <vtkTransformPolyDataFilter.h>

//##ModelId=3EF4A4A70345
mitk::Geometry2DDataToSurfaceFilter::Geometry2DDataToSurfaceFilter()
  : m_UseGeometryParametricBounds(true), m_XResolution(10), m_YResolution(10)
{
    m_VtkPlaneSource=vtkPlaneSource::New();
        m_VtkPlaneSource->SetXResolution(m_XResolution);
        m_VtkPlaneSource->SetYResolution(m_YResolution);
        m_VtkPlaneSource->SetOrigin(  0,   0, 0);
        m_VtkPlaneSource->SetPoint1(256,   0, 0);
        m_VtkPlaneSource->SetPoint2(  0, 256, 0);
    m_VtkTransformPlaneFilter=vtkTransformPolyDataFilter::New();
        m_VtkTransformPlaneFilter->SetInput(m_VtkPlaneSource->GetOutput());
}

//##ModelId=3EF4A4A70363
mitk::Geometry2DDataToSurfaceFilter::~Geometry2DDataToSurfaceFilter()
{
  m_VtkPlaneSource->Delete();
  m_VtkTransformPlaneFilter->Delete();
}

//##ModelId=3EF56DCD021E
void mitk::Geometry2DDataToSurfaceFilter::GenerateOutputInformation()
{
	mitk::Geometry2DData::ConstPointer input  = this->GetInput();
	mitk::Surface::Pointer output = this->GetOutput();

	itkDebugMacro(<<"GenerateOutputInformation()");

    if(input.IsNull()) return;
    if(input->GetGeometry2D()==NULL) return;

    Point3D origin;
    Point3D right, bottom;

    vtkPolyData* surface=NULL;
    //contains the Geometry2DData a PlaneGeometry?
    mitk::PlaneGeometry::ConstPointer planeGeometry = dynamic_cast<const PlaneGeometry *>(input->GetGeometry2D());
    if(planeGeometry.IsNotNull())
    {
      const PlaneGeometry* planeview=planeGeometry;
      origin = planeview->GetOrigin();
      right  = planeview->GetCornerPoint(false);
      bottom = planeview->GetCornerPoint(true, false);

        m_VtkPlaneSource->SetXResolution(1);
        m_VtkPlaneSource->SetYResolution(1);

        surface=m_VtkPlaneSource->GetOutput();
    }
    else
    {
        mitk::AbstractTransformGeometry::ConstPointer abstractGeometry = dynamic_cast<const AbstractTransformGeometry *>(input->GetGeometry2D());
        if(abstractGeometry.IsNotNull())
        {
          origin = abstractGeometry->GetPlane()->GetOrigin();
          right  = origin+abstractGeometry->GetPlane()->GetAxisVector(0);
          bottom = origin+abstractGeometry->GetPlane()->GetAxisVector(1);
        }
        else
            return;

        if(m_UseGeometryParametricBounds)
        { 
          m_VtkPlaneSource->SetXResolution((int)abstractGeometry->GetParametricExtent(0));
          m_VtkPlaneSource->SetYResolution((int)abstractGeometry->GetParametricExtent(1));
        }
        else
        {
          m_VtkPlaneSource->SetXResolution(m_XResolution);
          m_VtkPlaneSource->SetYResolution(m_YResolution);
        }

        m_VtkTransformPlaneFilter->SetTransform(abstractGeometry->GetVtkAbstractTransform());
        surface=m_VtkTransformPlaneFilter->GetOutput();
    }

    int w=vtkObject::GetGlobalWarningDisplay();
    vtkObject::GlobalWarningDisplayOff();
    m_VtkPlaneSource->SetOrigin(origin[0], origin[1], origin[2]);
    m_VtkPlaneSource->SetPoint1(right[0], right[1], right[2]);
    m_VtkPlaneSource->SetPoint2(bottom[0], bottom[1], bottom[2]);
    //we can only set this now, because the initialization is only now completed! (SetVtkPolyData calculates the bounds!)
    surface->Update();
    vtkObject::SetGlobalWarningDisplay(w);
    output->SetVtkPolyData(surface);
}

//##ModelId=3EF56DD10039
void mitk::Geometry2DDataToSurfaceFilter::GenerateData()
{
	mitk::Surface::Pointer output = this->GetOutput();

	itkDebugMacro(<<"GenerateData()");

    if(output.IsNull()) return;
    if(output->GetVtkPolyData()==NULL) return;

    output->GetVtkPolyData()->Update();
}

//##ModelId=3EF5795002B2
const mitk::Geometry2DData *mitk::Geometry2DDataToSurfaceFilter::GetInput(void)
{
	if (this->GetNumberOfInputs() < 1)
	{
		return 0;
	}

	return static_cast<const mitk::Geometry2DData * >
		(this->ProcessObject::GetInput(0) );
}

//##ModelId=3EF5795202A1
const mitk::Geometry2DData *mitk::Geometry2DDataToSurfaceFilter::GetInput(unsigned int idx)
{
    return static_cast< const mitk::Geometry2DData * >
		(this->ProcessObject::GetInput(idx));
}

//##ModelId=3EF59AF703BE
void mitk::Geometry2DDataToSurfaceFilter::SetInput(const mitk::Geometry2DData *input)
{
	// Process object is not const-correct so the const_cast is required here
	this->ProcessObject::SetNthInput(0, 
		const_cast< mitk::Geometry2DData * >( input ) );
}

//##ModelId=3EF59AFA01D8
void mitk::Geometry2DDataToSurfaceFilter::SetInput(unsigned int index, const mitk::Geometry2DData *input)
{
	if( index+1 > this->GetNumberOfInputs() )
	{
		this->SetNumberOfRequiredInputs( index + 1 );
	}
	// Process object is not const-correct so the const_cast is required here
	this->ProcessObject::SetNthInput(index, 
		const_cast< mitk::Geometry2DData *>( input ) );
}

