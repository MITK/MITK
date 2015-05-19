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

#include "mitkLabelSetImageToSurfaceThreadedFilter.h"

#include "mitkLabelSetImageToSurfaceFilter.h"
#include "mitkLabelSetImage.h"

namespace mitk
{

LabelSetImageToSurfaceThreadedFilter::LabelSetImageToSurfaceThreadedFilter():
m_RequestedLabel(1),
m_Result(nullptr)
{
}

LabelSetImageToSurfaceThreadedFilter::~LabelSetImageToSurfaceThreadedFilter()
{

}

void LabelSetImageToSurfaceThreadedFilter::Initialize(const NonBlockingAlgorithm* other)
{
  Superclass::Initialize(other);
}

bool LabelSetImageToSurfaceThreadedFilter::ReadyToRun()
{
  Image::Pointer image;
  GetPointerParameter("Input", image);

  return image.IsNotNull() && GetGroupNode();
}

bool LabelSetImageToSurfaceThreadedFilter::ThreadedUpdateFunction()
{
  LabelSetImage::Pointer image;
  this->GetPointerParameter("Input", image);

//  ProcessObserver::Pointer obsv;
/*
  try
  {
    this->GetPointerParameter("Observer", obsv);
  }
  catch (std::invalid_argument&)
  {
//    MITK_WARN << "None observer provided.";
  }
*/
  bool useSmoothing(false);
  try
  {
    this->GetParameter("Smooth", useSmoothing);
  }
  catch(std::invalid_argument&)
  {
    MITK_WARN << "\"Smooth\" parameter was not set: will use the default value (" << useSmoothing << ").";
  }

  try
  {
    this->GetParameter("RequestedLabel", m_RequestedLabel);
  }
  catch (std::invalid_argument&)
  {
     MITK_WARN << "\"RequestedLabel\" parameter was not set: will use the default value (" << m_RequestedLabel << ").";
  }

  mitk::LabelSetImageToSurfaceFilter::Pointer filter = mitk::LabelSetImageToSurfaceFilter::New();
  filter->SetInput(image);
//  filter->SetObserver(obsv);
  filter->SetGenerateAllLabels( false );
  filter->SetRequestedLabel( m_RequestedLabel );
  filter->SetUseSmoothing(useSmoothing);

  try
  {
    filter->Update();
  }
  catch (itk::ExceptionObject& e)
  {
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    return false;
  }
  catch (std::exception& e)
  {
     MITK_ERROR << "Exception caught: " << e.what();
     return false;
  }
  catch (...)
  {
     MITK_ERROR << "Unknown exception caught";
     return false;
  }

  m_Result = filter->GetOutput();

  if ( m_Result.IsNull() || !m_Result->GetVtkPolyData() )
     return false;

  m_Result->DisconnectPipeline();

  return true;
}

void LabelSetImageToSurfaceThreadedFilter::ThreadedUpdateSuccessful()
{
  LabelSetImage::Pointer image;
  this->GetPointerParameter("Input", image);

  std::string name = this->GetGroupNode()->GetName();
  name.append("-surf");

  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetData(m_Result);
  node->SetName(name);

  mitk::Color color = image->GetLabel(m_RequestedLabel)->GetColor();
  node->SetColor(color);

  this->InsertBelowGroupNode(node);

  Superclass::ThreadedUpdateSuccessful();
}

} // namespace
