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

#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkImageVtkAccessor.h>

// itk
#include <itkBinaryThresholdImageFilter.h>
#include <itkAntiAliasBinaryImageFilter.h>
#include <itkLabelObject.h>
#include <itkLabelMap.h>
#include <itkLabelImageToLabelMapFilter.h>
#include <itkAutoCropLabelMapFilter.h>
#include <itkLabelMapToLabelImageFilter.h>
#include <itkSmoothingRecursiveGaussianImageFilter.h>
#include <itkNumericTraits.h>

// vtk
#include <vtkSmartPointer.h>
#include <vtkMarchingCubes.h>
#include <vtkLinearTransform.h>
#include <vtkImageChangeInformation.h>
#include <vtkCleanPolyData.h>
#include <vtkImageData.h>


mitk::LabelSetImageToSurfaceFilter::LabelSetImageToSurfaceFilter() :
m_GenerateAllLabels(false),
m_RequestedLabel(1),
m_BackgroundLabel(0),
m_UseSmoothing(0),
m_Sigma(0.1)
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
    return nullptr;
  }

  return static_cast<const mitk::Image * >( this->ProcessObject::GetInput(0) );
}

void mitk::LabelSetImageToSurfaceFilter::GenerateOutputInformation()
{
  itkDebugMacro(<<"GenerateOutputInformation()");
}

void mitk::LabelSetImageToSurfaceFilter::GenerateData()
{
  Image::ConstPointer inputImage = this->GetInput();
  if ( inputImage.IsNull() ) return;

  mitk::Surface* outputSurface = this->GetOutput( );
  if (!outputSurface) return;

  AccessFixedDimensionByItk_1( inputImage, InternalProcessing, 3, outputSurface );
}

template < typename TPixel, unsigned int VDimension >
void mitk::LabelSetImageToSurfaceFilter::InternalProcessing( const itk::Image<TPixel, VDimension>* input, mitk::Surface* /*surface*/ )
{
  typedef itk::Image<TPixel, VDimension> ImageType;

  typedef itk::BinaryThresholdImageFilter< ImageType, ImageType > BinaryThresholdFilterType;
  typedef itk::LabelObject< TPixel, VDimension > LabelObjectType;
  typedef itk::LabelMap< LabelObjectType > LabelMapType;
  typedef itk::LabelImageToLabelMapFilter< ImageType, LabelMapType > Image2LabelMapType;
  typedef itk::AutoCropLabelMapFilter< LabelMapType > AutoCropType;
  typedef itk::LabelMapToLabelImageFilter< LabelMapType, ImageType > LabelMap2ImageType;

  typedef itk::Image<float, VDimension> RealImageType;

  typedef itk::AntiAliasBinaryImageFilter< ImageType, RealImageType >  AntiAliasFilterType;
  typedef itk::SmoothingRecursiveGaussianImageFilter< RealImageType, RealImageType >  GaussianFilterType;


  typename BinaryThresholdFilterType::Pointer thresholdFilter = BinaryThresholdFilterType::New();
  thresholdFilter->SetInput( input );
  thresholdFilter->SetLowerThreshold(m_RequestedLabel);
  thresholdFilter->SetUpperThreshold(m_RequestedLabel);
  thresholdFilter->SetOutsideValue(0);
  thresholdFilter->SetInsideValue(1);
//  thresholdFilter->ReleaseDataFlagOn();
  thresholdFilter->Update();

  typename Image2LabelMapType::Pointer image2label = Image2LabelMapType::New();
  image2label->SetInput(thresholdFilter->GetOutput());

  typename AutoCropType::SizeType border;
  border[0] = 3;
  border[1] = 3;
  border[2] = 3;

  typename AutoCropType::Pointer autoCropFilter = AutoCropType::New();
  autoCropFilter->SetInput( image2label->GetOutput() );
  autoCropFilter->SetCropBorder(border);
  autoCropFilter->InPlaceOn();

  typename LabelMap2ImageType::Pointer label2image = LabelMap2ImageType::New();
  label2image->SetInput( autoCropFilter->GetOutput() );

  label2image->Update();

  typename AntiAliasFilterType::Pointer antiAliasFilter = AntiAliasFilterType::New();
  antiAliasFilter->SetInput( label2image->GetOutput() );
  antiAliasFilter->SetMaximumRMSError(0.001);
  antiAliasFilter->SetNumberOfLayers(3);
  antiAliasFilter->SetUseImageSpacing(false);
  antiAliasFilter->SetNumberOfIterations(40);

  antiAliasFilter->Update();

  typename RealImageType::Pointer result;

  if (m_UseSmoothing)
  {
    typename GaussianFilterType::Pointer gaussianFilter = GaussianFilterType::New();
    gaussianFilter->SetSigma( m_Sigma );
    gaussianFilter->SetInput( antiAliasFilter->GetOutput() );
    gaussianFilter->Update();
    result = gaussianFilter->GetOutput();
  }
  else
  {
    result = antiAliasFilter->GetOutput();
  }

  result->DisconnectPipeline();

  typename ImageType::RegionType cropRegion;
  cropRegion = autoCropFilter->GetOutput()->GetLargestPossibleRegion();

  const typename ImageType::IndexType& cropIndex = cropRegion.GetIndex();

  m_ResultImage = mitk::Image::New();
  mitk::CastToMitkImage(result, m_ResultImage);

  mitk::BaseGeometry* newGeometry = m_ResultImage->GetSlicedGeometry();
  mitk::Point3D origin;
  vtk2itk(cropIndex, origin);
  this->GetInput()->GetGeometry()->IndexToWorld(origin, origin);
  newGeometry->SetOrigin(origin);

  mitk::ImageVtkAccessor accessor(m_ResultImage);
  mitk::ImageAccessLock lock(&accessor);
  vtkImageData* vtkimage = accessor.getVtkImageData(0);

  vtkSmartPointer<vtkImageChangeInformation> indexCoordinatesImageFilter = vtkSmartPointer<vtkImageChangeInformation>::New();
  indexCoordinatesImageFilter->SetInputData(vtkimage);
  indexCoordinatesImageFilter->SetOutputOrigin(0.0,0.0,0.0);

  vtkSmartPointer<vtkMarchingCubes> marching = vtkSmartPointer<vtkMarchingCubes>::New();
  marching->ComputeScalarsOff();
  marching->ComputeNormalsOn();
  marching->ComputeGradientsOn();
  marching->SetInputConnection(indexCoordinatesImageFilter->GetOutputPort());
  marching->SetValue(0, 0.0);

  marching->Update();

  vtkPolyData* polydata = marching->GetOutput();

  if ( (!polydata) || (!polydata->GetNumberOfPoints()) )
    throw itk::ExceptionObject (__FILE__,__LINE__,"marching cubes has failed.");

  mitk::Vector3D spacing = newGeometry->GetSpacing();

  vtkPoints * points = polydata->GetPoints();
  vtkMatrix4x4 *vtkmatrix = vtkMatrix4x4::New();
  newGeometry->GetVtkTransform()->GetMatrix(vtkmatrix);
  double (*matrix)[4] = vtkmatrix->Element;

  for(int i=0;i<3;++i)
    for(int j=0;j<3;++j)
      matrix[i][j]/=spacing[j];

  unsigned int n = points->GetNumberOfPoints();
  double point[3];

  for (unsigned int i=0; i<n; i++)
  {
    points->GetPoint(i, point);
    mitkVtkLinearTransformPoint(matrix,point,point);
    points->SetPoint(i, point);
  }
  vtkmatrix->Delete();

  vtkSmartPointer<vtkCleanPolyData> cleanPolyDataFilter = vtkSmartPointer<vtkCleanPolyData>::New();
  cleanPolyDataFilter->SetInputData(polydata);
  cleanPolyDataFilter->PieceInvariantOff();
  cleanPolyDataFilter->ConvertLinesToPointsOff();
  cleanPolyDataFilter->ConvertPolysToLinesOff();
  cleanPolyDataFilter->ConvertStripsToPolysOff();
  cleanPolyDataFilter->PointMergingOn();
  cleanPolyDataFilter->Update();

  mitk::Surface::Pointer output = this->GetOutput(0);
  output->SetVtkPolyData(cleanPolyDataFilter->GetOutput(), 0);
}
