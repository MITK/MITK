/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <mitkLabelSetImageToSurfaceFilter.h>

#include <vtkSmartPointer.h>
#include <vtkMarchingCubes.h>
#include <vtkLinearTransform.h>
#include <vtkImageChangeInformation.h>

#include <mitkImageAccessByItk.h>
#include <mitkTool.h>

#include <itkBinaryThresholdImageFilter.h>
#include <itkAntiAliasBinaryImageFilter.h>

#include <itkImageToVTKImageFilter.h>

#include <itkImageRegionIterator.h>
#include <itkNumericTraits.h>


mitk::LabelSetImageToSurfaceFilter::LabelSetImageToSurfaceFilter() :
m_GenerateAllLabels(false),
m_RequestedLabel(1),
m_BackgroundLabel(0),
m_UseSmoothing(0),
m_DataIsAvailable(false)
{
}

mitk::LabelSetImageToSurfaceFilter::~LabelSetImageToSurfaceFilter()
{
}

void mitk::LabelSetImageToSurfaceFilter::SetInput(const mitk::Image *image)
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(0, const_cast< mitk::Image * >( image ) );
}
/*
void mitk::LabelSetImageToSurfaceFilter::SetObserver(mitk::ProcessObserver::Pointer observer)
{
   m_Observer = observer;
}
*/
const mitk::Image *mitk::LabelSetImageToSurfaceFilter::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
  {
    return 0;
  }

  return static_cast<const mitk::Image * >
    ( this->ProcessObject::GetInput(0) );
}

void mitk::LabelSetImageToSurfaceFilter::GenerateOutputInformation()
{
  itkDebugMacro(<<"GenerateOutputInformation()");
}

void mitk::LabelSetImageToSurfaceFilter::GenerateData()
{
  this->m_DataIsAvailable = false;

  mitk::Image::ConstPointer inputImage = const_cast<mitk::Image*>( this->GetInput() );
  if ( inputImage.IsNull() ) return;

  mitk::Surface* outputSurface = this->GetOutput( );
  if (!outputSurface) return;

  m_LowerThreshold = m_RequestedLabel;
  m_UpperThreshold = m_RequestedLabel;

  AccessFixedDimensionByItk_1( inputImage, ITKProcessing, 3, outputSurface );

  this->m_DataIsAvailable = true;
}

bool mitk::LabelSetImageToSurfaceFilter::GetDataIsAvailable()
{
   return this->m_DataIsAvailable;
}

template < typename TPixel, unsigned int VImageDimension >
void mitk::LabelSetImageToSurfaceFilter::ITKProcessing( itk::Image<TPixel, VImageDimension>* input, mitk::Surface* surface )
{
   typedef itk::Image<TPixel, VImageDimension> InputImageType;
   typedef itk::Image<mitk::Tool::DefaultSegmentationDataType, VImageDimension> BinaryImageType;
   typedef itk::BinaryThresholdImageFilter< InputImageType, BinaryImageType > BinaryThresholdFilterType;
   typedef float FloatPixelType;
   typedef itk::Image<FloatPixelType, VImageDimension> FloatImageType;
   typedef itk::AntiAliasBinaryImageFilter< BinaryImageType, FloatImageType > AntiAliasFilterType;
   typedef itk::ImageToVTKImageFilter< FloatImageType > ITKFloatConvertType;

   typename BinaryThresholdFilterType::Pointer thresholdFilter = BinaryThresholdFilterType::New();
   thresholdFilter->SetInput( input );
   thresholdFilter->SetLowerThreshold(m_LowerThreshold);
   thresholdFilter->SetUpperThreshold(m_UpperThreshold);
   thresholdFilter->SetOutsideValue(0);
   thresholdFilter->SetInsideValue(1);
   thresholdFilter->ReleaseDataFlagOn();

   typename AntiAliasFilterType::Pointer antiAliasFilter;
   // todo: check why input parameters are not used in the ITK filter
   antiAliasFilter = AntiAliasFilterType::New();
   antiAliasFilter->SetMaximumRMSError(0.01);
   antiAliasFilter->SetNumberOfLayers(2);
   antiAliasFilter->SetNumberOfIterations(15);
   antiAliasFilter->SetInput( thresholdFilter->GetOutput() );
   antiAliasFilter->ReleaseDataFlagOn();
/*
   if (m_Observer.IsNotNull())
   {
      m_Observer->SetRemainingProgress(100);
      thresholdFilter->AddObserver(itk::AnyEvent(), m_Observer);
      antiAliasFilter->AddObserver(itk::AnyEvent(), m_Observer);
      m_Observer->AddStepsToDo(100);
   }
*/
   antiAliasFilter->Update();
/*
   if (m_Observer.IsNotNull())
   {
      thresholdFilter->RemoveAllObservers();
      antiAliasFilter->RemoveAllObservers();
      m_Observer->SetRemainingProgress(100);
   }
*/
   typename FloatImageType::Pointer outputFloatImage;

   outputFloatImage = antiAliasFilter->GetOutput();
   outputFloatImage->DisconnectPipeline();

   typename ITKFloatConvertType::Pointer connector;
   connector = ITKFloatConvertType::New();
   connector->SetInput( outputFloatImage );
   connector->ReleaseDataFlagOn();

   connector->Update();

   vtkSmartPointer<vtkImageChangeInformation> indexCoordinatesImageFilter = vtkSmartPointer<vtkImageChangeInformation>::New();
   indexCoordinatesImageFilter->SetInput(connector->GetOutput());
   indexCoordinatesImageFilter->SetOutputOrigin(0.0,0.0,0.0);

   vtkSmartPointer<vtkMarchingCubes> marching = vtkSmartPointer<vtkMarchingCubes>::New();
   marching->ComputeNormalsOn();
   marching->ComputeGradientsOn();
   marching->SetInput(indexCoordinatesImageFilter->GetOutput());
   marching->SetValue(0,0);
   marching->ReleaseDataFlagOn();

   marching->Update();

   vtkPolyData* polydata = marching->GetOutput();

   if ( (!polydata) || (!polydata->GetNumberOfPoints()) )
      throw itk::ExceptionObject (__FILE__,__LINE__,"LabelSetImageToSurfaceFilter: marching cubes has failed.");

   if (polydata->GetNumberOfPoints() > 0)
   {
    mitk::Vector3D spacing = this->GetInput()->GetGeometry(0)->GetSpacing();

    vtkPoints * points = polydata->GetPoints();
    vtkMatrix4x4 *vtkmatrix = vtkMatrix4x4::New();
    this->GetInput()->GetGeometry(0)->GetVtkTransform()->GetMatrix(vtkmatrix);
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

  surface->SetVtkPolyData( polydata );
}
