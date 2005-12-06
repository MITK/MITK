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
#include <vtkImageData.h>
#include <vtkDecimatePro.h>
#include <vtkImageChangeInformation.h>
#include <vtkLinearTransform.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>

#if (VTK_MAJOR_VERSION < 5)
#include <vtkDecimate.h>
#endif

mitk::ImageToSurfaceFilter::ImageToSurfaceFilter()
{
  m_Smooth = false;
  m_Decimate = NoDecimation;
  m_Threshold = 1;
  m_TargetReduction = 0.95f;
}

mitk::ImageToSurfaceFilter::~ImageToSurfaceFilter()
{
}

template <class T1, class T2, class T3>
inline void mitkVtkLinearTransformPoint(T1 matrix[4][4], 
                                           T2 in[3], T3 out[3])
{
  T3 x = matrix[0][0]*in[0]+matrix[0][1]*in[1]+matrix[0][2]*in[2]+matrix[0][3];
  T3 y = matrix[1][0]*in[0]+matrix[1][1]*in[1]+matrix[1][2]*in[2]+matrix[1][3];
  T3 z = matrix[2][0]*in[0]+matrix[2][1]*in[1]+matrix[2][2]*in[2]+matrix[2][3];

  out[0] = x;
  out[1] = y;
  out[2] = z;
}

void mitk::ImageToSurfaceFilter::CreateSurface(int time, vtkImageData *vtkimage, mitk::Surface * surface, const ScalarType threshold)
{
  vtkImageChangeInformation *indexCoordinatesImageFilter = vtkImageChangeInformation::New();
  indexCoordinatesImageFilter->SetInput(vtkimage);
  indexCoordinatesImageFilter->SetOutputOrigin(0.0,0.0,0.0);

  //MarchingCube -->create Surface
  vtkMarchingCubes *skinExtractor = vtkMarchingCubes::New();
  skinExtractor->SetInput(indexCoordinatesImageFilter->GetOutput());//RC++
  indexCoordinatesImageFilter->Delete();
  skinExtractor->SetValue(0, threshold);

  vtkPolyData *polydata;
  polydata = skinExtractor->GetOutput();
  polydata->Register(NULL);//RC++
  skinExtractor->Delete();

  if (m_Smooth)
  {
    int spIterations =50;
    float spRelaxation =0.1;

    vtkSmoothPolyDataFilter *smoother = vtkSmoothPolyDataFilter::New();
    //read poly1 (poly1 can be the original polygon, or the decimated polygon)
    smoother->SetInput(polydata);//RC++
    smoother->SetNumberOfIterations( spIterations );
    smoother->SetRelaxationFactor( spRelaxation );
    smoother->SetFeatureAngle( 60 );
    smoother->FeatureEdgeSmoothingOff();
    smoother->BoundarySmoothingOff();
    smoother->SetConvergence( 0 );

    polydata->Delete();//RC--
    polydata = smoother->GetOutput();
    polydata->Register(NULL);//RC++
    smoother->Delete();
  }

  //decimate = to reduce number of polygons
  if(m_Decimate==DecimatePro)
  {
    vtkDecimatePro *decimate = vtkDecimatePro::New();
    decimate->SplittingOff();
    decimate->SetErrorIsAbsolute(5);
    decimate->SetFeatureAngle(30);
    decimate->PreserveTopologyOn();
    decimate->BoundaryVertexDeletionOff();
    decimate->SetDegree(10); //std-value is 25!

    decimate->SetInput(polydata);//RC++
    decimate->SetTargetReduction(m_TargetReduction);
    decimate->SetMaximumError(0.002);

    polydata->Delete();//RC--
    polydata = decimate->GetOutput();
    polydata->Register(NULL);//RC++
    decimate->Delete();
  }
  else if (m_Decimate==Decimate)
  {

#if (VTK_MAJOR_VERSION < 5)
    vtkDecimate *decimate = vtkDecimate::New();
    decimate->SetInput( polydata );
    decimate->PreserveTopologyOn();
    decimate->BoundaryVertexDeletionOff();
    decimate->SetTargetReduction( m_TargetReduction );
    polydata->Delete();//RC--
    polydata = decimate->GetOutput();
    polydata->Register(NULL);//RC++
    decimate->Delete();
#else
    std::cerr << "vtkDecimate not available in used VTK version.";
    std::cerr << std::endl;
#endif

  }

  polydata->Update();

  polydata->SetSource(NULL);

  if(polydata->GetNumberOfPoints() > 0)
  {
    vtkFloatingPointType* spacing;
    spacing = vtkimage->GetSpacing();

    vtkPoints * points = polydata->GetPoints();
    vtkMatrix4x4 *vtkmatrix = vtkMatrix4x4::New();
    GetInput()->GetGeometry(time)->GetVtkTransform()->GetMatrix(vtkmatrix);
    double (*matrix)[4] = vtkmatrix->Element;

    unsigned int i,j;
    for(i=0;i<3;++i)
      for(j=0;j<3;++j)
        matrix[i][j]/=spacing[j];

    unsigned int n = points->GetNumberOfPoints();
    vtkFloatingPointType point[3];

    for (i = 0; i < n; i++)
    {
      points->GetPoint(i, point);
      mitkVtkLinearTransformPoint(matrix,point,point);
      points->SetPoint(i, point);
    }
    vtkmatrix->Delete();
  }

  surface->SetVtkPolyData(polydata, time);
  polydata->UnRegister(NULL);
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


void mitk::ImageToSurfaceFilter::SetInput(const mitk::Image *image)
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(0, const_cast< mitk::Image * >( image ) );
}


const mitk::Image *mitk::ImageToSurfaceFilter::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
  {
    return 0;
  }

  return static_cast<const mitk::Image * >
    ( this->ProcessObject::GetInput(0) );
}


void mitk::ImageToSurfaceFilter::GenerateOutputInformation()
{
  mitk::Image::ConstPointer inputImage  =(mitk::Image*) this->GetInput();
  
  //mitk::Image *inputImage = (mitk::Image*)this->GetImage();
  mitk::Surface::Pointer output = this->GetOutput();

  itkDebugMacro(<<"GenerateOutputInformation()");

  if(inputImage.IsNull()) return;
  
  //Set Data
}
