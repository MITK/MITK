/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPaintbrushTool.h>

#include <mitkAbstractTransformGeometry.h>
#include <mitkBaseRenderer.h>
#include <mitkToolManager.h>

#include <mitkContourModelUtils.h>
#include <mitkLevelWindowProperty.h>
#include <mitkImageWriteAccessor.h>

mitk::PaintbrushTool::PaintbrushTool(bool startWithFillMode)
  : FeedbackContourTool("PressMoveReleaseWithCTRLInversionAllMouseMoves"),
    m_FillMode(startWithFillMode),
    m_Size(10),
    m_LastContourSize(0) // other than initial mitk::PaintbrushTool::m_Size (around l. 28)
{
  m_MasterContour = ContourModel::New();
  m_MasterContour->Initialize();
  m_CurrentPlane = nullptr;
}

mitk::PaintbrushTool::~PaintbrushTool()
{
}

void mitk::PaintbrushTool::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("PrimaryButtonPressed", OnMousePressed);
  CONNECT_FUNCTION("Move", OnPrimaryButtonPressedMoved);
  CONNECT_FUNCTION("MouseMove", OnMouseMoved);
  CONNECT_FUNCTION("Release", OnMouseReleased);
  CONNECT_FUNCTION("InvertLogic", OnInvertLogic);
}

int mitk::PaintbrushTool::GetFillValue() const
{
  return 255;
}

void mitk::PaintbrushTool::Activated()
{
  Superclass::Activated();

  SizeChanged.Send(m_Size);
  this->GetToolManager()->WorkingDataChanged +=
    mitk::MessageDelegate<mitk::PaintbrushTool>(this, &mitk::PaintbrushTool::OnToolManagerWorkingDataModified);

  m_PaintingNode = DataNode::New();
  m_PaintingNode->SetProperty("levelwindow", mitk::LevelWindowProperty::New(mitk::LevelWindow(0, this->GetFillValue())));
  m_PaintingNode->SetProperty("binary", mitk::BoolProperty::New(true));

  m_PaintingNode->SetProperty("outline binary", mitk::BoolProperty::New(true));
  m_PaintingNode->SetProperty("name", mitk::StringProperty::New("Paintbrush_Node"));
  m_PaintingNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_PaintingNode->SetProperty("opacity", mitk::FloatProperty::New(0.8));
  m_PaintingNode->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
  auto allRenderWindows = BaseRenderer::GetAll3DRenderWindows();
  for (auto mapit = allRenderWindows.begin(); mapit != allRenderWindows.end(); ++mapit)
  {
    m_PaintingNode->SetVisibility(false, mapit->second);
  }

  this->UpdateFeedbackColor();
  FeedbackContourTool::SetFeedbackContourVisible(true);

  this->GetToolManager()->GetDataStorage()->Add(m_PaintingNode);
}

void mitk::PaintbrushTool::Deactivated()
{
  FeedbackContourTool::SetFeedbackContourVisible(false);
  if (this->GetToolManager()->GetDataStorage()->Exists(m_PaintingNode))
    this->GetToolManager()->GetDataStorage()->Remove(m_PaintingNode);
  m_WorkingSlice = nullptr;
  m_PaintingSlice = nullptr;
  m_CurrentPlane = nullptr;
  m_PaintingNode = nullptr;

  this->GetToolManager()->WorkingDataChanged -=
    mitk::MessageDelegate<mitk::PaintbrushTool>(this, &mitk::PaintbrushTool::OnToolManagerWorkingDataModified);

  Superclass::Deactivated();
}

void mitk::PaintbrushTool::SetSize(int value)
{
  m_Size = value;
}

mitk::Point2D mitk::PaintbrushTool::upperLeft(mitk::Point2D p)
{
  p[0] -= 0.5;
  p[1] += 0.5;
  return p;
}

mitk::ContourModel::Pointer mitk::PaintbrushTool::CreateBrushContour(int size)
{
  const int radius = size / 2;
  const float fradius = static_cast<float>(size) / 2.0f;

  auto contourInImageIndexCoordinates = ContourModel::New();
  contourInImageIndexCoordinates->SetClosed(true);

  // estimate center point of the brush ( relative to the pixel the mouse points on )
  // -- left upper corner for even sizes,
  // -- midpoint for uneven sizes
  Point2D centerCorrection;
  centerCorrection.Fill(0);

  const bool evenSize = ((size % 2) == 0);
  if (evenSize)
  {
    centerCorrection[0] += 0.5;
    centerCorrection[1] += 0.5;
  }

  // compute the control points for the upper right quarter of the circle contour
  std::vector<Point2D> quarterCycleUpperRight;
  std::vector<Point2D> quarterCycleLowerRight;
  std::vector<Point2D> quarterCycleLowerLeft;
  std::vector<Point2D> quarterCycleUpperLeft;

  Point2D curPoint;
  bool curPointIsInside = true;
  curPoint[0] = 0;
  curPoint[1] = radius;
  quarterCycleUpperRight.push_back(upperLeft(curPoint));

  while (curPoint[1] > 0)
  {
    float curPointX_squared = 0.0f;
    float curPointY_squared = (curPoint[1] - centerCorrection[1]) * (curPoint[1] - centerCorrection[1]);
    while (curPointIsInside)
    {
      curPoint[0]++;
      curPointX_squared = (curPoint[0] - centerCorrection[0]) * (curPoint[0] - centerCorrection[0]);
      const float len = sqrt(curPointX_squared + curPointY_squared);
      if (len > fradius)
        curPointIsInside = false;
    }
    quarterCycleUpperRight.push_back(upperLeft(curPoint));

    while (!curPointIsInside)
    {
      curPoint[1]--;
      curPointY_squared = (curPoint[1] - centerCorrection[1]) * (curPoint[1] - centerCorrection[1]);
      const float len = sqrt(curPointX_squared + curPointY_squared);
      if (len <= fradius)
      {
        curPointIsInside = true;
        quarterCycleUpperRight.push_back(upperLeft(curPoint));
      }

      if (curPoint[1] <= 0)
        break;
    }
  }

  // QuarterCycle is full! Now copy quarter cycle to other quarters.
  if (!evenSize)
  {
    for (const auto& p0 : quarterCycleUpperRight)
    {
      Point2D p = p0;
      p[1] *= -1;
      quarterCycleLowerRight.push_back(p);
      p[0] *= -1;
      quarterCycleLowerLeft.push_back(p);
      p[1] *= -1;
      quarterCycleUpperLeft.push_back(p);
    }
  }
  else
  {
    for (const auto& p0 : quarterCycleUpperRight)
    {
      Point2D q;
      q = p0; q[1] = -1.0f * q[1] + 1;                          quarterCycleLowerRight.push_back(q);
      q = p0; q[1] = -1.0f * q[1] + 1; q[0] = -1.0f * q[0] + 1; quarterCycleLowerLeft.push_back(q);
      q = p0; q[0] = -1.0f * q[0] + 1;                          quarterCycleUpperLeft.push_back(q);
    }
  }

  // fill contour with points in right ordering, starting with the upperRight block
  Point3D tempPoint;
  tempPoint[2] = 0;
  for (const auto& p : quarterCycleUpperRight)
  {
    tempPoint[0] = p[0]; tempPoint[1] = p[1];
    contourInImageIndexCoordinates->AddVertex(tempPoint);
  }
  for (int i = static_cast<int>(quarterCycleLowerRight.size()) - 1; i >= 0; --i)
  {
    tempPoint[0] = quarterCycleLowerRight[i][0]; tempPoint[1] = quarterCycleLowerRight[i][1];
    contourInImageIndexCoordinates->AddVertex(tempPoint);
  }
  for (const auto& p : quarterCycleLowerLeft)
  {
    tempPoint[0] = p[0]; tempPoint[1] = p[1];
    contourInImageIndexCoordinates->AddVertex(tempPoint);
  }
  for (int i = static_cast<int>(quarterCycleUpperLeft.size()) - 1; i >= 0; --i)
  {
    tempPoint[0] = quarterCycleUpperLeft[i][0]; tempPoint[1] = quarterCycleUpperLeft[i][1];
    contourInImageIndexCoordinates->AddVertex(tempPoint);
  }

  return contourInImageIndexCoordinates;
}

mitk::ContourModel::Pointer mitk::PaintbrushTool::CreateGapContour(const Point3D& from,
                                                                    const Point3D& to,
                                                                    double radius)
{
  Point3D direction;
  direction[0] = to[0] - from[0];
  direction[1] = to[1] - from[1];
  direction[2] = to[2] - from[2];
  const auto dirVec = direction.GetVnlVector().normalize();
  direction[0] = dirVec[0];
  direction[1] = dirVec[1];
  direction[2] = dirVec[2];

  // 90 degrees rotation of direction in the slice plane
  Point3D normal;
  normal[0] = -1.0 * direction[1];
  normal[1] = direction[0];
  normal[2] = 0;

  auto gapContour = ContourModel::New();
  gapContour->SetClosed(true);

  Point3D vertex;
  vertex[2] = 0;

  vertex[0] = from[0] + normal[0] * radius;
  vertex[1] = from[1] + normal[1] * radius;
  gapContour->AddVertex(vertex);

  vertex[0] = to[0] + normal[0] * radius;
  vertex[1] = to[1] + normal[1] * radius;
  gapContour->AddVertex(vertex);

  vertex[0] = to[0] - normal[0] * radius;
  vertex[1] = to[1] - normal[1] * radius;
  gapContour->AddVertex(vertex);

  vertex[0] = from[0] - normal[0] * radius;
  vertex[1] = from[1] - normal[1] * radius;
  gapContour->AddVertex(vertex);

  return gapContour;
}

void mitk::PaintbrushTool::UpdateContour(const InteractionPositionEvent *positionEvent)
{
  if (!positionEvent)
    return;

  m_MasterContour = CreateBrushContour(m_Size);
}

void mitk::PaintbrushTool::OnMousePressed(StateMachineAction *, InteractionEvent *interactionEvent)
{
  if (m_WorkingSlice.IsNull())
    return;

  auto* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
  if (!positionEvent)
    return;

  this->ResetWorkingSlice(positionEvent);

  m_WorkingSlice->GetGeometry()->WorldToIndex(positionEvent->GetPositionInWorld(), m_LastPosition);
  this->m_PaintingNode->SetVisibility(true);

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();
  m_MasterContour->SetClosed(true);
  this->MouseMoved(interactionEvent, true);
}

void mitk::PaintbrushTool::OnMouseMoved(StateMachineAction *, InteractionEvent *interactionEvent)
{
  MouseMoved(interactionEvent, false);
}

void mitk::PaintbrushTool::OnPrimaryButtonPressedMoved(StateMachineAction *, InteractionEvent *interactionEvent)
{
  MouseMoved(interactionEvent, true);
}

/**
  Insert the point to the feedback contour,finish to build the contour and at the same time the painting function
  */
void mitk::PaintbrushTool::MouseMoved(mitk::InteractionEvent *interactionEvent, bool leftMouseButtonPressed)
{
  auto *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);

  bool newSlice = CheckIfCurrentSliceHasChanged(positionEvent);
  if (newSlice)
  {
    this->ResetWorkingSlice(positionEvent);
  }

  if (m_LastContourSize != m_Size)
  {
    UpdateContour(positionEvent);
    m_LastContourSize = m_Size;
  }

  Point3D worldCoordinates = positionEvent->GetPositionInWorld();
  Point3D indexCoordinates;

  m_WorkingSlice->GetGeometry()->WorldToIndex(worldCoordinates, indexCoordinates);

  // round to nearest voxel center (abort if this hasn't changed)
  if (m_Size % 2 == 0) // even
  {
    indexCoordinates[0] = std::round(indexCoordinates[0]);
    indexCoordinates[1] = std::round(indexCoordinates[1]);
  }
  else // odd
  {
    indexCoordinates[0] = std::round(indexCoordinates[0]);
    indexCoordinates[1] = std::round(indexCoordinates[1]);
  }

  static Point3D lastPos; // uninitialized: if somebody finds out how this can be initialized in a one-liner, tell me
  if (fabs(indexCoordinates[0] - lastPos[0]) > mitk::eps || fabs(indexCoordinates[1] - lastPos[1]) > mitk::eps ||
      fabs(indexCoordinates[2] - lastPos[2]) > mitk::eps || leftMouseButtonPressed)
  {
    lastPos = indexCoordinates;
  }
  else
  {
    return;
  }

  auto contour = ContourModel::New();
  contour->SetClosed(true);

  auto it = m_MasterContour->Begin();
  auto end = m_MasterContour->End();

  while (it != end)
  {
    auto point = (*it)->Coordinates;
    point[0] += indexCoordinates[0];
    point[1] += indexCoordinates[1];

    contour->AddVertex(point);
    ++it;
  }

  if (leftMouseButtonPressed)
  {
    ContourModelUtils::FillContourInSlice2(contour, m_PaintingSlice, this->GetFillValue());

    const double dist = indexCoordinates.EuclideanDistanceTo(m_LastPosition);
    const double radius = static_cast<double>(m_Size) / 2.0;

    // if points are >= radius away draw rectangle to fill empty holes
    // in between the 2 points
    if (dist > radius)
    {
      auto gapContour = CreateGapContour(m_LastPosition, indexCoordinates, radius);
      ContourModelUtils::FillContourInSlice2(gapContour, m_PaintingSlice, this->GetFillValue());
    }
  }
  else
  {
    // switched from different renderwindow
    // no activate hover highlighting. Otherwise undo / redo wont work
    this->m_PaintingNode->SetVisibility(false);
  }

  m_LastPosition = indexCoordinates;

  // visualize contour
  ContourModel::Pointer tmp =
    FeedbackContourTool::BackProjectContourFrom2DSlice(m_WorkingSlice->GetGeometry(), contour);

  this->UpdateCurrentFeedbackContour(tmp);

  if (newSlice)
  {
    RenderingManager::GetInstance()->RequestUpdateAll();
  }
  else
  {
    assert(positionEvent->GetSender()->GetRenderWindow());
    RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
  }
}

void mitk::PaintbrushTool::OnMouseReleased(StateMachineAction *, InteractionEvent *interactionEvent)
{
  // When mouse is released write segmentation result back into image
  auto *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
  if (!positionEvent)
    return;

  auto workingSeg = this->GetWorkingData();
  auto activeLabel = workingSeg->GetActiveLabel();
  if (nullptr == activeLabel)
    return;
  Label::PixelType activePixelValue = activeLabel->GetValue();
  if (!m_FillMode)
  {
    activePixelValue = MultiLabelSegmentation::UNLABELED_VALUE;
  }

  //as paintbrush tools should always allow to manipulate active label
  //(that is what the user expects/knows when using tools so far:
  //the active label can always be changed even if locked)
  //we realize that by cloning the relevant label and changing the lock state
  //this fillLabelSet is used for the transfer.
  auto destinationLabels = workingSeg->GetConstLabelsByValue(workingSeg->GetLabelValuesByGroup(workingSeg->GetActiveLayer()));
  auto activeLabelClone = workingSeg->GetActiveLabel()->Clone();
  if (nullptr != activeLabelClone)
  {
    activeLabelClone->SetLocked(false);
    auto activeIter = std::find(destinationLabels.begin(), destinationLabels.end(), workingSeg->GetActiveLabel());
    if (activeIter == destinationLabels.end()) mitkThrow() << "Application is in an invalid state. Active label is not contained in the labelset, but its group was requested.";
    *activeIter = activeLabelClone;
  }


  TransferLabelContentAtTimeStep(m_PaintingSlice, m_WorkingSlice, destinationLabels, 0, MultiLabelSegmentation::UNLABELED_VALUE, MultiLabelSegmentation::UNLABELED_VALUE, false, { {this->GetFillValue(), activePixelValue}}, mitk::MultiLabelSegmentation::MergeStyle::Merge);

  this->WriteBackSegmentationResult(positionEvent, m_WorkingSlice->Clone());

  // deactivate visibility of helper node
  m_PaintingNode->SetVisibility(false);
  m_PaintingNode->SetData(nullptr);
  m_PaintingSlice = nullptr;
  m_WorkingSlice = nullptr;

  RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::PaintbrushTool::UpdateFeedbackColor()
{
  mitk::Color currentColor;
  if (m_FillMode)
  {
    FeedbackContourTool::SetFeedbackContourColorDefault();
    currentColor.Set(0.0, 1.0, 0.);
  }
  else
  {
    FeedbackContourTool::SetFeedbackContourColor(1.0, 0.0, 0.0);
    currentColor.Set(1.0, 0.0, 0.);
  }

  if (m_PaintingNode.IsNotNull())
  {
    m_PaintingNode->SetProperty("color", mitk::ColorProperty::New(currentColor[0], currentColor[1], currentColor[2]));
  }
}

/**
  Called when the CTRL key is pressed.
  */
void mitk::PaintbrushTool::OnInvertLogic(StateMachineAction *, InteractionEvent *)
{
  m_FillMode = !m_FillMode;
  UpdateFeedbackColor();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

bool mitk::PaintbrushTool::CheckIfCurrentSliceHasChanged(const InteractionPositionEvent *event)
{
  const PlaneGeometry* planeGeometry((event->GetSender()->GetCurrentWorldPlaneGeometry()));
  const auto* abstractTransformGeometry(
    dynamic_cast<const AbstractTransformGeometry *>(event->GetSender()->GetCurrentWorldPlaneGeometry()));
  if (nullptr == planeGeometry || nullptr != abstractTransformGeometry)
  {
    return false;
  }

  bool newPlane = false;

  if (m_CurrentPlane.IsNull() || m_WorkingSlice.IsNull()
      //or not the same slice
     || !mitk::MatrixEqualElementWise(planeGeometry->GetIndexToWorldTransform()->GetMatrix(),
       m_CurrentPlane->GetIndexToWorldTransform()->GetMatrix())
     || !mitk::Equal(planeGeometry->GetIndexToWorldTransform()->GetOffset(),
       m_CurrentPlane->GetIndexToWorldTransform()->GetOffset()))
  {
    m_CurrentPlane = planeGeometry;
    newPlane = true;
  }

  return newPlane;
}

void mitk::PaintbrushTool::ResetWorkingSlice(const InteractionPositionEvent* event)
{
  const PlaneGeometry* planeGeometry((event->GetSender()->GetCurrentWorldPlaneGeometry()));
  const auto* abstractTransformGeometry(
    dynamic_cast<const AbstractTransformGeometry*>(event->GetSender()->GetCurrentWorldPlaneGeometry()));
  if (nullptr == planeGeometry || nullptr != abstractTransformGeometry)
  {
    return;
  }

  m_WorkingSlice = nullptr;
  m_PaintingSlice = nullptr;
  m_PaintingNode->SetData(nullptr);

  auto segmentation = this->GetWorkingData();
  if (nullptr == segmentation)
  {
    return;
  }

  auto affectedSlice = SegTool2D::GetAffectedImageSliceAs2DImage(event, segmentation->GetGroupImage(segmentation->GetActiveLayer()));
  if (affectedSlice.IsNull())
  {
    return;
  }
  m_WorkingSlice = affectedSlice->Clone();

  m_PaintingSlice = Image::New();
  m_PaintingSlice->Initialize(m_WorkingSlice);

  unsigned int byteSize = m_PaintingSlice->GetPixelType().GetSize();
  for (unsigned int dim = 0; dim < m_PaintingSlice->GetDimension(); ++dim)
  {
    byteSize *= m_PaintingSlice->GetDimension(dim);
  }
  mitk::ImageWriteAccessor writeAccess(m_PaintingSlice.GetPointer(), m_PaintingSlice->GetVolumeData(0));
  memset(writeAccess.GetData(), 0, byteSize);

  m_PaintingNode->SetData(m_PaintingSlice);
}

void mitk::PaintbrushTool::OnToolManagerWorkingDataModified()
{
  // Here we simply set the current working slice to null. The next time the mouse is moved
  // within a renderwindow a new slice will be extracted from the new working data
  m_WorkingSlice = nullptr;
  m_PaintingSlice = nullptr;
}
