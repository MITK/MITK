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

LabelSetImageToSurfaceThreadedFilter::LabelSetImageToSurfaceThreadedFilter(): m_RequestedLabel(1)
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
  try
  {
    Image::Pointer image;
    this->GetPointerParameter("Input", image);

    if (image.IsNull())
       return false;

//    ProcessObserver::Pointer obsv;
//    this->GetPointerParameter("Observer", obsv);

//    if (obsv.IsNull())
//       return false;

  }
  catch (std::invalid_argument&)
  {
    return false;
  }

  return true;
}


bool LabelSetImageToSurfaceThreadedFilter::ThreadedUpdateFunction()
{
  MITK_INFO << "ThreadedUpdateFunction 1";

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
  try
  {
    this->GetParameter("RequestedLabel", m_RequestedLabel);
  }
  catch (std::invalid_argument&)
  {
     MITK_WARN << "\"RequestedLabel\" parameter was not set: will use the default value (" << m_RequestedLabel << ").";
  }

  m_Filter = mitk::LabelSetImageToSurfaceFilter::New();
  m_Filter->SetInput(image);
//  m_Filter->SetObserver(obsv);
  m_Filter->SetGenerateAllLabels( false );
  m_Filter->SetRequestedLabel( m_RequestedLabel );

  try
  {
     m_Filter->Update();
  }
  catch (itk::ExceptionObject& exception)
  {
     MITK_ERROR << "Exception caught: " << exception.GetDescription();
     return false;
  }
  catch (std::exception& exception)
  {
     MITK_ERROR << "Exception caught: " << exception.what();
     return false;
  }
  catch (...)
  {
     MITK_ERROR << "Unknown exception caught";
     return false;
  }

  if (!m_Filter->GetDataIsAvailable())
  {
     MITK_ERROR << "LabelSetImageToSurfaceThreadedFilter failed";
     return false;
  }

  return true;
}

void LabelSetImageToSurfaceThreadedFilter::ThreadedUpdateSuccessful()
{
   MITK_INFO << "ThreadedUpdateSuccessful 1";

   LabelSetImage::Pointer image;
   this->GetPointerParameter("Input", image);

   std::string name = image->GetLabelName(m_RequestedLabel);
   name.append("-surf");

   mitk::Surface::Pointer surface = m_Filter->GetOutput(0);
   surface->DisconnectPipeline();

   mitk::DataNode::Pointer node = mitk::DataNode::New();
   node->SetData(surface);
   node->SetName(name);

   mitk::Color color = image->GetLabelColor(m_RequestedLabel);
   node->SetColor(color);

   this->GetDataStorage()->Add(node);

   Superclass::ThreadedUpdateSuccessful();
}

} // namespace
