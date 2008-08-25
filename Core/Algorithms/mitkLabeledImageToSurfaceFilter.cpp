/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

#include <mitkLabeledImageToSurfaceFilter.h>

#include <vtkImageChangeInformation.h>
#include <vtkImageThreshold.h>
#include <vtkImageGaussianSmooth.h>
#include <vtkImageMarchingCubes.h>
#include <vtkPolyData.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkDecimatePro.h>
#if (VTK_MAJOR_VERSION < 5)
#include <vtkDecimate.h>
#endif
#include <vtkLinearTransform.h>
#include <vtkMatrix4x4.h>

#include <mitkImageAccessByItk.h>
#include <itkImageRegionIterator.h>
#include <itkNumericTraits.h>


mitk::LabeledImageToSurfaceFilter::LabeledImageToSurfaceFilter() :
m_GaussianStandardDeviation(1.5),
m_GenerateAllLabels(true),
m_Label(1),
m_BackgroundLabel(0)
{
}

mitk::LabeledImageToSurfaceFilter::~LabeledImageToSurfaceFilter()
{
}

void mitk::LabeledImageToSurfaceFilter::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();
  //
  // check which labels are available in the image
  //
  m_AvailableLabels = this->GetAvailableLabels();
  m_IdxToLabels.clear();
  
  //
  // if we don't want to generate surfaces for all labels
  // we have to remove all labels except m_Label and m_BackgroundLabel 
  // from the list of available labels
  //
  if ( ! m_GenerateAllLabels )
  {
    LabelMapType tmp;
    LabelMapType::iterator it;
    it = m_AvailableLabels.find( m_Label );
    if ( it != m_AvailableLabels.end() )
      tmp[m_Label] = it->second;
    else
      tmp[m_Label] = 0;
    
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
  // then number of available labels.
  //
  unsigned int numberOfOutputs = 0;
  if ( m_AvailableLabels.find( m_BackgroundLabel ) == m_AvailableLabels.end() )
    numberOfOutputs = m_AvailableLabels.size();
  else
    numberOfOutputs = m_AvailableLabels.size() - 1;
  if ( numberOfOutputs == 0 )
  {
    itkWarningMacro("Number of outputs == 0");
  }

  //
  // determine the number of time steps of the input image
  //
  mitk::Image* image =  ( mitk::Image* )GetInput();
  unsigned int numberOfTimeSteps = image->GetTimeSlicedGeometry()->GetTimeSteps();
    
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
      output->Expand( numberOfTimeSteps ); 
      this->SetNthOutput( i, output.GetPointer() );
    }
  }
}


void mitk::LabeledImageToSurfaceFilter::GenerateData()
{
  mitk::Image* image =  ( mitk::Image* )GetInput();
  if ( image == NULL )
  {
    itkWarningMacro("Image is NULL");
    return;
  }
    
  mitk::Image::RegionType outputRegion = image->GetRequestedRegion();

  m_IdxToLabels.clear();
  
  if ( this->GetNumberOfOutputs() == 0 )
    return;
  
  //
  // traverse the known labels and create surfaces for them.
  //
  unsigned int currentOutputIndex = 0;
  for ( LabelMapType::iterator it = m_AvailableLabels.begin() ; it != m_AvailableLabels.end() ; ++it )
  {
    if ( it->first == m_BackgroundLabel )
      continue;
    if ( ( it->second == 0 ) && m_GenerateAllLabels )
      continue;
    
    assert ( currentOutputIndex < this->GetNumberOfOutputs() );
    mitk::Surface::Pointer surface = this->GetOutput( currentOutputIndex );
    assert( surface.IsNotNull() );
    
    int tstart=outputRegion.GetIndex(3);
    int tmax=tstart+outputRegion.GetSize(3); //GetSize()==1 - will aber 0 haben, wenn nicht zeitaufgeloet
    int t;
    for( t=tstart; t < tmax; ++t)
    {
      vtkImageData *vtkimagedata =  image->GetVtkImageData( t );
      CreateSurface( t,vtkimagedata,surface.GetPointer(), it->first );
    }
    m_IdxToLabels[ currentOutputIndex ] = it->first;
    currentOutputIndex++;
  }
}

void mitk::LabeledImageToSurfaceFilter::CreateSurface( int time, vtkImageData *vtkimage, mitk::Surface * surface, mitk::LabeledImageToSurfaceFilter::LabelType label )
{
  vtkImageChangeInformation *indexCoordinatesImageFilter = vtkImageChangeInformation::New();
  indexCoordinatesImageFilter->SetInput(vtkimage);
  indexCoordinatesImageFilter->SetOutputOrigin(0.0,0.0,0.0);

    vtkImageThreshold* threshold = vtkImageThreshold::New();
    threshold->SetInput( indexCoordinatesImageFilter->GetOutput() );
    //indexCoordinatesImageFilter->Delete();
    threshold->SetInValue( 100 );
    threshold->SetOutValue( 0 );
    threshold->ThresholdBetween( label, label ); 
    threshold->SetOutputScalarTypeToUnsignedChar();
    threshold->ReleaseDataFlagOn();

    vtkImageGaussianSmooth *gaussian = vtkImageGaussianSmooth::New();
    gaussian->SetInput( threshold->GetOutput() ); 
    //threshold->Delete();
    gaussian->SetDimensionality( 3  );
    gaussian->SetRadiusFactor( 0.49 );
    gaussian->SetStandardDeviation( GetGaussianStandardDeviation() );
    gaussian->ReleaseDataFlagOn();
    gaussian->UpdateInformation();
    gaussian->Update();

  //MarchingCube -->create Surface
  vtkMarchingCubes *skinExtractor = vtkMarchingCubes::New();
  skinExtractor->ReleaseDataFlagOn();
  skinExtractor->SetInput(gaussian->GetOutput());//RC++
  indexCoordinatesImageFilter->Delete();
  skinExtractor->SetValue(0, 50);

  vtkPolyData *polydata;
  polydata = skinExtractor->GetOutput();
  polydata->Register(NULL);//RC++
  skinExtractor->Delete();

  if (m_Smooth)
  {
    vtkSmoothPolyDataFilter *smoother = vtkSmoothPolyDataFilter::New();
    //read poly1 (poly1 can be the original polygon, or the decimated polygon)
    smoother->SetInput(polydata);//RC++
    smoother->SetNumberOfIterations( m_SmoothIteration );
    smoother->SetRelaxationFactor( m_SmoothRelaxation );
    smoother->SetFeatureAngle( 60 );
    smoother->FeatureEdgeSmoothingOff();
    smoother->BoundarySmoothingOff();
    smoother->SetConvergence( 0 );

    polydata->Delete();//RC--
    polydata = smoother->GetOutput();
    polydata->Register(NULL);//RC++
    smoother->Delete();
  }

//
//#if (VTK_MAJOR_VERSION >= 5)
//  if (m_Decimate == Decimate )
//  {
//    std::cerr << "vtkDecimate not available for VTK 5.0 and above.";
//    std::cerr << " Using vtkDecimatePro instead." << std::endl;
//    m_Decimate = DecimatePro;
//  }
//#endif

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
#if (VTK_MAJOR_VERSION < 5)
  else if (m_Decimate==Decimate)
  {
    vtkDecimate *decimate = vtkDecimate::New();
    decimate->SetInput( polydata );
    decimate->PreserveTopologyOn();
    decimate->BoundaryVertexDeletionOff();
    decimate->SetTargetReduction( m_TargetReduction );
    polydata->Delete();//RC--
    polydata = decimate->GetOutput();
    polydata->Register(NULL);//RC++
    decimate->Delete();
  }
#endif

  polydata->Update();

  polydata->SetSource(NULL);

  if(polydata->GetNumberOfPoints() > 0)
  {
    mitk::Vector3D spacing = GetInput()->GetGeometry(time)->GetSpacing();

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

  gaussian->Delete();
  threshold->Delete();
}

template < typename TPixel, unsigned int VImageDimension >
    void GetAvailableLabelsInternal( itk::Image<TPixel, VImageDimension>* image, mitk::LabeledImageToSurfaceFilter::LabelMapType& availableLabels )
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::ImageRegionIterator< ImageType > ImageRegionIteratorType;
  availableLabels.clear();
  ImageRegionIteratorType it( image, image->GetLargestPossibleRegion() );
  it.GoToBegin();
  mitk::LabeledImageToSurfaceFilter::LabelMapType::iterator labelIt;
  while( ! it.IsAtEnd() )
  {
    labelIt = availableLabels.find( ( mitk::LabeledImageToSurfaceFilter::LabelType ) ( it.Get() ) );
    if ( labelIt == availableLabels.end() )
    {
      availableLabels[ ( mitk::LabeledImageToSurfaceFilter::LabelType ) ( it.Get() ) ] = 1;
    }
    else
    {
      labelIt->second += 1;
    }

    ++it;
  }
}

InstantiateAccessFunctionForFixedDimension_1(GetAvailableLabelsInternal, 3, mitk::LabeledImageToSurfaceFilter::LabelMapType&);


mitk::LabeledImageToSurfaceFilter::LabelMapType mitk::LabeledImageToSurfaceFilter::GetAvailableLabels()
{
  mitk::Image::Pointer image =  ( mitk::Image* )GetInput();
  LabelMapType availableLabels;
  AccessFixedDimensionByItk_1( image, GetAvailableLabelsInternal, 3, availableLabels );
  return availableLabels;
}


void mitk::LabeledImageToSurfaceFilter::CreateSurface(int, vtkImageData*, mitk::Surface*, const ScalarType)
{
  itkWarningMacro( "This function should never be called!" );
  assert(false);  
}

mitk::LabeledImageToSurfaceFilter::LabelType mitk::LabeledImageToSurfaceFilter::GetLabelForNthOutput( const unsigned int& idx )
{
  IdxToLabelMapType::iterator it = m_IdxToLabels.find( idx );
  if ( it != m_IdxToLabels.end() )
  {
    return it->second;
  }
  else
  {
    itkWarningMacro( "Unknown index encountered: " << idx << ". There are " << this->GetNumberOfOutputs() << " outputs available." );
    return itk::NumericTraits<LabelType>::max();
  }
}

mitk::ScalarType mitk::LabeledImageToSurfaceFilter::GetVolumeForNthOutput( const unsigned int& i )
{
  return GetVolumeForLabel( GetLabelForNthOutput( i ) );
}

mitk::ScalarType mitk::LabeledImageToSurfaceFilter::GetVolumeForLabel( const mitk::LabeledImageToSurfaceFilter::LabelType& label )
{
  // get the image spacing 
  mitk::Image* image =  ( mitk::Image* )GetInput();
  const float* spacing = image->GetSlicedGeometry()->GetFloatSpacing();
  
  // get the number of voxels encountered for the given label,
  // calculate the volume and return it.
  LabelMapType::iterator it = m_AvailableLabels.find( label );
  if ( it != m_AvailableLabels.end() )
  {
    return static_cast<float>(it->second) * ( spacing[0] * spacing[1] * spacing[2] / 1000.0f );
  }
  else
  {
    itkWarningMacro( "Unknown label encountered: " << label );
    return 0.0;
  }
}


