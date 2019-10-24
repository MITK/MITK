#include "mitkResectionTool.h"

#include <vtkCamera.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

#include <itkMaskImageFilter.h>

#include <mitkImageAccessByItk.h>
#include <mitkImageCaster.h>
#include <mitkImageTimeSelector.h>
#include <mitkITKImageImport.h>
#include <mitkProgressBar.h>
#include <mitkResectionMaskFilter.h>
#include <mitkSurfaceUtils.h>
#include <mitkToolManager.h>

namespace mitk
{
MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, ResectionTool, "Resection tool");

ResectionTool::ResectionTool() : ContourTool(),
  m_State(ResectionState::NO_CONTOUR)
{
  SegTool2D::SetAllow3dMapper(true);
  FeedbackContourTool::SetFeedbackContourDraw3D(true);
  Tool::setLock3D(true);
}

ResectionTool::~ResectionTool()
{
}

#define QT_TRANSLATE_NOOP(x,y) y
const char* ResectionTool::GetName() const
{
  return QT_TRANSLATE_NOOP("SegmentationTool", "Resection");
}

const char** ResectionTool::GetXPM() const
{
  return mitkDrawPaintbrushTool_xpm;
}

us::ModuleResource ResectionTool::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("ResectionTool_48x48.png");
  return resource;
}

us::ModuleResource ResectionTool::GetCursorIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("ResectionTool_Cursor_32x32.png");
  return resource;
}

void ResectionTool::OnMousePressed(StateMachineAction* action, InteractionEvent* interactionEvent)
{
  bool renderIs2D = interactionEvent->GetSender()->GetMapperID() == mitk::BaseRenderer::Standard2D;
  m_FeedbackContourNode->SetProperty("contour.render2D", mitk::BoolProperty::New(renderIs2D));
  m_FeedbackContourNode->SetProperty("contour.render3D", mitk::BoolProperty::New(!renderIs2D));
  RenderingManager::GetInstance()->RequestUpdateAll();

  Superclass::OnMousePressed(action, interactionEvent);
}

void ResectionTool::OnMouseReleased(StateMachineAction*, InteractionEvent* interactionEvent)
{
  RenderingManager::GetInstance()->RequestUpdateAll();

  unsigned int targetTimeStep = interactionEvent->GetSender()->GetSliceNavigationController()->GetTime()->GetPos();

  auto pointCount = m_FeedbackContour->GetNumberOfVertices(targetTimeStep);
  if (pointCount < 3) {
    return;
  }

  m_LastEventSender = interactionEvent->GetSender();
  m_State = ResectionState::CONTOUR_AVAILABLE;
  stateChanged.Send(m_State);
}

ResectionTool::ResectionState ResectionTool::GetState()
{
  return m_State;
}

void ResectionTool::Activated()
{
  Superclass::Activated();

  m_State = ResectionState::NO_CONTOUR;
  stateChanged.Send(m_State);
}

template<typename TPixel, unsigned int VImageDimension = 3U>
void CreateMask(const itk::Image<TPixel, VImageDimension>* itkImage, vtkSmartPointer<vtkPoints> points, vtkSmartPointer<vtkMatrix4x4> viewMatrix, itk::Image<char, 3>::Pointer& outputMask)
{
  using ResectionMaskFilter = mitk::ResectionMaskFilter<itk::Image<TPixel, VImageDimension>>;
  typename ResectionMaskFilter::Pointer resBuilder = ResectionMaskFilter::New();
  resBuilder->SetInputPoints(points);
  resBuilder->setViewportMatrix(viewMatrix);
  resBuilder->SetInput(itkImage);
  resBuilder->InPlaceOff();
  resBuilder->Update();
  outputMask = resBuilder->GetOutput();
}

template<typename TPixel, unsigned int VImageDimension = 3U>
void GetMinimumValue(const itk::Image<TPixel, VImageDimension>* itkImage, float& min)
{
  typename itk::MinimumMaximumImageCalculator<itk::Image<TPixel, VImageDimension>>::Pointer minMaxCalculator =
    itk::MinimumMaximumImageCalculator<itk::Image<TPixel, VImageDimension>>::New();
  minMaxCalculator->SetImage(itkImage);
  minMaxCalculator->ComputeMinimum();
  min = (float)minMaxCalculator->GetMinimum();
}

template<typename TPixel, unsigned int VImageDimension = 3U>
void ResectByMask(const itk::Image<TPixel, VImageDimension>* itkImage, itk::Image<char, 3>::Pointer mask, char maskValue, float fillValue)
{
  typedef itk::MaskImageFilter<itk::Image<TPixel, VImageDimension>, itk::Image<char, 3>> MaskFilterType;
  typename MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput(itkImage);
  maskFilter->SetMaskImage(mask);
  maskFilter->SetOutsideValue(fillValue);
  maskFilter->SetMaskingValue(maskValue);
  maskFilter->InPlaceOn();
  maskFilter->Update();
}

vtkMatrix4x4* getWorldToViewTransform(vtkRenderer* renderer)
{
  vtkCamera* camera = renderer->GetActiveCamera();

  // figure out the same aspect ratio used by the render engine
  // (see vtkOpenGLCamera::Render())
  int  lowerLeft[2];
  int usize, vsize;
  double aspect1[2];
  double aspect2[2];
  renderer->GetTiledSizeAndOrigin(&usize, &vsize, lowerLeft, lowerLeft + 1);
  renderer->ComputeAspect();
  renderer->GetAspect(aspect1);
  renderer->vtkViewport::ComputeAspect();
  renderer->vtkViewport::GetAspect(aspect2);
  double aspectModification = (aspect1[0] * aspect2[1]) / (aspect1[1] * aspect2[0]);
  double aspect = aspectModification * usize / vsize;
  vtkMatrix4x4* matrix = camera->GetCompositeProjectionTransformMatrix(aspect, -1, 1);

  return matrix;
}

void ResectionTool::Resect(ResectionType type)
{
  unsigned int targetTimeStep = m_LastEventSender->GetSliceNavigationController()->GetTime()->GetPos();

  auto pointCount = m_FeedbackContour->GetNumberOfVertices(targetTimeStep);
  if (pointCount < 3) {
    return;
  }

  vtkSmartPointer<vtkPoints> points = vtkPoints::New();
  points->Resize(pointCount);
  for (int i = 0; i < pointCount; i++) {
    Point3D contourVertex = m_FeedbackContour->GetVertexAt(i, targetTimeStep)->Coordinates;
    points->InsertNextPoint(contourVertex.GetDataPointer());
  }

  DataNode* workingData = m_ToolManager->GetWorkingData().front();
  Image::Pointer segmentation = dynamic_cast<Image*>(workingData->GetData());

  Image::Pointer mitkImage3d = Image::New();
  if (segmentation->GetDimension() > 3) {
    ImageTimeSelector::Pointer timeSelector = ImageTimeSelector::New();
    timeSelector->SetInput(segmentation);
    timeSelector->SetTimeNr(targetTimeStep);
    timeSelector->UpdateLargestPossibleRegion();
    mitkImage3d = timeSelector->GetOutput();
  } else {
    mitkImage3d = segmentation->Clone();
  }

  vtkMatrix4x4* worldView = getWorldToViewTransform(m_LastEventSender->GetVtkRenderer());
  itk::Image<char, 3>::Pointer resectionMask;
  AccessFixedDimensionByItk_n(mitkImage3d, CreateMask, 3U, (points, worldView, resectionMask));

  float minValue;
  AccessFixedDimensionByItk_n(mitkImage3d, GetMinimumValue, 3U, (minValue));

  switch (type)
  {
  case mitk::ResectionTool::INSIDE:
  case mitk::ResectionTool::OUTSIDE: {
    if (type == ResectionTool::ResectionType::INSIDE) {
      AccessFixedDimensionByItk_n(mitkImage3d, ResectByMask, 3U, (resectionMask, 1, minValue));
    } else if (type == ResectionTool::ResectionType::OUTSIDE) {
      AccessFixedDimensionByItk_n(mitkImage3d, ResectByMask, 3U, (resectionMask, 0, minValue));
    }

    AccessFixedDimensionByItk_n(mitkImage3d, paste3Dto4DByItk, 3U, (segmentation, targetTimeStep));
    segmentation->Modified();

    SurfaceCreator::Pointer surfaceCreator = SurfaceCreator::New();
    SurfaceCreator::SurfaceCreationArgs surfaceArgs;
    surfaceArgs.recreate = true;
    surfaceArgs.outputStorage = m_ToolManager->GetDataStorage();
    surfaceArgs.timestep = targetTimeStep;
    surfaceCreator->setArgs(surfaceArgs);
    surfaceCreator->setInput(workingData);

    CommandProgressUpdate::Pointer progressCommand = CommandProgressUpdate::New();
    surfaceCreator->AddObserver(itk::ProgressEvent(), progressCommand);

    mitk::ProgressBar::GetInstance()->Reset();
    mitk::ProgressBar::GetInstance()->AddStepsToDo(100);

    surfaceCreator->Update();
    break;
  }
  case mitk::ResectionTool::DIVIDE: {
    auto insideClone = mitkImage3d->Clone();
    auto outsideClone = mitkImage3d->Clone();
    AccessFixedDimensionByItk_n(insideClone, ResectByMask, 3U, (resectionMask, 0, minValue));
    AccessFixedDimensionByItk_n(outsideClone, ResectByMask, 3U, (resectionMask, 1, minValue));

    auto insideSegmentation = segmentation->Clone();
    auto outsideSegmentation = segmentation->Clone();
    AccessFixedDimensionByItk_n(insideClone, paste3Dto4DByItk, 3U, (insideSegmentation, targetTimeStep));
    AccessFixedDimensionByItk_n(outsideClone, paste3Dto4DByItk, 3U, (outsideSegmentation, targetTimeStep));
    insideSegmentation->Modified();
    outsideSegmentation->Modified();

    std::string baseName = workingData->GetName();
    std::string baseCaption = "";
    workingData->GetStringProperty("caption", baseCaption);

    mitk::DataNode::Pointer insideNode = workingData->Clone();
    insideNode->SetData(insideClone);
    insideNode->ConcatenatePropertyList(workingData->GetPropertyList()->Clone(), true);

    insideNode->SetName(baseName + " (1)");
    insideNode->SetStringProperty("caption", (baseCaption + " (1)").c_str());

    mitk::DataNode::Pointer outsideNode = workingData->Clone();
    outsideNode->SetData(outsideClone);
    outsideNode->ConcatenatePropertyList(workingData->GetPropertyList()->Clone(), true);

    outsideNode->SetName(baseName + " (2)");
    outsideNode->SetStringProperty("caption", (baseCaption + " (2)").c_str());

    auto parents = m_ToolManager->GetDataStorage()->GetSources(workingData);
    mitk::DataNode* parent = (parents->size() > 0) ? parents->at(0) : nullptr;
    m_ToolManager->GetDataStorage()->Add(insideNode, parent);
    m_ToolManager->GetDataStorage()->Add(outsideNode, parent);

    mitk::ProgressBar::GetInstance()->Reset();
    mitk::ProgressBar::GetInstance()->AddStepsToDo(200);

    SurfaceCreator::Pointer surfaceCreator = SurfaceCreator::New();
    SurfaceCreator::SurfaceCreationArgs surfaceArgs;
    surfaceArgs.outputStorage = m_ToolManager->GetDataStorage();
    surfaceArgs.recreate = true;
    surfaceArgs.timestep = targetTimeStep;
    surfaceCreator->setArgs(surfaceArgs);
    surfaceCreator->setInput(insideNode);

    CommandProgressUpdate::Pointer progressCommand = CommandProgressUpdate::New();
    unsigned long progressTag = surfaceCreator->AddObserver(itk::ProgressEvent(), progressCommand);

    surfaceCreator->Update();

    surfaceCreator->RemoveObserver(progressTag);
    CommandProgressUpdate::Pointer secondProgressCommand = CommandProgressUpdate::New();
    surfaceCreator->AddObserver(itk::ProgressEvent(), secondProgressCommand);

    surfaceCreator->setInput(outsideNode);

    surfaceCreator->Update();
    break;
  }
  }

  mitk::ProgressBar::GetInstance()->Reset();

  m_FeedbackContour->Clear();
  RenderingManager::GetInstance()->RequestUpdateAll();

  m_State = ResectionState::NO_CONTOUR;
  stateChanged.Send(m_State);
}

void ResectionTool::CommandProgressUpdate::Execute(const itk::Object* caller, const itk::EventObject& event)
{
  const itk::ProcessObject* filter = static_cast<const itk::ProcessObject*>(caller);
  if (!itk::ProgressEvent().CheckEvent(&event)) {
    return;
  }

  int nextStep = (int)(filter->GetProgress() * m_NumberOfSteps);
  if (nextStep > m_LastStep) {
    mitk::ProgressBar::GetInstance()->Progress(nextStep - m_LastStep);
    m_LastStep = nextStep;
  }
}

}

