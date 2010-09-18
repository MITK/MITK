#include "mitkMorphologicTool.h"
#include "mitkToolManager.h"
#include "mitkRenderingManager.h"
#include "mitkBoundingObject.h"
#include "mitkImageCast.h"
#include "mitkBoundingObjectToSegmentationFilter.h"

#include "itkMaskImageFilter.h"

mitk::MorphologicTool::MorphologicTool() : Tool("dummy"),
m_Radius(0),
m_Preview(true)
{
  this->SupportRoiOn();

  m_FeedbackNode = mitk::DataNode::New();
  m_FeedbackNode->SetColor(1.0,1.0,1.0);
  m_FeedbackNode->SetName("feedback node");
  m_FeedbackNode->SetProperty("helper object", mitk::BoolProperty::New("true"));
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
      ds->Add(resultNode);
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

void mitk::MorphologicTool::SetupPreviewNodeFor(mitk::DataNode* nodeToProceed)
{
  if (nodeToProceed)
  {
    mitk::Image::Pointer image = dynamic_cast<Image*>( nodeToProceed->GetData() );
    if (image.IsNotNull())
    {
      // initialize and a new node with the same image as our reference image
      m_FeedbackNode->SetData( image );

      if (mitk::DataStorage* ds = m_ToolManager->GetDataStorage())
      {
        if (ds->Exists(m_FeedbackNode))
          ds->Remove(m_FeedbackNode);
        ds->Add( m_FeedbackNode, nodeToProceed );
      }
    }
  }
}

void mitk::MorphologicTool::Activated()
{
  m_ToolManager->RoiDataChanged += mitk::MessageDelegate<mitk::MorphologicTool>(this, &mitk::MorphologicTool::OnRoiDataChanged);

  m_OriginalNode = m_ToolManager->GetReferenceData(0);
  m_NodeToProceed = m_OriginalNode;

  if( m_NodeToProceed != NULL)
    SetupPreviewNodeFor(m_NodeToProceed);
  //if no referencedata is set deactivate tool
  else
    m_ToolManager->ActivateTool(-1);
}

void mitk::MorphologicTool::Deactivated()
{
  m_ToolManager->RoiDataChanged -= mitk::MessageDelegate<mitk::MorphologicTool>(this, &mitk::MorphologicTool::OnRoiDataChanged);

  m_NodeToProceed = NULL;

  if (mitk::DataStorage* ds = m_ToolManager->GetDataStorage())
  {
    ds->Remove(m_FeedbackNode);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  m_FeedbackNode->SetData(NULL);
}

void mitk::MorphologicTool::OnRoiDataChanged()
{
  typedef itk::Image<int, 3> ItkImageType;
  typedef itk::Image<unsigned char, 3> ItkMaskType;
  typedef itk::MaskImageFilter<ItkImageType, ItkMaskType, ItkImageType> MaskFilterType;
  mitk::DataNode* node = m_ToolManager->GetRoiData(0);
  if (node == NULL)
  {
    this->SetupPreviewNodeFor(m_OriginalNode);
    m_NodeToProceed = m_OriginalNode;
    return;
  }

  mitk::DataNode::Pointer new_node = mitk::DataNode::New();
  mitk::Image* image = dynamic_cast<mitk::Image*> (m_OriginalNode->GetData());
  mitk::Image::Pointer new_image = mitk::Image::New();

  mitk::Image::Pointer roi;
  mitk::BoundingObject* boundingObject = dynamic_cast<mitk::BoundingObject*> (node->GetData());

  if (boundingObject)
  {
    mitk::BoundingObjectToSegmentationFilter::Pointer filter = mitk::BoundingObjectToSegmentationFilter::New();
    filter->SetBoundingObject( boundingObject);
    filter->SetInput(image);
    filter->Update();
    roi = filter->GetOutput();
  }
  else
    roi =  dynamic_cast<mitk::Image*> (node->GetData());

  if (roi)
  {
    MaskFilterType::Pointer filter = MaskFilterType::New();
    ItkMaskType::Pointer itkRoi = ItkMaskType::New();
    ItkImageType::Pointer itkImage = ItkImageType::New();
    mitk::CastToItkImage(image, itkImage);
    mitk::CastToItkImage(roi, itkRoi);
    filter->SetInput1(itkImage);
    filter->SetInput2(itkRoi);
    filter->SetOutsideValue(0);
    filter->Update();
    mitk::CastToMitkImage(filter->GetOutput(),new_image);
  }
  new_node->SetData(new_image);

  this->SetupPreviewNodeFor(new_node);
  m_NodeToProceed = new_node;

  UpdatePreview();
}