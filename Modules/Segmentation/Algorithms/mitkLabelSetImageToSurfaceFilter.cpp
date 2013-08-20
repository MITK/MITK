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
  mitk::Image* inputImage = const_cast<mitk::Image*> ( this->GetInput() );
  mitk::Surface* output = this->GetOutput();

  itkDebugMacro(<<"GenerateOutputInformation()");

  //
  // check which labels are available in the image
  //
  m_AvailableLabels = this->GetAvailableLabels();

  m_IndexToLabels.clear();

  //
  // if we don't want to generate surfaces for all labels
  // we have to remove all labels except m_RequestedLabel and m_BackgroundLabel
  // from the list of available labels
  //
  if ( ! m_GenerateAllLabels )
  {
    LabelMapType tmp;
    LabelMapType::iterator it;
    it = m_AvailableLabels.find( m_RequestedLabel );
    if ( it != m_AvailableLabels.end() )
      tmp[m_RequestedLabel] = it->second;
    else
      tmp[m_RequestedLabel] = 0;

    it = m_AvailableLabels.find( m_BackgroundLabel );
    if ( it != m_AvailableLabels.end() )
      tmp[m_BackgroundLabel] = it->second;
    else
      tmp[m_BackgroundLabel] = 0;

    m_AvailableLabels = tmp;
  }

  //
  // check for the number of labels: if the whole image is filled, no
  // background is available and thus the numberOfOutpus is equal to the
  // number of available labels in the image (which is a special case).
  // If we have background voxels, the number of outputs is one less than
  // the number of available labels.
  //
  unsigned int numberOfOutputs = 0;
  if ( m_AvailableLabels.find( m_BackgroundLabel ) == m_AvailableLabels.end() )
    numberOfOutputs = m_AvailableLabels.size();
  else
    numberOfOutputs = m_AvailableLabels.size() - 1;

  if ( numberOfOutputs == 0 )
  {
    MITK_WARN << "Number of outputs == 0";
  }

  mitk::Image::ConstPointer image = this->GetInput();

  //
  // set the number of outputs to the number of labels used.
  // initialize the output surfaces accordingly (incl. time steps)
  //
  this->SetNumberOfOutputs( numberOfOutputs );
  this->SetNumberOfRequiredOutputs( numberOfOutputs );
  for ( unsigned int i = 0 ; i < numberOfOutputs; ++i )
  {
    if ( ! this->GetOutput( i ) )
    {
      mitk::Surface::Pointer output = static_cast<mitk::Surface*>( this->MakeOutput(0).GetPointer() );
      assert ( output.IsNotNull() );
      this->SetNthOutput( i, output );
    }
  }
}

void mitk::LabelSetImageToSurfaceFilter::GenerateData()
{
  this->m_DataIsAvailable = false;

  mitk::Image::ConstPointer inputImage = const_cast<mitk::Image*>( this->GetInput() );
  if ( inputImage.IsNull() ) return;

  m_IndexToLabels.clear();

  // traverse the known labels and create surfaces for them.
  //
  unsigned int currentOutputIndex = 0;
  for ( LabelMapType::iterator it = m_AvailableLabels.begin() ; it != m_AvailableLabels.end() ; ++it )
  {
    if ( it->first == m_BackgroundLabel )
      continue;

    if ( ( it->second == 0 ) && m_GenerateAllLabels )
      continue;

    if ( currentOutputIndex > this->GetNumberOfOutputs() )
    {
       this->m_DataIsAvailable = false;
       return;
    }

    mitk::Surface* outputSurface = this->GetOutput( currentOutputIndex );
    if (!outputSurface) return;

    m_LowerThreshold = static_cast<int>( it->first );
    m_UpperThreshold = static_cast<int>( it->first );

    AccessFixedDimensionByItk_1( inputImage, ITKProcessing, 3, outputSurface );

    m_IndexToLabels[ currentOutputIndex ] = it->first;
    currentOutputIndex++;
  }

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

template < typename TPixel, unsigned int VImageDimension >
void mitk::LabelSetImageToSurfaceFilter::GetAvailableLabelsInternal(
   itk::Image<TPixel, VImageDimension>* input, mitk::LabelSetImageToSurfaceFilter::LabelMapType& availableLabels )
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;
  typedef itk::ImageRegionIterator< InputImageType > ImageRegionIteratorType;
  availableLabels.clear();

  ImageRegionIteratorType it( input, input->GetLargestPossibleRegion() );
  it.GoToBegin();

  mitk::LabelSetImageToSurfaceFilter::LabelMapType::iterator labelIt;
  while( ! it.IsAtEnd() )
  {
    labelIt = availableLabels.find( static_cast< mitk::LabelSetImageToSurfaceFilter::LabelType> ( it.Get() ) );
    if ( labelIt == availableLabels.end() )
    {
      availableLabels[ static_cast< mitk::LabelSetImageToSurfaceFilter::LabelType> ( it.Get() ) ] = 1;
    }
    else
    {
      labelIt->second += 1;
    }

    ++it;
  }
}

//InstantiateAccessFunctionForFixedDimension_1(GetAvailableLabelsInternal, 3, mitk::LabelSetImageToSurfaceFilter::LabelMapType&);


mitk::LabelSetImageToSurfaceFilter::LabelMapType mitk::LabelSetImageToSurfaceFilter::GetAvailableLabels()
{
   mitk::Image::ConstPointer inputImage = this->GetInput();
   LabelMapType availableLabels;

   AccessFixedDimensionByItk_1( inputImage, GetAvailableLabelsInternal, 3, availableLabels );

   this->m_DataIsAvailable = true;

   return availableLabels;
}

mitk::LabelSetImageToSurfaceFilter::LabelType mitk::LabelSetImageToSurfaceFilter::GetLabelForNthOutput( const unsigned int& index )
{
  IndexToLabelMapType::iterator it = m_IndexToLabels.find( index );
  if ( it != m_IndexToLabels.end() )
  {
    return it->second;
  }
  else
  {
    MITK_WARN << "Unknown index encountered: " << index << ". There are " << this->GetNumberOfOutputs() << " outputs available";
    return itk::NumericTraits<LabelType>::max();
  }
}

mitk::ScalarType mitk::LabelSetImageToSurfaceFilter::GetVolumeForNthOutput( const unsigned int& i )
{
  return this->GetVolumeForLabel( GetLabelForNthOutput( i ) );
}

mitk::ScalarType mitk::LabelSetImageToSurfaceFilter::GetVolumeForLabel( const mitk::LabelSetImageToSurfaceFilter::LabelType& label )
{
  // get the image spacing
  mitk::Image::ConstPointer image = this->GetInput();
  const float* spacing = image->GetSlicedGeometry()->GetFloatSpacing();

  // get the number of voxels encountered for the given label,
  // calculate the volume and return it.
  LabelMapType::iterator it = m_AvailableLabels.find( label );
  if ( it != m_AvailableLabels.end() )
  {
    return static_cast<mitk::ScalarType>(it->second) * ( spacing[0] * spacing[1] * spacing[2] / 1000.0f );
  }
  else
  {
    MITK_WARN << "Unknown label encountered: " << label;
    return 0.0;
  }
}
