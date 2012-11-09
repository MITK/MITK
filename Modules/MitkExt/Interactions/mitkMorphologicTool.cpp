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
#include "mitkMorphologicTool.h"
#include "mitkToolManager.h"
#include "mitkRenderingManager.h"
#include "mitkBoundingObject.h"
#include "mitkImageCast.h"

#include "mitkMaskAndCutRoiImageFilter.h"

mitk::MorphologicTool::MorphologicTool() : Tool("dummy"),
m_Radius(0),
m_Preview(true),
m_StructElement(BALL)
{
  this->SupportRoiOn();

  m_FeedbackNode = mitk::DataNode::New();
  m_FeedbackNode->SetColor(1.0,1.0,1.0);
  m_FeedbackNode->SetName("feedback node");
  m_FeedbackNode->SetProperty("helper object", mitk::BoolProperty::New("true"));
  m_FeedbackNode->SetProperty("opacity", mitk::FloatProperty::New(1.0));
}

mitk::MorphologicTool::~MorphologicTool()
{

}

const char* mitk::MorphologicTool::GetGroup() const
{
  return "morphologic";
}

void mitk::MorphologicTool::AcceptPreview(const std::string& name, const Color& color)
{
  mitk::DataNode::Pointer resultNode = mitk::DataNode::New();

  mitk::Image::Pointer image;
  if (m_Preview)
    image = dynamic_cast<Image*>( m_FeedbackNode->GetData() );
  else
    image = ApplyFilter(dynamic_cast<Image*> (m_NodeToProceed->GetData()));

  if (image.IsNotNull())
  {
    resultNode->SetData(image);
    resultNode->SetName(name);
    resultNode->SetColor(color);

    if (mitk::DataStorage* ds = m_ToolManager->GetDataStorage())
    {
      ds->Add(resultNode, m_OriginalNode);
    }
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  m_ToolManager->ActivateTool(-1);
}

void mitk::MorphologicTool::CancelPreviewing()
{
  m_ToolManager->ActivateTool(-1);
}

void mitk::MorphologicTool::SetRadius(unsigned int value)
{
  m_Radius = value;
  this->UpdatePreview();
}

void mitk::MorphologicTool::UpdatePreview()
{
  mitk::DataNode::Pointer node = m_ToolManager->GetRoiData(0);

  if (node.IsNotNull())
  {
    mitk::DataNode::Pointer new_node = mitk::DataNode::New();
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*> (m_NodeToProceed->GetData());
    mitk::Image::Pointer maskedImage = mitk::Image::New();

    mitk::MaskAndCutRoiImageFilter::Pointer roiFilter = mitk::MaskAndCutRoiImageFilter::New();
    roiFilter->SetInput(image);
    roiFilter->SetRegionOfInterest(node->GetData());
    roiFilter->Update();

    maskedImage = roiFilter->GetOutput();
    new_node->SetData(maskedImage);

    m_NodeToProceed = new_node;

  }
  else
    m_NodeToProceed = m_OriginalNode;

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( m_NodeToProceed->GetData() );

  if (image && m_Preview)
  {
    if (m_Radius == 0)
      m_FeedbackNode->SetData(image);
    else
    {
      m_FeedbackNode->SetData(ApplyFilter(image));
    }
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

mitk::Image::Pointer mitk::MorphologicTool::ApplyFilter(mitk::Image::Pointer image)
{
  return image;
}

void mitk::MorphologicTool::Activated()
{
  m_ToolManager->RoiDataChanged += mitk::MessageDelegate<mitk::MorphologicTool>(this, &mitk::MorphologicTool::UpdatePreview);

  m_OriginalNode = m_ToolManager->GetReferenceData(0);
  m_NodeToProceed = m_OriginalNode;

  if( m_NodeToProceed.IsNotNull())
  {
    if (mitk::DataStorage* ds = m_ToolManager->GetDataStorage())
    {
      if (!ds->Exists(m_FeedbackNode))
        ds->Add( m_FeedbackNode, m_OriginalNode );
    }
  }
  //if no referencedata is set deactivate tool
  else
    m_ToolManager->ActivateTool(-1);
}

void mitk::MorphologicTool::Deactivated()
{
  m_ToolManager->RoiDataChanged -= mitk::MessageDelegate<mitk::MorphologicTool>(this, &mitk::MorphologicTool::UpdatePreview);

  if (mitk::DataStorage* ds = m_ToolManager->GetDataStorage())
  {
    ds->Remove(m_FeedbackNode);
  }

  m_NodeToProceed = NULL;
  m_OriginalNode = NULL;
  m_FeedbackNode->SetData(NULL);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::MorphologicTool::SetStructuringElementType(int id)
{
  if (id==2)
    m_StructElement = CROSS;
  else
    m_StructElement = BALL;

  this->UpdatePreview();
}
