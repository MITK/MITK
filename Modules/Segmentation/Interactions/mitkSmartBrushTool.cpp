#include "mitkSmartBrushTool.h"

#include "mitkDrawPaintbrushTool.xpm"

#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

#include <itkIntensityWindowingImageFilter.h>
#include <itkPasteImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>

#include "mitkAbstractTransformGeometry.h"
#include "mitkImageToItk.h"
#include "mitkITKImageImport.h"
#include "mitkLevelWindowProperty.h"
#include "mitkToolManager.h"

#define ROUND(a) ((a) > 0 ? (int)((a) + .5) : -(int)(.5 - (a)))

namespace mitk {
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, SmartBrushTool, "Smart Brush drawing tool");
}

mitk::SmartBrushTool::SmartBrushTool() :
  FeedbackContourTool("SmartBrushTool"),
  m_LastContourRadius(-1),
  m_Radius(10),
  m_WorkingImage(nullptr),
  m_ImageObserverTag(0),
  m_LastTimeStep(0),
  m_IsReferencePointSet(false),
  m_Sensitivity(50)
{
  m_MasterContour = ContourModel::New();
  m_MasterContour->Initialize();
  m_CurrentPlane = nullptr;

  m_WorkingNode = DataNode::New();
  m_WorkingNode->SetProperty("levelwindow", mitk::LevelWindowProperty::New(mitk::LevelWindow(0, 1)));
  m_WorkingNode->SetProperty("binary", mitk::BoolProperty::New(true));

  m_ThresholdFilter = itk::BinaryThresholdImageFilter<itk::Image<float, 3>, SegmentationType>::New();
  m_SmartBrushStrokeFilter = SmartBrushStrokeFilter<InternalImageType>::New();

  m_ThresholdFilter->SetInput(m_StrokeBuffer);
  m_ThresholdFilter->SetLowerThreshold(.5f);
  m_ThresholdFilter->SetUpperThreshold(1.0001f);
  m_ThresholdFilter->SetInsideValue(1);
  m_ThresholdFilter->SetOutsideValue(0);
}

mitk::SmartBrushTool::~SmartBrushTool()
{
}

const char* mitk::SmartBrushTool::GetName() const
{
  return "Smart Brush";
}

const char** mitk::SmartBrushTool::GetXPM() const
{
  return mitkDrawPaintbrushTool_xpm;
}

us::ModuleResource mitk::SmartBrushTool::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("SmartBrush_48x48.png");
  return resource;
}

us::ModuleResource mitk::SmartBrushTool::GetCursorIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("SmartBrush_Cursor_32x32.png");
  return resource;
}

void mitk::SmartBrushTool::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("PrimaryButtonPressed", OnPrimaryButtonPressed);
  CONNECT_FUNCTION("AltPrimaryButtonPressed", OnAltPrimaryButtonPressed);
  CONNECT_FUNCTION("AltSecondaryButtonPressed", OnAltSecondaryButtonPressed);
  CONNECT_FUNCTION("CtrlPrimaryButtonPressed", OnCtrlPrimaryButtonPressed);
  CONNECT_FUNCTION("Move", OnPrimaryButtonPressedMoved);
  CONNECT_FUNCTION("MouseMoved", OnMouseMoved);
  CONNECT_FUNCTION("Release", OnMouseReleased);
}

void mitk::SmartBrushTool::Activated()
{
  Superclass::Activated();

  FeedbackContourTool::SetFeedbackContourVisible(true);
  radiusChanged.Send(m_Radius);
  sensitivityChanged.Send(m_Sensitivity);
  m_ToolManager->WorkingDataChanged += mitk::MessageDelegate<mitk::SmartBrushTool>(this, &mitk::SmartBrushTool::OnToolManagerWorkingDataModified);

   m_Windows = {
    mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1")),
    mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget2")),
    mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3"))
  };

  for (const auto& window : m_Windows) {
    window->GetSliceNavigationController()->ConnectGeometrySliceEvent(this, false);
  }

  m_PointSet = mitk::PointSet::New();
  m_PointSetNode = mitk::DataNode::New();
  m_PointSetNode->SetData(m_PointSet);
  m_PointSetNode->SetName("SmartBrush_ReferencePoint");
  m_PointSetNode->SetBoolProperty("helper object", true);
  m_PointSetNode->SetColor(0., 1., 0.);
  m_PointSetNode->SetVisibility(true);
  m_ToolManager->GetDataStorage()->Add(m_PointSetNode, m_ToolManager->GetReferenceData(0));
}

void mitk::SmartBrushTool::Deactivated()
{
  m_ToolManager->GetDataStorage()->Remove(m_PointSetNode);
  m_PointSetNode = nullptr;

  for (const auto& window : m_Windows) {
    window->GetSliceNavigationController()->Disconnect(this);
  }

  FeedbackContourTool::SetFeedbackContourVisible(false);
  if (m_ToolManager->GetDataStorage()->Exists(m_WorkingNode)) {
    m_ToolManager->GetDataStorage()->Remove(m_WorkingNode);
  }

  m_WorkingSlice = nullptr;
  m_CurrentPlane = nullptr;

  if (m_WorkingImage && m_ImageObserverTag) {
    m_WorkingImage->RemoveObserver(m_ImageObserverTag);
  }
  m_WorkingImage = nullptr;
  m_StrokeBuffer = nullptr;

  m_ToolManager->WorkingDataChanged -= mitk::MessageDelegate<mitk::SmartBrushTool>(this, &mitk::SmartBrushTool::OnToolManagerWorkingDataModified);

  Superclass::Deactivated();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::SmartBrushTool::MouseMoved(mitk::InteractionEvent* interactionEvent, bool leftMouseButtonPressed)
{
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);

  m_LastTimeStep = positionEvent->GetSender()->GetTimeStep();
  m_LastWorldCoordinates = positionEvent->GetPlanePositionInWorld();

  MouseMovedImpl(positionEvent->GetSender()->GetCurrentWorldPlaneGeometry(), leftMouseButtonPressed);

  RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}

void mitk::SmartBrushTool::MousePressed(InteractionEvent* interactionEvent)
{
  if (m_WorkingSlice.IsNull()) {
    return;
  }

  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
  m_WorkingSlice->GetGeometry()->WorldToIndex(positionEvent->GetPlanePositionInWorld(), m_LastPosition);

  if (!positionEvent) {
    return;
  }

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  m_MasterContour->SetClosed(true);
  this->MouseMoved(interactionEvent, true);
}

void mitk::SmartBrushTool::OnPrimaryButtonPressed(StateMachineAction*, InteractionEvent* interactionEvent)
{
  m_CurrentDirection = Directions::ADD;
  MousePressed(interactionEvent);
}

void mitk::SmartBrushTool::OnAltPrimaryButtonPressed(StateMachineAction*, InteractionEvent* interactionEvent)
{
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);

  if (positionEvent == NULL) {
    return;
  }

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  m_WorkingImage->GetGeometry()->WorldToIndex(positionEvent->GetPlanePositionInWorld(), m_ReferencePoint);
  m_ReferencePoint[0] = ROUND(m_ReferencePoint[0]);
  m_ReferencePoint[1] = ROUND(m_ReferencePoint[1]);
  m_ReferencePoint[2] = ROUND(m_ReferencePoint[2]);

  mitk::Point3D pointCenter;
  m_WorkingImage->GetGeometry()->IndexToWorld(m_ReferencePoint, pointCenter);

  m_PointSet->InsertPoint(1, pointCenter);
  m_IsReferencePointSet = true;

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::SmartBrushTool::OnAltSecondaryButtonPressed(StateMachineAction*, InteractionEvent* interactionEvent)
{
  m_PointSet->Clear();
  m_IsReferencePointSet = false;

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::SmartBrushTool::OnCtrlPrimaryButtonPressed(StateMachineAction*, InteractionEvent* interactionEvent)
{
  m_CurrentDirection = Directions::REMOVE;
  MousePressed(interactionEvent);
}

void mitk::SmartBrushTool::OnMouseMoved(StateMachineAction*, InteractionEvent* interactionEvent)
{
  MouseMoved(interactionEvent, false);
}

void mitk::SmartBrushTool::OnPrimaryButtonPressedMoved(StateMachineAction*, InteractionEvent* interactionEvent)
{
  MouseMoved(interactionEvent, true);
}

void mitk::SmartBrushTool::OnMouseReleased(StateMachineAction*, InteractionEvent* interactionEvent)
{
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
  if (!positionEvent) return;

  RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::SmartBrushTool::SetRadius(int value)
{
  m_Radius = value;
}

void mitk::SmartBrushTool::SetSensitivity(int value)
{
  m_Sensitivity = value;
}

void mitk::SmartBrushTool::SetGeometrySlice(const itk::EventObject& geometrySliceEvent)
{
  const SliceNavigationController::GeometrySliceEvent* sliceEvent =
    dynamic_cast<const SliceNavigationController::GeometrySliceEvent*>(&geometrySliceEvent);
  int pos = sliceEvent->GetPos();

  const mitk::TimeGeometry* timeGeom = sliceEvent->GetTimeGeometry();
  const mitk::BaseGeometry* baseGeometry = timeGeom->GetGeometryForTimeStep(m_LastTimeStep);
  const mitk::SlicedGeometry3D* slicedGeometry = dynamic_cast<const mitk::SlicedGeometry3D*>(baseGeometry);
  const mitk::PlaneGeometry* planeGeometry = slicedGeometry->GetPlaneGeometry(pos);

  MouseMovedImpl(planeGeometry, false);
}

void mitk::SmartBrushTool::MouseMovedImpl(const mitk::PlaneGeometry* planeGeometry, bool leftMouseButtonPressed)
{
  CheckIfCurrentSliceHasChanged(planeGeometry);

  if (m_LastContourRadius != m_Radius) {
    UpdateContour();
    m_LastContourRadius = m_Radius;
  }

  Point3D indexCoordinates;
  m_WorkingSlice->GetGeometry()->WorldToIndex(m_LastWorldCoordinates, indexCoordinates);

  // Take third axis value from image coordinates
  Point3D globalIndexCoordinates;
  m_WorkingImage->GetGeometry()->WorldToIndex(m_LastWorldCoordinates, globalIndexCoordinates);

  indexCoordinates[0] = ROUND(indexCoordinates[0]);
  indexCoordinates[1] = ROUND(indexCoordinates[1]);
  indexCoordinates[2] = ROUND(globalIndexCoordinates[2]);

  if (fabs(indexCoordinates[0] - m_LastPosition[0]) > mitk::eps ||
      fabs(indexCoordinates[1] - m_LastPosition[1]) > mitk::eps ||
      fabs(indexCoordinates[2] - m_LastPosition[2]) > mitk::eps ||
      leftMouseButtonPressed) {
    m_LastPosition = indexCoordinates;
  } else {
    return;
  }

  ContourModel::Pointer contour = ContourModel::New();
  contour->Expand(m_LastTimeStep + 1);
  contour->SetClosed(true, m_LastTimeStep);

  ContourModel::VertexIterator it = m_MasterContour->Begin();
  ContourModel::VertexIterator end = m_MasterContour->End();

  auto planeSpacing = planeGeometry->GetSpacing();

  while (it != end) {
    Point3D point = (*it)->Coordinates;
    point[0] /= planeSpacing[0] / m_MinSpacing;
    point[1] /= planeSpacing[1] / m_MinSpacing;
    point[0] += indexCoordinates[0];
    point[1] += indexCoordinates[1];

    contour->AddVertex(point, m_LastTimeStep);
    it++;
  }

  ContourModel::Pointer displayContour = this->GetFeedbackContour();
  displayContour->Clear();

  ContourModel::Pointer tmp = FeedbackContourTool::BackProjectContourFrom2DSlice(m_WorkingSlice->GetGeometry(), contour);

  it = tmp->Begin();
  end = tmp->End();

  while (it != end) {
    Point3D point = (*it)->Coordinates;

    displayContour->AddVertex(point, m_LastTimeStep);
    it++;
  }

  m_FeedbackContourNode->GetData()->Modified();

  // Apply filter
  if (leftMouseButtonPressed) {
    itk::Image<float, 3>::SizeType brushSize;
    brushSize.Fill(2 * m_Radius + 1);

    itk::Image<float, 3>::IndexType brushStart;
    brushStart[0] = globalIndexCoordinates[0] - m_Radius;
    brushStart[1] = globalIndexCoordinates[1] - m_Radius;
    brushStart[2] = globalIndexCoordinates[2] - m_Radius;

    // Clamp region in image
    for (int i = 0; i < 3; i++) {
      if (brushStart[i] < 0) {
        brushSize[i] += brushStart[i];
        brushStart[i] = 0;
      }
      itk::Image<float, 3>::SizeType imageSize = m_OriginalImage->GetLargestPossibleRegion().GetSize();
      if ((brushStart[i] + brushSize[i]) >= imageSize[i]) {
        brushSize[i] -= (brushStart[i] + brushSize[i]) - imageSize[i];
      }
    }

    itk::Image<float, 3>::RegionType brushRegion(brushStart, brushSize);

    SegmentationType::SizeType segSize = brushSize;
    SegmentationType::IndexType segStart = brushStart;
    SegmentationType::RegionType segRegion(segStart, segSize);

    auto smartBrushStrokeFilter = dynamic_cast<SmartBrushStrokeFilter<InternalImageType>*>(m_SmartBrushStrokeFilter.GetPointer());
    smartBrushStrokeFilter->SetInput(m_StrokeBuffer);
    smartBrushStrokeFilter->SetOriginalImage(m_OriginalImage);
    smartBrushStrokeFilter->SetImageSpacing(m_ImageSpacing);
    smartBrushStrokeFilter->SetRadius(m_Radius * m_MinSpacing);
    InternalImageType::IndexType targetIndex;
    if (m_IsReferencePointSet) {
      Point3D pointIndexCoordinates;
      m_WorkingImage->GetGeometry()->WorldToIndex(m_PointSet->GetPoint(1), pointIndexCoordinates);

      targetIndex[0] = ROUND(pointIndexCoordinates[0]);
      targetIndex[1] = ROUND(pointIndexCoordinates[1]);
      targetIndex[2] = ROUND(pointIndexCoordinates[2]);
    } else {
      targetIndex[0] = globalIndexCoordinates[0];
      targetIndex[1] = globalIndexCoordinates[1];
      targetIndex[2] = globalIndexCoordinates[2];
    }
    smartBrushStrokeFilter->SetTargetIntensity(m_OriginalImage->GetPixel(targetIndex));
    smartBrushStrokeFilter->SetCenter(globalIndexCoordinates);
    smartBrushStrokeFilter->SetDirection(m_CurrentDirection);
    smartBrushStrokeFilter->SetSensitivity((float)m_Sensitivity / 100.f);
    smartBrushStrokeFilter->GetOutput()->SetRequestedRegion(brushRegion);
    smartBrushStrokeFilter->Update();

    using FloatPasteFilter = itk::PasteImageFilter<InternalImageType, InternalImageType>;
    FloatPasteFilter::Pointer strokePastFilter = FloatPasteFilter::New();
    strokePastFilter->SetSourceImage(smartBrushStrokeFilter->GetOutput());
    strokePastFilter->SetSourceRegion(brushRegion);
    strokePastFilter->SetDestinationImage(m_StrokeBuffer);
    strokePastFilter->SetDestinationIndex(brushStart);
    strokePastFilter->SetInPlace(true);
    strokePastFilter->Update();

    m_StrokeBuffer = strokePastFilter->GetOutput(); // Itk Logic =_=
    m_ThresholdFilter->SetInput(m_StrokeBuffer);
    m_ThresholdFilter->GetOutput()->SetRequestedRegion(segRegion);

    DataNode* workingNode(m_ToolManager->GetWorkingData(0));
    Image::Pointer image = dynamic_cast<Image*>(workingNode->GetData());

    SegmentationType::Pointer target = mitk::ImageToItkImage<mitk::Tool::DefaultSegmentationDataType, 3>(image);

    using PasteFilter = itk::PasteImageFilter<SegmentationType, SegmentationType>;
    PasteFilter::Pointer pasteFilter = PasteFilter::New();
    pasteFilter->SetSourceImage(m_ThresholdFilter->GetOutput());
    pasteFilter->SetSourceRegion(segRegion);
    pasteFilter->SetDestinationImage(target);
    pasteFilter->SetDestinationIndex(segStart);
    pasteFilter->SetInPlace(true);
    pasteFilter->Update();

    image->Update();
    image->Modified();

    // Somehow extremly slows down everything
    //RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void mitk::SmartBrushTool::CheckIfCurrentSliceHasChanged(const mitk::PlaneGeometry* planeGeometry)
{
  const AbstractTransformGeometry* abstractTransformGeometry(dynamic_cast<const AbstractTransformGeometry*> (planeGeometry));
  DataNode* workingNode(m_ToolManager->GetWorkingData(0));
  DataNode* refNode(m_ToolManager->GetReferenceData(0));

  if (!workingNode || !refNode) {
    return;
  }

  Image::Pointer image = dynamic_cast<Image*>(workingNode->GetData());
  Image::Pointer referenceImage = dynamic_cast<Image*>(refNode->GetData());

  if (!image || !referenceImage || !planeGeometry || abstractTransformGeometry) {
    return;
  }

  if (m_WorkingImage != image) {
    if (m_WorkingImage && m_ImageObserverTag) {
      m_WorkingImage->RemoveObserver(m_ImageObserverTag);
    }

    m_WorkingImage = image;

    SegmentationType::Pointer origSegmentation = SegmentationType::New();
    mitk::CastToItkImage(m_WorkingImage, origSegmentation);

    using SegInputFilterType = itk::BinaryThresholdImageFilter<SegmentationType, itk::Image<float, 3>>;
    SegInputFilterType::Pointer inputThresholdFilter = SegInputFilterType::New();
    inputThresholdFilter->SetLowerThreshold(1);
    inputThresholdFilter->SetUpperThreshold(1000);
    inputThresholdFilter->SetInsideValue(1.f);
    inputThresholdFilter->SetOutsideValue(0.f);
    inputThresholdFilter->SetInput(origSegmentation);
    inputThresholdFilter->UpdateLargestPossibleRegion();
    m_StrokeBuffer = inputThresholdFilter->GetOutput();

    m_OriginalImage = InternalImageType::New();
    mitk::CastToItkImage(referenceImage, m_OriginalImage);

    mitk::LevelWindow levelWindow;
    refNode->GetLevelWindow(levelWindow);

    using RescaleFilter = itk::IntensityWindowingImageFilter<InternalImageType, InternalImageType>;
    RescaleFilter::Pointer rescaleFilter = RescaleFilter::New();
    rescaleFilter->SetInput(m_OriginalImage);
    rescaleFilter->SetOutputMinimum(0.f);
    rescaleFilter->SetOutputMaximum(1.f);
    rescaleFilter->SetWindowMinimum(levelWindow.GetLowerWindowBound());
    rescaleFilter->SetWindowMaximum(levelWindow.GetUpperWindowBound());
    rescaleFilter->SetInPlace(true);
    rescaleFilter->Update();
    m_OriginalImage = rescaleFilter->GetOutput();

    // Uncomment to use min max scaling
    /*using RescaleFilter = itk::RescaleIntensityImageFilter<InternalImageType, InternalImageType>;
    RescaleFilter::Pointer rescaleFilter = RescaleFilter::New();
    rescaleFilter->SetInput(m_OriginalImage);
    rescaleFilter->SetOutputMinimum(0.f);
    rescaleFilter->SetOutputMaximum(1.f);
    rescaleFilter->SetInPlace(true);
    rescaleFilter->Update();
    m_OriginalImage = rescaleFilter->GetOutput();*/

    itk::Image<float, 3>::IndexType start;
    start.Fill(0);

    itk::Image<float, 3>::SizeType size = m_OriginalImage->GetLargestPossibleRegion().GetSize();

    itk::Image<float, 3>::RegionType region;
    region.SetSize(size);
    region.SetIndex(start);

    auto command = itk::ReceptorMemberCommand<SmartBrushTool>::New();
    command->SetCallbackFunction(this, &SmartBrushTool::OnToolManagerImageModified);
    m_ImageObserverTag = m_WorkingImage->AddObserver(itk::ModifiedEvent(), command);

    auto spacing = referenceImage->GetSlicedGeometry()->GetSpacing();
    m_MinSpacing = std::min(abs(spacing[0]), abs(spacing[1]));
    m_MinSpacing = std::min(m_MinSpacing, (float)abs(spacing[2]));

    m_ImageSpacing[0] = spacing[0];
    m_ImageSpacing[1] = spacing[1];
    m_ImageSpacing[2] = spacing[2];
  }

  if (m_CurrentPlane.IsNull() || m_WorkingSlice.IsNull()) {
    m_CurrentPlane = const_cast<PlaneGeometry*>(planeGeometry);
    m_WorkingSlice = SegTool2D::GetAffectedImageSliceAs2DImage(planeGeometry, image, m_LastTimeStep)->Clone();
    m_WorkingNode->ReplaceProperty("color", workingNode->GetProperty("color"));
    m_WorkingNode->SetData(m_WorkingSlice);
  } else {
    bool isSameSlice(false);
    isSameSlice = mitk::MatrixEqualElementWise(
        planeGeometry->GetIndexToWorldTransform()->GetMatrix(),
        m_CurrentPlane->GetIndexToWorldTransform()->GetMatrix());
    isSameSlice &= mitk::Equal(
        planeGeometry->GetIndexToWorldTransform()->GetOffset(),
        m_CurrentPlane->GetIndexToWorldTransform()->GetOffset());

    if (!isSameSlice) {
      m_ToolManager->GetDataStorage()->Remove(m_WorkingNode);
      m_CurrentPlane = nullptr;
      m_WorkingSlice = nullptr;
      m_WorkingNode = nullptr;

      m_CurrentPlane = const_cast<PlaneGeometry*>(planeGeometry);
      m_WorkingSlice = SegTool2D::GetAffectedImageSliceAs2DImage(planeGeometry, image, m_LastTimeStep)->Clone();

      m_WorkingNode = mitk::DataNode::New();
      m_WorkingNode->SetProperty("levelwindow", mitk::LevelWindowProperty::New(mitk::LevelWindow(0, 1)));
      m_WorkingNode->SetProperty("binary", mitk::BoolProperty::New(true));

      m_WorkingNode->SetData(m_WorkingSlice);

      RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }

  if (!m_ToolManager->GetDataStorage()->Exists(m_WorkingNode)) {
    m_WorkingNode->SetProperty("outline binary", mitk::BoolProperty::New(true));
    m_WorkingNode->SetProperty("color", workingNode->GetProperty("color"));
    m_WorkingNode->SetProperty("name", mitk::StringProperty::New("SmartBrush_Node"));
    m_WorkingNode->SetProperty("helper object", mitk::BoolProperty::New(true));
    m_WorkingNode->SetProperty("opacity", mitk::FloatProperty::New(.8));
    m_WorkingNode->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
    m_WorkingNode->SetVisibility(false, mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4")));

    m_ToolManager->GetDataStorage()->Add(m_WorkingNode);
  }
}

void mitk::SmartBrushTool::OnToolManagerImageModified(const itk::EventObject&)
{
  OnToolManagerWorkingDataModified();
}

void mitk::SmartBrushTool::OnToolManagerWorkingDataModified()
{
  // Here we simply set the current working slice to null. The next time the mouse is moved
  // within a renderwindow a new slice will be extracted from the new working data
  m_WorkingSlice = nullptr;
}

mitk::Point2D mitk::SmartBrushTool::upperLeft(mitk::Point2D p)
{
  p[0] -= .5;
  p[1] += .5;
  return p;
}

void mitk::SmartBrushTool::UpdateContour()
{
  float fradius = static_cast<float>(m_Radius) + .5;

  mitk::Point2D centerCorrection;
  centerCorrection.Fill(0);

  std::vector<mitk::Point2D> quarterCycleUpperRight;
  std::vector<mitk::Point2D> quarterCycleLowerRight;
  std::vector<mitk::Point2D> quarterCycleLowerLeft;
  std::vector<mitk::Point2D> quarterCycleUpperLeft;

  mitk::Point2D curPoint;
  bool curPointIsInside = true;
  curPoint[0] = 0;
  curPoint[1] = m_Radius;
  quarterCycleUpperRight.push_back(upperLeft(curPoint));

  while (curPoint[1] > 0) {
    float curPointXSquared = 0.f;
    float curPointYSquared = (curPoint[1] - centerCorrection[1]) * (curPoint[1] - centerCorrection[1]);
    while (curPointIsInside) {
      curPoint[0]++;
      curPointXSquared = (curPoint[0] - centerCorrection[0]) * (curPoint[0] - centerCorrection[0]);

      const float len = sqrt(curPointXSquared + curPointYSquared);
      if (len > fradius) {
        curPointIsInside = false;
      }
    }
    quarterCycleUpperRight.push_back(upperLeft(curPoint));

    while (!curPointIsInside) {
      curPoint[1]--;
      curPointYSquared = (curPoint[1] - centerCorrection[1]) * (curPoint[1] - centerCorrection[1]);
      const float len = sqrt(curPointXSquared + curPointYSquared);
      if (len <= fradius) {
        curPointIsInside = true;
        quarterCycleUpperRight.push_back(upperLeft(curPoint));
      }

      if (curPoint[1] <= 0) {
        break;
      }
    }
  }

  std::vector<mitk::Point2D>::const_iterator it = quarterCycleUpperRight.begin();
  while(it != quarterCycleUpperRight.end()) {
    mitk::Point2D p, q;
    p = *it;

    p[1] *= -1;
    quarterCycleLowerRight.push_back(p);

    p[0] *= -1;
    quarterCycleLowerLeft.push_back(p);

    p[1] *= -1;
    quarterCycleUpperLeft.push_back(p);

    it++;
  }

  ContourModel::Pointer contourInImageIndexCoordinates = ContourModel::New();

  mitk::Point3D tempPoint;
  for (unsigned int i = 0; i < quarterCycleUpperRight.size(); i++) {
    tempPoint[0] = quarterCycleUpperRight[i][0];
    tempPoint[1] = quarterCycleUpperRight[i][1];
    tempPoint[2] = 0;
    contourInImageIndexCoordinates->AddVertex(tempPoint);
  }

  for (int i = quarterCycleLowerRight.size() - 1; i >= 0; i--) {
    tempPoint[0] = quarterCycleLowerRight[i][0];
    tempPoint[1] = quarterCycleLowerRight[i][1];
    tempPoint[2] = 0;
    contourInImageIndexCoordinates->AddVertex(tempPoint);
  }

  for (unsigned int i = 0; i < quarterCycleLowerLeft.size(); i++) {
    tempPoint[0] = quarterCycleLowerLeft[i][0];
    tempPoint[1] = quarterCycleLowerLeft[i][1];
    tempPoint[2] = 0;
    contourInImageIndexCoordinates->AddVertex(tempPoint);
  }

  for (int i = quarterCycleUpperLeft.size() - 1; i >= 0; i--) {
    tempPoint[0] = quarterCycleUpperLeft[i][0];
    tempPoint[1] = quarterCycleUpperLeft[i][1];
    tempPoint[2] = 0;
    contourInImageIndexCoordinates->AddVertex(tempPoint);
  }

  m_MasterContour = contourInImageIndexCoordinates;
}

