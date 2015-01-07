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

#include "mitkContourModelSetToImageFilter.h"

#include <mitkExtractSliceFilter.h>
#include <mitkContourModelSet.h>
#include <mitkContourModelUtils.h>
#include <mitkImageWriteAccessor.h>
#include <mitkProgressBar.h>
#include <mitkTimeHelper.h>

// TODO Move this one to Core so that this Filter can be in the ContourModel Module!
#include <mitkVtkImageOverwrite.h>
// TODO


mitk::ContourModelSetToImageFilter::ContourModelSetToImageFilter()
  : m_MakeOutputBinary( true ),
    m_TimeStep( 0 )
{
  // Create the output.
  itk::DataObject::Pointer output = this->MakeOutput(0);
  Superclass::SetNumberOfRequiredInputs(1);
  Superclass::SetNumberOfRequiredOutputs(1);
  Superclass::SetNthOutput(0, output);
}

mitk::ContourModelSetToImageFilter::~ContourModelSetToImageFilter()
{
}

void mitk::ContourModelSetToImageFilter::GenerateInputRequestedRegion()
{
  mitk::Image* output = this->GetOutput();
  if((output->IsInitialized()==false) )
    return;

  GenerateTimeInInputRegion(output, const_cast< mitk::Image * > ( this->GetImage() ));
}

void mitk::ContourModelSetToImageFilter::GenerateOutputInformation()
{
  mitk::Image *inputImage = (mitk::Image*)this->GetImage();
  mitk::Image::Pointer output = this->GetOutput();

  itkDebugMacro(<<"GenerateOutputInformation()");

  if((inputImage == NULL) ||
     (inputImage->IsInitialized() == false) ||
     (inputImage->GetTimeGeometry() == NULL)) return;

  if (m_MakeOutputBinary)
  {
    output->Initialize(mitk::MakeScalarPixelType<unsigned char>() , *inputImage->GetTimeGeometry(),1,1);
  }
  else
  {
    output->Initialize(inputImage->GetPixelType(), *inputImage->GetTimeGeometry());
  }

  output->SetPropertyList(inputImage->GetPropertyList()->Clone());
}

itk::DataObject::Pointer mitk::ContourModelSetToImageFilter::MakeOutput ( DataObjectPointerArraySizeType /*idx*/ )
{
  return OutputType::New().GetPointer();
}

itk::DataObject::Pointer mitk::ContourModelSetToImageFilter::MakeOutput( const DataObjectIdentifierType & name )
{
  itkDebugMacro("MakeOutput(" << name << ")");
  if( this->IsIndexedOutputName(name) )
  {
    return this->MakeOutput( this->MakeIndexFromOutputName(name) );
  }
  return OutputType::New().GetPointer();
}

const mitk::ContourModelSet* mitk::ContourModelSetToImageFilter::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
  {
    return 0;
  }

  return static_cast< const mitk::ContourModelSet* >
    ( this->ProcessObject::GetInput(0) );
}

void mitk::ContourModelSetToImageFilter::SetInput(const ContourModelSet *input)
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(0,
    const_cast< mitk::ContourModelSet* >( input ) );
}

void mitk::ContourModelSetToImageFilter::SetImage(const mitk::Image *refImage)
{
  this->ProcessObject::SetNthInput( 1, const_cast< mitk::Image * >( refImage ) );
}

const mitk::Image* mitk::ContourModelSetToImageFilter::GetImage(void)
{
  return static_cast< const mitk::Image * >(this->ProcessObject::GetInput(1));
}

void mitk::ContourModelSetToImageFilter::GenerateData()
{
  mitk::ContourModelSet* contourSet = const_cast<mitk::ContourModelSet*>(this->GetInput());

  // Assure that the volume data of the output is set (fill volume with zeros)
  this->InitializeOutputEmpty();

  mitk::Image::Pointer outputImage = const_cast<mitk::Image*>(this->GetOutput());


  if(outputImage.IsNull() || outputImage->IsInitialized() == false || !outputImage->IsVolumeSet(m_TimeStep))
  {
    MITK_ERROR<<"Error creating output for specified image!";
    return;
  }

  if (!contourSet || contourSet->GetContourModelList()->size() == 0)
  {
    MITK_ERROR<<"No contours specified!";
    return;
  }

  mitk::Geometry3D* outputImageGeo = outputImage->GetGeometry(m_TimeStep);

  // Create mitkVtkImageOverwrite which is needed to write the slice back into the volume
  vtkSmartPointer<mitkVtkImageOverwrite> reslice = vtkSmartPointer<mitkVtkImageOverwrite>::New();

  // Create ExtractSliceFilter for extracting the corresponding slices from the volume
  mitk::ExtractSliceFilter::Pointer extractor = mitk::ExtractSliceFilter::New(reslice);
  extractor->SetInput( outputImage );
  extractor->SetTimeStep( m_TimeStep );
  extractor->SetResliceTransformByGeometry( outputImageGeo );

  // Fill each contour of the contourmodelset into the image
  mitk::ContourModelSet::ContourModelSetIterator it = contourSet->Begin();
  mitk::ContourModelSet::ContourModelSetIterator end = contourSet->End();
  while (it != end)
  {
    mitk::ContourModel* contour = it->GetPointer();

    // 1. Create slice geometry using the contour points
    mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();
    mitk::Point3D point3D, tempPoint;
    mitk::Vector3D normal;

    mitk::Image::Pointer slice;

    int sliceIndex;
    bool isFrontside = true;
    bool isRotated = false;

    // Determine plane orientation
    point3D = contour->GetVertexAt(0)->Coordinates;
    tempPoint = contour->GetVertexAt(contour->GetNumberOfVertices()*0.5)->Coordinates;
    mitk::Vector3D vec = point3D - tempPoint;
    vec.Normalize();
    outputImageGeo->WorldToIndex(point3D, point3D);

    mitk::PlaneGeometry::PlaneOrientation orientation;
    if (mitk::Equal(vec[0], 0))
    {
      orientation = mitk::PlaneGeometry::Sagittal;
      sliceIndex = point3D[0];
    }
    else if (mitk::Equal(vec[1], 0))
    {
      orientation = mitk::PlaneGeometry::Frontal;
      sliceIndex = point3D[1];
    }
    else if (mitk::Equal(vec[2], 0))
    {
      orientation = mitk::PlaneGeometry::Axial;
      sliceIndex = point3D[2];
    }
    else
    {
      // TODO Maybe rotate geometry to extract slice?
      MITK_ERROR<<"Cannot detect correct slice number! Only axial, sagittal and frontal oriented contours are supported!";
      return;
    }

    // Initialize plane using the detected orientation
    plane->InitializeStandardPlane(outputImageGeo, orientation, sliceIndex, isFrontside, isRotated);
    point3D = plane->GetOrigin();
    normal = plane->GetNormal();
    normal.Normalize();
    point3D += normal * 0.5;//pixelspacing is 1, so half the spacing is 0.5
    plane->SetOrigin(point3D);

    // 2. Extract slice at the given position
    extractor->SetWorldGeometry( plane );
    extractor->SetVtkOutputRequest(false);
    reslice->SetOverwriteMode(false);

    extractor->Modified();
    extractor->Update();

    slice = extractor->GetOutput();

    // 3. Fill contour into slice
    mitk::ContourModel::Pointer projectedContour = mitk::ContourModelUtils::ProjectContourTo2DSlice(slice, contour, true, false);
    mitk::ContourModelUtils::FillContourInSlice(projectedContour, slice);

    // 4. Write slice back into image volume
    reslice->SetInputSlice(slice->GetVtkImageData());

    //set overwrite mode to true to write back to the image volume
    reslice->SetOverwriteMode(true);
    reslice->Modified();

    extractor->SetVtkOutputRequest(true);
    extractor->Modified();
    extractor->Update();

    mitk::ProgressBar::GetInstance()->Progress();
    ++it;
  }

  outputImage->Modified();
  outputImage->GetVtkImageData()->Modified();
}

void mitk::ContourModelSetToImageFilter::InitializeOutputEmpty()
{
  // Initialize the output's volume with zeros
  mitk::Image* output = this->GetOutput();
  unsigned int byteSize = output->GetPixelType().GetSize();

  if(output->GetDimension() < 4)
  {
    for (unsigned int dim = 0; dim < output->GetDimension(); ++dim)
    {
      byteSize *= output->GetDimension(dim);
    }

    mitk::ImageWriteAccessor writeAccess(output, output->GetVolumeData(0));

    memset( writeAccess.GetData(), 0, byteSize );
  }
  else
  {
    //if we have a time-resolved image we need to set memory to 0 for each time step
    for (unsigned int dim = 0; dim < 3; ++dim)
    {
      byteSize *= output->GetDimension(dim);
    }

    for( unsigned int volumeNumber = 0; volumeNumber < output->GetDimension(3); volumeNumber++)
    {
      mitk::ImageWriteAccessor writeAccess(output, output->GetVolumeData(volumeNumber));

      memset( writeAccess.GetData(), 0, byteSize );
    }
  }
}
