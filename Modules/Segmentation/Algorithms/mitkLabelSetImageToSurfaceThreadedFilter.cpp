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
/*
  m_Filter = mitk::LabelSetImageToSurfaceFilter::New();
  m_Filter->SetInput(image);
//  m_Filter->SetObserver(obsv);
  m_Filter->SetGenerateAllLabels( false );
  m_Filter->SetRequestedLabel( m_RequestedLabel );
*/
  mitk::LabelSetImageToSurfaceFilter::Pointer filter = mitk::LabelSetImageToSurfaceFilter::New();
  filter->SetInput(image);
//  filter->SetObserver(obsv);
  filter->SetGenerateAllLabels( false );
  filter->SetRequestedLabel( m_RequestedLabel );

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

  m_ResultSurface = filter->GetOutput();

  if ( m_ResultSurface.IsNull() || !m_ResultSurface->GetVtkPolyData() )
     return false;

//  m_ResultSurface->DisconnectPipeline();

  return true;
}

void LabelSetImageToSurfaceThreadedFilter::ThreadedUpdateSuccessful()
{
   LabelSetImage::Pointer image;
   this->GetPointerParameter("Input", image);

   std::string name = image->GetLabelName(m_RequestedLabel);
   name.append("-surf");

  // mitk::Surface::Pointer surface = m_ResultSurface;
  // surface->DisconnectPipeline();

   mitk::DataNode::Pointer node = mitk::DataNode::New();
   node->SetData(m_ResultSurface);
   node->SetName(name);

   mitk::Color color = image->GetLabelColor(m_RequestedLabel);
   node->SetColor(color);

   this->GetDataStorage()->Add(node);

   Superclass::ThreadedUpdateSuccessful();
}

} // namespace
