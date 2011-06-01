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

#include <mitkToFVisualizationFilter.h>
#include <mitkInstantiateAccessFunctions.h>
#include <mitkIpPic.h>

#include <vtkPolyDataMapper.h>
#include <itkImage.h>
#include <vtkCamera.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <mitkVtkRepresentationProperty.h>

#include <math.h>

mitk::ToFVisualizationFilter::ToFVisualizationFilter()
{
  this->m_ImageWidth = 0;
  this->m_ImageHeight = 0;
  this->m_ImageSize = 0;

  this->m_MitkDistanceImage = NULL;
  this->m_MitkAmplitudeImage = NULL;
  this->m_MitkIntensityImage = NULL;

  this->m_Widget1ColorTransferFunction = NULL;
  this->m_Widget2ColorTransferFunction = NULL;
  this->m_Widget3ColorTransferFunction = NULL;

  SetImageType(this->m_Widget1ImageType, 0);
  SetImageType(this->m_Widget2ImageType, 1);
  SetImageType(this->m_Widget3ImageType, 2);

  OutputImageType::Pointer output0 = static_cast<OutputImageType*>(this->MakeOutput(0).GetPointer());
  OutputImageType::Pointer output1 = static_cast<OutputImageType*>(this->MakeOutput(1).GetPointer());
  OutputImageType::Pointer output2 = static_cast<OutputImageType*>(this->MakeOutput(2).GetPointer());

  SetNumberOfRequiredOutputs(3);
  SetNthOutput(0, output0.GetPointer());
  SetNthOutput(1, output1.GetPointer());
  SetNthOutput(2, output2.GetPointer());

}

mitk::ToFVisualizationFilter::~ToFVisualizationFilter()
{

}

void mitk::ToFVisualizationFilter::SetInput(  mitk::Image* distanceImage )
{
  this->SetInput(0, distanceImage);
}

//TODO: braucht man diese Methode?
void mitk::ToFVisualizationFilter::SetInput( unsigned int idx,  mitk::Image* image )
{
  if ((image == NULL) && (idx == this->GetNumberOfInputs() - 1)) // if the last input is set to NULL, reduce the number of inputs by one
    this->SetNumberOfInputs(this->GetNumberOfInputs() - 1);
  else
    this->ProcessObject::SetNthInput(idx, image);   // Process object is not const-correct so the const_cast is required here

  this->CreateOutputsForAllInputs();

  if (idx == 0)
  {
    this->m_ImageWidth = image->GetDimension(0);
    this->m_ImageHeight = image->GetDimension(1);
    this->m_ImageSize = this->m_ImageWidth * this->m_ImageHeight * sizeof(float);
  }
  CheckTransferFunction(idx, image);
}

 mitk::Image* mitk::ToFVisualizationFilter::GetInput()
{
  return this->GetInput(0);
}

 mitk::Image* mitk::ToFVisualizationFilter::GetInput( unsigned int idx )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL; //TODO: geeignete exception werfen
  return static_cast< mitk::Image*>(this->ProcessObject::GetInput(idx));
}

void mitk::ToFVisualizationFilter::GenerateData()
{
  this->m_MitkDistanceImage = this->GetInput(0);
  this->m_MitkAmplitudeImage = this->GetInput(1);
  this->m_MitkIntensityImage = this->GetInput(2);

  //this->m_DistanceFloatData = (float*)this->m_MitkDistanceImage->GetData();
  //this->m_AmplitudeFloatData = (float*)this->m_MitkAmplitudeImage->GetData();
  //this->m_IntensityFloatData = (float*)this->m_MitkIntensityImage->GetData();

  mitk::Image::Pointer outputImageWidget1 = this->GetOutput(0);
  mitk::Image::Pointer outputImageWidget2 = this->GetOutput(1);
  mitk::Image::Pointer outputImageWidget3 = this->GetOutput(2);

  //colorTransferFunction = m_Controls->m_ToFVisualisationSettingsWidget->GetWidget3ColorTransferFunction();
  //imageType = m_Controls->m_ToFVisualisationSettingsWidget->GetWidget3ImageType();
  //RenderWidget(m_MultiWidget->mitkWidget3, this->m_QmitkToFImageBackground3, this->m_Widget3ImageType, imageType,
  //  colorTransferFunction, videoTexture, this->m_Widget3Texture );

  //outputImageWidget1->Initialize(input->GetPixelType(), *input->GetTimeSlicedGeometry());
  //outputImageWidget1->SetPropertyList(input->GetPropertyList()->Clone());
  InitImage(outputImageWidget1);

  float* floatData;
  unsigned char* image;
  image = (unsigned char*)outputImageWidget1->GetData();
  floatData = GetDataFromImageByImageType(this->m_Widget1ImageType);
  ConvertFloatImageToRGBImage(this->m_Widget1ColorTransferFunction, floatData, image);
  // copy input 0...n to output 0...n
  for (unsigned int idx=0; idx<this->GetNumberOfOutputs(); idx++)
  {
    mitk::Image::Pointer outputImage = this->GetOutput(idx);
    mitk::Image::Pointer inputImage = this->GetInput(idx);
    if (outputImage.IsNotNull()&&inputImage.IsNotNull())
    {
      outputImage->CopyInformation(inputImage);
      outputImage->Initialize(inputImage);
      outputImage->SetSlice(inputImage->GetSliceData()->GetData());
    }
  }
}

void mitk::ToFVisualizationFilter::InitImage(mitk::Image::Pointer image)
{
  unsigned int dimensions[2];
  dimensions[0] = this->m_ImageWidth;
  dimensions[1] = this->m_ImageHeight;
  image->Initialize(mitk::PixelType(mitkIpPicUInt, 24, 3), 2, dimensions); //unsigned char RGB
}

void mitk::ToFVisualizationFilter::CreateOutputsForAllInputs()
{
  this->SetNumberOfOutputs(this->GetNumberOfInputs());  // create outputs for all inputs
  for (unsigned int idx = 0; idx < this->GetNumberOfOutputs(); ++idx)
    if (this->GetOutput(idx) == NULL)
    {
      DataObjectPointer newOutput = this->MakeOutput(idx);
      this->SetNthOutput(idx, newOutput);
    }
    this->Modified();
}

void mitk::ToFVisualizationFilter::GenerateOutputInformation()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  if (output->IsInitialized())
    return;

  itkDebugMacro(<<"GenerateOutputInformation()");

  output->Initialize(input->GetPixelType(), *input->GetTimeSlicedGeometry());
  output->SetPropertyList(input->GetPropertyList()->Clone());
}

vtkColorTransferFunction* mitk::ToFVisualizationFilter::GetWidget1ColorTransferFunction()
{
  return this->m_Widget1ColorTransferFunction;
}

void mitk::ToFVisualizationFilter::SetWidget1ColorTransferFunction(vtkColorTransferFunction* colorTransferFunction)
{
  this->m_Widget1ColorTransferFunction = colorTransferFunction;
}

vtkColorTransferFunction* mitk::ToFVisualizationFilter::GetWidget2ColorTransferFunction()
{
  return this->m_Widget2ColorTransferFunction;
}

void mitk::ToFVisualizationFilter::SetWidget2ColorTransferFunction(vtkColorTransferFunction* colorTransferFunction)
{
  m_Widget2ColorTransferFunction = colorTransferFunction;
}

vtkColorTransferFunction* mitk::ToFVisualizationFilter::GetWidget3ColorTransferFunction()
{
  return this->m_Widget3ColorTransferFunction;
}

void mitk::ToFVisualizationFilter::SetWidget3ColorTransferFunction(vtkColorTransferFunction* colorTransferFunction)
{
  this->m_Widget3ColorTransferFunction = colorTransferFunction;
}

//vtkColorTransferFunction* mitk::ToFVisualizationFilter::GetWidget4ColorTransferFunction()
//{
//  return this->m_Widget4ColorTransferFunction;
//}
//
//void mitk::ToFVisualizationFilter::SetWidget4ColorTransferFunction(vtkColorTransferFunction* colorTransferFunction)
//{
//  this->m_Widget4ColorTransferFunction = colorTransferFunction;
//}

std::string mitk::ToFVisualizationFilter::GetWidget1ImageType()
{
  return this->m_Widget1ImageType;
}

void mitk::ToFVisualizationFilter::SetWidget1ImageType(std::string imageType)
{
  this->m_Widget1ImageType = imageType;
}

std::string mitk::ToFVisualizationFilter::GetWidget2ImageType()
{
  return this->m_Widget2ImageType;
}

void mitk::ToFVisualizationFilter::SetWidget2ImageType(std::string imageType)
{
  this->m_Widget2ImageType = imageType;
}

std::string mitk::ToFVisualizationFilter::GetWidget3ImageType()
{
  return this->m_Widget3ImageType;
}

void mitk::ToFVisualizationFilter::SetWidget3ImageType(std::string imageType)
{
  this->m_Widget3ImageType = imageType;
}

int mitk::ToFVisualizationFilter::GetWidget1TransferFunctionType()
{
  return this->m_Widget1TransferFunctionType;
}

void mitk::ToFVisualizationFilter::SetWidget1TransferFunctionType(int transferFunctionType)
{
  this->m_Widget1TransferFunctionType = transferFunctionType;
}

int mitk::ToFVisualizationFilter::GetWidget2TransferFunctionType()
{
  return this->m_Widget2TransferFunctionType;
}

void mitk::ToFVisualizationFilter::SetWidget2TransferFunctionType(int transferFunctionType)
{
  this->m_Widget2TransferFunctionType = transferFunctionType;
}

int mitk::ToFVisualizationFilter::GetWidget3TransferFunctionType()
{
  return this->m_Widget3TransferFunctionType;
}

void mitk::ToFVisualizationFilter::SetWidget3TransferFunctionType(int transferFunctionType)
{
  this->m_Widget3TransferFunctionType = transferFunctionType;
}

vtkColorTransferFunction* mitk::ToFVisualizationFilter::GetColorTransferFunctionByImageType(std::string imageType)
{
  if (this->m_Widget1ImageType.compare(imageType) == 0)
  {
    return this->m_Widget1ColorTransferFunction;
  }
  else if (this->m_Widget2ImageType.compare(imageType) == 0)
  {
    return this->m_Widget2ColorTransferFunction;
  }
  else if (this->m_Widget3ImageType.compare(imageType) == 0)
  {
    return this->m_Widget3ColorTransferFunction;
  }
  else
  {
    return this->m_Widget3ColorTransferFunction;
  }
}

void mitk::ToFVisualizationFilter::CheckTransferFunction(int idx, mitk::Image* image)
{
  if (idx == 0)
  {
    if (this->m_Widget1ColorTransferFunction == NULL )
    {
      InitializeTransferFunction(idx, image);
    }
  }
  else if (idx == 1)
  {
    if (this->m_Widget2ColorTransferFunction == NULL )
    {
      InitializeTransferFunction(idx, image);
    }
  }
  else if (idx == 2)
  {
    if (this->m_Widget3ColorTransferFunction == NULL )
    {
      InitializeTransferFunction(idx, image);
    }
  }
}

void mitk::ToFVisualizationFilter::InitializeTransferFunction(int idx, mitk::Image* image)
{
  int min, max;
  if (idx == 0)
  {
    if (image)
    {
      min = image->GetScalarValueMin();
      max = image->GetScalarValueMax();
    }
    else
    {
      min = 0;
      max = 7000;
    }
    this->m_Widget1ColorTransferFunction = vtkColorTransferFunction::New();
    this->m_Widget1TransferFunctionType = 1;
    ResetTransferFunction(this->m_Widget1ColorTransferFunction, this->m_Widget1TransferFunctionType, min, max);
  }
  else if (idx == 1)
  {
    if (image)
    {
      min = image->GetScalarValueMin();
      max = image->GetScalarValueMax();
    }
    else
    {
      min = 0;
      max = 20000;
    }
    this->m_Widget2ColorTransferFunction = vtkColorTransferFunction::New();
    this->m_Widget2TransferFunctionType = 0;
    ResetTransferFunction(this->m_Widget2ColorTransferFunction, this->m_Widget2TransferFunctionType, min, max);
  }
  else if (idx == 2)
  {
    if (image)
    {
      min = image->GetScalarValueMin();
      max = image->GetScalarValueMax();
    }
    else
    {
      min = 0;
      max = 20000;
    }
    this->m_Widget3ColorTransferFunction = vtkColorTransferFunction::New();
    this->m_Widget3TransferFunctionType = 0;
    ResetTransferFunction(this->m_Widget3ColorTransferFunction, this->m_Widget3TransferFunctionType, min, max);
  }
}

void mitk::ToFVisualizationFilter::SetImageType(std::string& str, int index)
{
  if (index == 0)
  {
    str = std::string("Distance");
  }
  else if (index == 1)
  {
    str = std::string("Amplitude");
  }
  else if (index == 2)
  {
    str = std::string("Intensity");
  }
  else if (index == 3)
  {
    str = std::string("Video");
  }
  else if (index == 5)
  {
    str = std::string("Surface");
  }
}

void mitk::ToFVisualizationFilter::ResetTransferFunction(vtkColorTransferFunction* colorTransferFunction, int type, double min, double max)
{
  colorTransferFunction->RemoveAllPoints();
  if (type == 0)
  {
    colorTransferFunction->AddRGBPoint(min, 0, 0, 0);
    colorTransferFunction->AddRGBPoint(max, 1, 1, 1);
  }
  else
  {
    if (min>0.01)
    {
      colorTransferFunction->AddRGBPoint(0.0, 0, 0, 0);
      colorTransferFunction->AddRGBPoint(min-0.01, 0, 0, 0);
    }
    colorTransferFunction->AddRGBPoint(min, 1, 0, 0);
    colorTransferFunction->AddRGBPoint(min+(max-min)/2, 1, 1, 0);
    colorTransferFunction->AddRGBPoint(max, 0, 0, 1);
  }
  colorTransferFunction->SetColorSpaceToHSV();
}

float* mitk::ToFVisualizationFilter::GetDataFromImageByImageType(std::string imageType)
{
  void* data;
  if (imageType.compare("Distance")==0)
  {
    data = this->m_MitkDistanceImage->GetData();
  }
  else if (imageType.compare("Amplitude")==0)
  {
    data = this->m_MitkAmplitudeImage->GetData();
  }
  if (imageType.compare("Intensity")==0)
  {
    data = this->m_MitkIntensityImage->GetData();
  }
  return (float*)data;
}

void mitk::ToFVisualizationFilter::ConvertFloatImageToRGBImage(vtkColorTransferFunction* colorTransferFunction, float* floatData, unsigned char* image, bool flip)
{
  vtkFloatArray* floatArrayDist;
  floatArrayDist = vtkFloatArray::New();
  floatArrayDist->Initialize();
  int numOfPixel = this->m_ImageWidth * this->m_ImageHeight;
  float* flippedFloatData;

  if (flip)
  {
    flippedFloatData = new float[numOfPixel];
    for (int i=0; i<this->m_ImageHeight; i++)
    {
      for (int j=0; j<this->m_ImageWidth; j++)
      {
        flippedFloatData[i*this->m_ImageWidth+j] = floatData[((this->m_ImageHeight-1-i)*this->m_ImageWidth)+j];
      }
    }
    floatArrayDist->SetArray(flippedFloatData, numOfPixel, 0);
  } else
  {
    floatArrayDist->SetArray(floatData, numOfPixel, 0);
  }

  colorTransferFunction->MapScalarsThroughTable(floatArrayDist, image, VTK_RGB);

  if (flip)
  {
    delete[] flippedFloatData;
  }
}
