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


#include "mitkImageToSurfaceFilter.h"
#if VTK_MAJOR_VERSION >= 5
#define USE_VTK_DECIMATE_PRO
#endif

#ifdef USE_VTK_DECIMATE_PRO
#include <vtkDecimatePro.h>
#else
#include <vtkDecimate.h>
#endif

mitk::ImageToSurfaceFilter::ImageToSurfaceFilter()
  : m_SetSmooth(false), m_SetDecimate(false)
  { }
mitk::ImageToSurfaceFilter::~ImageToSurfaceFilter(){};



void mitk::ImageToSurfaceFilter::CreateSurface(int time, vtkImageData *vtkimage, mitk::Surface * surface, const ScalarType threshold)
{
  //Inkrement Referenzcounter Counter (hier: RC)
  vtkimage->Register(NULL);

  vtkPolyData *polydata = vtkPolyData::New();
  polydata->Register(NULL);// RC++

  //MarchingCube -->create Surface
  vtkMarchingCubes *skinExtractor = vtkMarchingCubes::New();
  skinExtractor->SetInput(vtkimage);//RC++
  vtkimage->Delete();//RC--
  skinExtractor->SetValue(0, threshold); 
  polydata = skinExtractor->GetOutput();
  
  if (m_SetSmooth)  
  {
    int spIterations =50;
    float spRelaxation =0.1;
    
    vtkSmoothPolyDataFilter *smoother = vtkSmoothPolyDataFilter::New();
    //read poly1 (poly1 can be the original polygon, or the decimated polygon)
    smoother->SetInput(polydata);//RC++
    polydata->Delete();//RC--
    smoother->SetNumberOfIterations( spIterations );
    smoother->SetRelaxationFactor( spRelaxation );
    smoother->SetFeatureAngle( 60 );
    smoother->FeatureEdgeSmoothingOff();
    smoother->BoundarySmoothingOff();
    smoother->SetConvergence( 0 );

    polydata = smoother->GetOutput(); 

  }

  //decimate = to reduce number of polygons
  if(m_SetDecimate)
  {
#ifdef USE_VTK_DECIMATE_PRO
    vtkDecimatePro *decimate = vtkDecimatePro::New();
    decimate->SplittingOff();
#else
    vtkDecimate *decimate = vtkDecimate::New();
    decimate->PreserveEdgesOn();
    decimate->GenerateErrorScalarsOn();
    decimate->SetInitialError(0.0005);
    decimate->SetErrorIncrement(0.0005);
    decimate->SetMaximumIterations(10);

    decimate->SetInitialFeatureAngle(15);
    decimate->SetFeatureAngleIncrement(5);
    decimate->SetMaximumFeatureAngle(30);

    decimate->SetAspectRatio(10);
#endif
    
    decimate->PreserveTopologyOn();
    decimate->BoundaryVertexDeletionOff();
    decimate->SetDegree(10); //std-value is 25!
    
    decimate->SetInput(polydata);//RC++
    polydata->Delete();//RC--
    decimate->SetTargetReduction(0.05f);

    
    decimate->SetReleaseDataFlag(true);
    decimate->SetMaximumError(0.002);

    polydata = decimate->GetOutput();
  
  }


  surface->SetVtkPolyData(polydata, time);
}



void mitk::ImageToSurfaceFilter::GenerateData()
{
  mitk::Surface *surface = this->GetOutput();
  mitk::Image * image        =  (mitk::Image*)GetInput();
  mitk::Image::RegionType outputRegion = image->GetRequestedRegion();
 
  int tstart=outputRegion.GetIndex(3);
  int tmax=tstart+outputRegion.GetSize(3); //GetSize()==1 - will aber 0 haben, wenn nicht zeitaufgelöst

  int t;
  for( t=tstart; t < tmax; ++t)
  {
    vtkImageData *vtkimagedata =  image->GetVtkImageData(t);
    CreateSurface(t,vtkimagedata,surface,m_Threshold);
  }
}


void mitk::ImageToSurfaceFilter::GenerateOutputInformation()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Surface::Pointer output  = this->GetOutput();

  itkDebugMacro(<<"GenerateOutputInformation()");
  if(input.IsNull()) return;

  itkDebugMacro(<<"GenerateOutputInformation()");
}



void mitk::ImageToSurfaceFilter::SetInput(const mitk::Image *image)
{
  // Process object is not const-correct so the const_cast is required here  
  this->ProcessObject::SetNthInput(0, const_cast< mitk::Image * >( image ) );
}


/**
* Get image input
*/
const mitk::Image *mitk::ImageToSurfaceFilter::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
  {
    return 0;
  }

  return static_cast<const mitk::Image * >
    ( this->ProcessObject::GetInput(0) );
}
