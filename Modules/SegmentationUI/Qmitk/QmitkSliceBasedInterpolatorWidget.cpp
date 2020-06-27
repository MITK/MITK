/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSliceBasedInterpolatorWidget.h"

#include <mitkColorProperty.h>
#include <mitkDiffSliceOperation.h>
#include <mitkDiffSliceOperationApplier.h>
#include <mitkExtractSliceFilter.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkImageTimeSelector.h>
#include <mitkLabelSetImage.h>
#include <mitkOperationEvent.h>
#include <mitkProgressBar.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>
#include <mitkSegTool2D.h>
#include <mitkSliceNavigationController.h>
#include <mitkToolManager.h>
#include <mitkToolManagerProvider.h>
#include <mitkUndoController.h>
#include <mitkVtkImageOverwrite.h>

#include "QmitkStdMultiWidget.h"

#include <itkCommand.h>

#include <QApplication>
#include <QCursor>
#include <QMenu>
#include <QMessageBox>

QmitkSliceBasedInterpolatorWidget::QmitkSliceBasedInterpolatorWidget(QWidget *parent, const char * /*name*/)
  : QWidget(parent),
    m_SliceInterpolatorController(mitk::SliceBasedInterpolationController::New()),
    m_ToolManager(nullptr),
    m_Activated(false),
    m_DataStorage(nullptr),
    m_LastSNC(nullptr),
    m_LastSliceIndex(0)
{
  m_Controls.setupUi(this);

  m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager(mitk::ToolManagerProvider::MULTILABEL_SEGMENTATION);

  m_ToolManager->WorkingDataChanged += mitk::MessageDelegate<QmitkSliceBasedInterpolatorWidget>(
    this, &QmitkSliceBasedInterpolatorWidget::OnToolManagerWorkingDataModified);

  connect(m_Controls.m_btStart, SIGNAL(toggled(bool)), this, SLOT(OnToggleWidgetActivation(bool)));
  connect(m_Controls.m_btApplyForCurrentSlice, SIGNAL(clicked()), this, SLOT(OnAcceptInterpolationClicked()));
  connect(m_Controls.m_btApplyForAllSlices, SIGNAL(clicked()), this, SLOT(OnAcceptAllInterpolationsClicked()));

  itk::ReceptorMemberCommand<QmitkSliceBasedInterpolatorWidget>::Pointer command =
    itk::ReceptorMemberCommand<QmitkSliceBasedInterpolatorWidget>::New();
  command->SetCallbackFunction(this, &QmitkSliceBasedInterpolatorWidget::OnSliceInterpolationInfoChanged);
  m_InterpolationInfoChangedObserverTag = m_SliceInterpolatorController->AddObserver(itk::ModifiedEvent(), command);

  // feedback node and its visualization properties
  m_PreviewNode = mitk::DataNode::New();
  m_PreviewNode->SetName("3D tool preview");

  m_PreviewNode->SetProperty("texture interpolation", mitk::BoolProperty::New(false));
  m_PreviewNode->SetProperty("layer", mitk::IntProperty::New(100));
  m_PreviewNode->SetProperty("binary", mitk::BoolProperty::New(true));
  m_PreviewNode->SetProperty("outline binary", mitk::BoolProperty::New(true));
  m_PreviewNode->SetProperty("outline binary shadow", mitk::BoolProperty::New(true));
  m_PreviewNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_PreviewNode->SetOpacity(1.0);
  m_PreviewNode->SetColor(0.0, 1.0, 0.0);

  m_Controls.m_btApplyForCurrentSlice->setEnabled(false);
  m_Controls.m_btApplyForAllSlices->setEnabled(false);

  this->setEnabled(false);
}

QmitkSliceBasedInterpolatorWidget::~QmitkSliceBasedInterpolatorWidget()
{
  m_ToolManager->WorkingDataChanged -= mitk::MessageDelegate<QmitkSliceBasedInterpolatorWidget>(
    this, &QmitkSliceBasedInterpolatorWidget::OnToolManagerWorkingDataModified);

  foreach (mitk::SliceNavigationController *slicer, m_ControllerToSliceObserverTag.keys())
  {
    slicer->RemoveObserver(m_ControllerToDeleteObserverTag.take(slicer));
    slicer->RemoveObserver(m_ControllerToTimeObserverTag.take(slicer));
    slicer->RemoveObserver(m_ControllerToSliceObserverTag.take(slicer));
  }

  m_ActionToSliceDimensionMap.clear();

  // remove observer
  m_SliceInterpolatorController->RemoveObserver(m_InterpolationInfoChangedObserverTag);
}

const QmitkSliceBasedInterpolatorWidget::ActionToSliceDimensionMapType
  QmitkSliceBasedInterpolatorWidget::CreateActionToSliceDimension()
{
  ActionToSliceDimensionMapType actionToSliceDimension;
  foreach (mitk::SliceNavigationController *slicer, m_ControllerToDeleteObserverTag.keys())
  {
    std::string name = slicer->GetRenderer()->GetName();
    if (name == "stdmulti.widget0")
      name = "Axial (red window)";
    else if (name == "stdmulti.widget1")
      name = "Sagittal (green window)";
    else if (name == "stdmulti.widget2")
      name = "Coronal (blue window)";
    actionToSliceDimension[new QAction(QString::fromStdString(name), nullptr)] = slicer;
  }

  return actionToSliceDimension;
}

void QmitkSliceBasedInterpolatorWidget::SetDataStorage(mitk::DataStorage &storage)
{
  m_DataStorage = &storage;
}

void QmitkSliceBasedInterpolatorWidget::SetSliceNavigationControllers(
  const QList<mitk::SliceNavigationController *> &controllers)
{
  Q_ASSERT(!controllers.empty());

  // connect to the slice navigation controller. after each change, call the interpolator
  foreach (mitk::SliceNavigationController *slicer, controllers)
  {
    // Has to be initialized
    m_LastSNC = slicer;

    m_TimePoints.insert(slicer, slicer->GetSelectedTimePoint());

    itk::MemberCommand<QmitkSliceBasedInterpolatorWidget>::Pointer deleteCommand =
      itk::MemberCommand<QmitkSliceBasedInterpolatorWidget>::New();
    deleteCommand->SetCallbackFunction(this, &QmitkSliceBasedInterpolatorWidget::OnSliceNavigationControllerDeleted);
    m_ControllerToDeleteObserverTag.insert(slicer, slicer->AddObserver(itk::DeleteEvent(), deleteCommand));

    itk::MemberCommand<QmitkSliceBasedInterpolatorWidget>::Pointer timeChangedCommand =
      itk::MemberCommand<QmitkSliceBasedInterpolatorWidget>::New();
    timeChangedCommand->SetCallbackFunction(this, &QmitkSliceBasedInterpolatorWidget::OnTimeChanged);
    m_ControllerToTimeObserverTag.insert(
      slicer,
      slicer->AddObserver(mitk::SliceNavigationController::TimeGeometryEvent(nullptr, 0), timeChangedCommand));

    itk::MemberCommand<QmitkSliceBasedInterpolatorWidget>::Pointer sliceChangedCommand =
      itk::MemberCommand<QmitkSliceBasedInterpolatorWidget>::New();
    sliceChangedCommand->SetCallbackFunction(this, &QmitkSliceBasedInterpolatorWidget::OnSliceChanged);
    m_ControllerToSliceObserverTag.insert(
      slicer, slicer->AddObserver(mitk::SliceNavigationController::GeometrySliceEvent(nullptr, 0), sliceChangedCommand));
  }

  m_ActionToSliceDimensionMap = this->CreateActionToSliceDimension();
}

void QmitkSliceBasedInterpolatorWidget::OnToolManagerWorkingDataModified()
{
  mitk::DataNode *workingNode = this->m_ToolManager->GetWorkingData(0);
  if (!workingNode)
  {
    this->setEnabled(false);
    return;
  }

  mitk::LabelSetImage *workingImage = dynamic_cast<mitk::LabelSetImage *>(workingNode->GetData());
  // TODO adapt tool manager so that this check is done there, e.g. convenience function
  //  Q_ASSERT(workingImage);
  if (!workingImage)
  {
    this->setEnabled(false);
    return;
  }

  if (workingImage->GetDimension() > 4 || workingImage->GetDimension() < 3)
  {
    this->setEnabled(false);
    return;
  }

  m_WorkingImage = workingImage;

  this->setEnabled(true);
}

void QmitkSliceBasedInterpolatorWidget::OnTimeChanged(itk::Object *sender, const itk::EventObject &e)
{
  // Check if we really have a GeometryTimeEvent
  if (!dynamic_cast<const mitk::SliceNavigationController::GeometryTimeEvent *>(&e))
    return;

  mitk::SliceNavigationController *slicer = dynamic_cast<mitk::SliceNavigationController *>(sender);
  Q_ASSERT(slicer);

  m_TimePoints[slicer] = slicer->GetSelectedTimePoint();

  // TODO Macht das hier wirklich Sinn????
  if (m_LastSNC == slicer)
  {
    slicer->SendSlice(); // will trigger a new interpolation
  }
}

void QmitkSliceBasedInterpolatorWidget::OnSliceChanged(itk::Object *sender, const itk::EventObject &e)
{
  if (m_Activated && m_WorkingImage.IsNotNull())
  {
    // Check whether we really have a GeometrySliceEvent
    if (!dynamic_cast<const mitk::SliceNavigationController::GeometrySliceEvent *>(&e))
      return;

    mitk::SliceNavigationController *slicer = dynamic_cast<mitk::SliceNavigationController *>(sender);
    if (slicer)
    {
      this->TranslateAndInterpolateChangedSlice(e, slicer);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      //  slicer->GetRenderer()->RequestUpdate();
    }
  }
}

void QmitkSliceBasedInterpolatorWidget::TranslateAndInterpolateChangedSlice(const itk::EventObject &e,
                                                                            mitk::SliceNavigationController *slicer)
{
  if (m_Activated && m_WorkingImage.IsNotNull())
  {
    const mitk::SliceNavigationController::GeometrySliceEvent &geometrySliceEvent =
      dynamic_cast<const mitk::SliceNavigationController::GeometrySliceEvent &>(e);
    mitk::TimeGeometry *timeGeometry = geometrySliceEvent.GetTimeGeometry();
    if (timeGeometry && m_TimePoints.contains(slicer) && timeGeometry->IsValidTimePoint(m_TimePoints[slicer]))
    {
      mitk::SlicedGeometry3D *slicedGeometry =
        dynamic_cast<mitk::SlicedGeometry3D *>(timeGeometry->GetGeometryForTimePoint(m_TimePoints[slicer]).GetPointer());
      if (slicedGeometry)
      {
        mitk::PlaneGeometry *plane = slicedGeometry->GetPlaneGeometry(geometrySliceEvent.GetPos());
        if (plane)
        {
          m_LastSNC = slicer;
          this->Interpolate(plane, m_TimePoints[slicer], slicer);
        }
      }
    }
  }
}

void QmitkSliceBasedInterpolatorWidget::Interpolate(mitk::PlaneGeometry *plane,
                                                    mitk::TimePointType timePoint,
                                                    mitk::SliceNavigationController *slicer)
{
  int clickedSliceDimension(-1);
  int clickedSliceIndex(-1);

  if (!m_WorkingImage->GetTimeGeometry()->IsValidTimePoint(timePoint))
  {
    MITK_WARN << "Cannot interpolate WorkingImage. Passed time point is not within the time bounds of WorkingImage. Time point: " << timePoint;
    return;
  }
  const auto timeStep = m_WorkingImage->GetTimeGeometry()->TimePointToTimeStep(timePoint);

  // calculate real slice position, i.e. slice of the image and not slice of the TimeSlicedGeometry
  // see if timestep is needed here
  mitk::SegTool2D::DetermineAffectedImageSlice(m_WorkingImage, plane, clickedSliceDimension, clickedSliceIndex);

  mitk::Image::Pointer interpolation =
    m_SliceInterpolatorController->Interpolate(clickedSliceDimension, clickedSliceIndex, plane, timeStep);

  m_PreviewNode->SetData(interpolation);

  const mitk::Color &color = m_WorkingImage->GetActiveLabel()->GetColor();
  m_PreviewNode->SetColor(color);

  m_LastSNC = slicer;
  m_LastSliceIndex = clickedSliceIndex;
}

mitk::Image::Pointer QmitkSliceBasedInterpolatorWidget::GetWorkingSlice(const mitk::PlaneGeometry *planeGeometry)
{
  const auto timePoint = m_LastSNC->GetSelectedTimePoint();

  if (!m_WorkingImage->GetTimeGeometry()->IsValidTimePoint(timePoint))
  {
    MITK_WARN << "Cannot get slice of WorkingImage. Time point selected by SliceNavigationController is not within the time bounds of WorkingImage. Time point: " << timePoint;
    return nullptr;
  }

  // Make sure that for reslicing and overwriting the same alogrithm is used. We can specify the mode of the vtk
  // reslicer
  vtkSmartPointer<mitkVtkImageOverwrite> reslice = vtkSmartPointer<mitkVtkImageOverwrite>::New();
  // set to false to extract a slice
  reslice->SetOverwriteMode(false);
  reslice->Modified();

  // use ExtractSliceFilter with our specific vtkImageReslice for overwriting and extracting
  mitk::ExtractSliceFilter::Pointer extractor = mitk::ExtractSliceFilter::New(reslice);
  extractor->SetInput(m_WorkingImage);
  const auto timeStep = m_WorkingImage->GetTimeGeometry()->TimePointToTimeStep(timePoint);
  extractor->SetTimeStep(timeStep);
  extractor->SetWorldGeometry(planeGeometry);
  extractor->SetVtkOutputRequest(false);
  extractor->SetResliceTransformByGeometry(m_WorkingImage->GetTimeGeometry()->GetGeometryForTimeStep(timeStep));

  extractor->Modified();

  try
  {
    extractor->Update();
  }
  catch (itk::ExceptionObject &excep)
  {
    MITK_ERROR << "Exception caught: " << excep.GetDescription();
    return nullptr;
  }

  mitk::Image::Pointer slice = extractor->GetOutput();

  // specify the undo operation with the non edited slice
  // MLI TODO added code starts here
  mitk::SlicedGeometry3D *sliceGeometry = dynamic_cast<mitk::SlicedGeometry3D *>(slice->GetGeometry());
  // m_undoOperation = new mitk::DiffSliceOperation(m_WorkingImage, extractor->GetVtkOutput(), slice->GetGeometry(),
  // timeStep, const_cast<mitk::PlaneGeometry*>(planeGeometry));
  // added code ends here
  m_undoOperation = new mitk::DiffSliceOperation(
    m_WorkingImage, extractor->GetOutput(), sliceGeometry, timeStep, const_cast<mitk::PlaneGeometry *>(planeGeometry));

  slice->DisconnectPipeline();

  return slice;
}

void QmitkSliceBasedInterpolatorWidget::OnToggleWidgetActivation(bool enabled)
{
  Q_ASSERT(m_ToolManager);

  mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);
  if (!workingNode)
    return;

  m_Controls.m_btApplyForCurrentSlice->setEnabled(enabled);
  m_Controls.m_btApplyForAllSlices->setEnabled(enabled);

  if (enabled)
    m_Controls.m_btStart->setText("Stop");
  else
    m_Controls.m_btStart->setText("Start");

  unsigned int numberOfExistingTools = m_ToolManager->GetTools().size();
  for (unsigned int i = 0; i < numberOfExistingTools; i++)
  {
    // mitk::SegTool2D* tool = dynamic_cast<mitk::SegTool2D*>(m_ToolManager->GetToolById(i));
    // MLI TODO
    // if (tool) tool->SetEnable2DInterpolation( enabled );
  }

  if (enabled)
  {
    if (!m_DataStorage->Exists(m_PreviewNode))
    {
      m_DataStorage->Add(m_PreviewNode);
    }

    m_SliceInterpolatorController->SetWorkingImage(m_WorkingImage);
    this->UpdateVisibleSuggestion();
  }
  else
  {
    if (m_DataStorage->Exists(m_PreviewNode))
    {
      m_DataStorage->Remove(m_PreviewNode);
    }

    mitk::UndoController::GetCurrentUndoModel()->Clear();
  }

  m_Activated = enabled;

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

template <typename TPixel, unsigned int VImageDimension>
void QmitkSliceBasedInterpolatorWidget::WritePreviewOnWorkingImage(itk::Image<TPixel, VImageDimension> *targetSlice,
                                                                   const mitk::Image *sourceSlice,
                                                                   int overwritevalue)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;

  typename ImageType::Pointer sourceSliceITK;
  mitk::CastToItkImage(sourceSlice, sourceSliceITK);

  // now the original slice and the ipSegmentation-painted slice are in the same format, and we can just copy all pixels
  // that are non-zero
  typedef itk::ImageRegionIterator<ImageType> OutputIteratorType;
  typedef itk::ImageRegionConstIterator<ImageType> InputIteratorType;

  InputIteratorType inputIterator(sourceSliceITK, sourceSliceITK->GetLargestPossibleRegion());
  OutputIteratorType outputIterator(targetSlice, targetSlice->GetLargestPossibleRegion());

  outputIterator.GoToBegin();
  inputIterator.GoToBegin();

  int activePixelValue = m_WorkingImage->GetActiveLabel()->GetValue();

  if (activePixelValue == 0) // if exterior is the active label
  {
    while (!outputIterator.IsAtEnd())
    {
      if (inputIterator.Get() != 0)
      {
        outputIterator.Set(overwritevalue);
      }
      ++outputIterator;
      ++inputIterator;
    }
  }
  else if (overwritevalue != 0) // if we are not erasing
  {
    while (!outputIterator.IsAtEnd())
    {
      int targetValue = static_cast<int>(outputIterator.Get());
      if (inputIterator.Get() != 0)
      {
        if (!m_WorkingImage->GetLabel(targetValue)->GetLocked())
          outputIterator.Set(overwritevalue);
      }

      ++outputIterator;
      ++inputIterator;
    }
  }
  else // if we are erasing
  {
    while (!outputIterator.IsAtEnd())
    {
      const int targetValue = outputIterator.Get();
      if (inputIterator.Get() != 0)
      {
        if (targetValue == activePixelValue)
          outputIterator.Set(overwritevalue);
      }

      ++outputIterator;
      ++inputIterator;
    }
  }
}

void QmitkSliceBasedInterpolatorWidget::OnAcceptInterpolationClicked()
{
  if (m_WorkingImage.IsNotNull() && m_PreviewNode->GetData())
  {
    const mitk::PlaneGeometry *planeGeometry = m_LastSNC->GetCurrentPlaneGeometry();
    if (!planeGeometry)
      return;

    mitk::Image::Pointer sliceImage = this->GetWorkingSlice(planeGeometry);
    if (sliceImage.IsNull())
      return;

    mitk::Image::Pointer previewSlice = dynamic_cast<mitk::Image *>(m_PreviewNode->GetData());

    AccessFixedDimensionByItk_2(
      sliceImage, WritePreviewOnWorkingImage, 2, previewSlice, m_WorkingImage->GetActiveLabel()->GetValue());

    // Make sure that for reslicing and overwriting the same alogrithm is used. We can specify the mode of the vtk
    // reslicer
    vtkSmartPointer<mitkVtkImageOverwrite> overwrite = vtkSmartPointer<mitkVtkImageOverwrite>::New();
    overwrite->SetInputSlice(sliceImage->GetVtkImageData());
    // set overwrite mode to true to write back to the image volume
    overwrite->SetOverwriteMode(true);
    overwrite->Modified();

    const auto timePoint = m_LastSNC->GetSelectedTimePoint();
    if (!m_WorkingImage->GetTimeGeometry()->IsValidTimePoint(timePoint))
    {
      MITK_WARN << "Cannot accept interpolation. Time point selected by SliceNavigationController is not within the time bounds of WorkingImage. Time point: " << timePoint;
      return;
    }

    mitk::ExtractSliceFilter::Pointer extractor = mitk::ExtractSliceFilter::New(overwrite);
    extractor->SetInput(m_WorkingImage);
    const auto timeStep = m_WorkingImage->GetTimeGeometry()->TimePointToTimeStep(timePoint);
    extractor->SetTimeStep(timeStep);
    extractor->SetWorldGeometry(planeGeometry);
    extractor->SetVtkOutputRequest(false);
    extractor->SetResliceTransformByGeometry(m_WorkingImage->GetTimeGeometry()->GetGeometryForTimeStep(timeStep));

    extractor->Modified();

    try
    {
      extractor->Update();
    }
    catch (itk::ExceptionObject &excep)
    {
      MITK_ERROR << "Exception caught: " << excep.GetDescription();
      return;
    }

    // the image was modified within the pipeline, but not marked so
    m_WorkingImage->Modified();

    int clickedSliceDimension(-1);
    int clickedSliceIndex(-1);

    mitk::SegTool2D::DetermineAffectedImageSlice(
      m_WorkingImage, planeGeometry, clickedSliceDimension, clickedSliceIndex);

    m_SliceInterpolatorController->SetChangedSlice(sliceImage, clickedSliceDimension, clickedSliceIndex, timeStep);

    // specify the undo operation with the edited slice
    // MLI TODO added code starts here
    mitk::SlicedGeometry3D *sliceGeometry = dynamic_cast<mitk::SlicedGeometry3D *>(sliceImage->GetGeometry());
    // m_undoOperation = new mitk::DiffSliceOperation(m_WorkingImage, extractor->GetVtkOutput(), slice->GetGeometry(),
    // timeStep, const_cast<mitk::PlaneGeometry*>(planeGeometry));
    // added code ends here
    m_doOperation = new mitk::DiffSliceOperation(m_WorkingImage,
                                                 extractor->GetOutput(),
                                                 sliceGeometry,
                                                 timeStep,
                                                 const_cast<mitk::PlaneGeometry *>(planeGeometry));

    // create an operation event for the undo stack
    mitk::OperationEvent *undoStackItem = new mitk::OperationEvent(
      mitk::DiffSliceOperationApplier::GetInstance(), m_doOperation, m_undoOperation, "Slice Interpolation");

    // add it to the undo controller
    mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent(undoStackItem);

    // clear the pointers as the operation are stored in the undo controller and also deleted from there
    m_undoOperation = nullptr;
    m_doOperation = nullptr;

    m_PreviewNode->SetData(nullptr);

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkSliceBasedInterpolatorWidget::AcceptAllInterpolations(mitk::SliceNavigationController *slicer)
{
  // Since we need to shift the plane it must be clone so that the original plane isn't altered
  mitk::PlaneGeometry::Pointer reslicePlane = slicer->GetCurrentPlaneGeometry()->Clone();
  const auto timePoint = slicer->GetSelectedTimePoint();
  if (!m_WorkingImage->GetTimeGeometry()->IsValidTimePoint(timePoint))
  {
    MITK_WARN << "Cannot accept all interpolations. Time point selected by SliceNavigationController is not within the time bounds of WorkingImage. Time point: " << timePoint;

    return;
  }
  const auto timeStep = m_WorkingImage->GetTimeGeometry()->TimePointToTimeStep(timePoint);

  int sliceDimension(-1);
  int sliceIndex(-1);

  mitk::SegTool2D::DetermineAffectedImageSlice(m_WorkingImage, reslicePlane, sliceDimension, sliceIndex);

  unsigned int zslices = m_WorkingImage->GetDimension(sliceDimension);

  mitk::ProgressBar::GetInstance()->Reset();
  mitk::ProgressBar::GetInstance()->AddStepsToDo(zslices);

  mitk::Point3D origin = reslicePlane->GetOrigin();

  for (unsigned int idx = 0; idx < zslices; ++idx)
  {
    // Transforming the current origin of the reslice plane
    // so that it matches the one of the next slice
    m_WorkingImage->GetSlicedGeometry()->WorldToIndex(origin, origin);
    origin[sliceDimension] = idx;
    m_WorkingImage->GetSlicedGeometry()->IndexToWorld(origin, origin);
    reslicePlane->SetOrigin(origin);

    mitk::Image::Pointer interpolation =
      m_SliceInterpolatorController->Interpolate(sliceDimension, idx, reslicePlane, timeStep);

    if (interpolation.IsNotNull())
    {
      m_PreviewNode->SetData(interpolation);

      mitk::Image::Pointer sliceImage = this->GetWorkingSlice(reslicePlane);
      if (sliceImage.IsNull())
        return;

      AccessFixedDimensionByItk_2(
        sliceImage, WritePreviewOnWorkingImage, 2, interpolation, m_WorkingImage->GetActiveLabel()->GetValue());

      // Make sure that for reslicing and overwriting the same alogrithm is used. We can specify the mode of the vtk
      // reslicer
      vtkSmartPointer<mitkVtkImageOverwrite> overwrite = vtkSmartPointer<mitkVtkImageOverwrite>::New();
      overwrite->SetInputSlice(sliceImage->GetVtkImageData());
      // set overwrite mode to true to write back to the image volume
      overwrite->SetOverwriteMode(true);
      overwrite->Modified();

      mitk::ExtractSliceFilter::Pointer extractor = mitk::ExtractSliceFilter::New(overwrite);
      extractor->SetInput(m_WorkingImage);
      extractor->SetTimeStep(timeStep);
      extractor->SetWorldGeometry(reslicePlane);
      extractor->SetVtkOutputRequest(true);
      extractor->SetResliceTransformByGeometry(m_WorkingImage->GetTimeGeometry()->GetGeometryForTimeStep(timeStep));

      extractor->Modified();

      try
      {
        extractor->Update();
      }
      catch (itk::ExceptionObject &excep)
      {
        MITK_ERROR << "Exception caught: " << excep.GetDescription();
        return;
      }

      m_WorkingImage->Modified();

      mitk::RenderingManager::GetInstance()->RequestUpdateAll(mitk::RenderingManager::REQUEST_UPDATE_2DWINDOWS);
    }

    mitk::ProgressBar::GetInstance()->Progress();
  }

  m_SliceInterpolatorController->SetWorkingImage(m_WorkingImage);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSliceBasedInterpolatorWidget::OnAcceptAllInterpolationsClicked()
{
  QMenu orientationPopup(this);
  std::map<QAction *, mitk::SliceNavigationController *>::const_iterator it;
  for (it = m_ActionToSliceDimensionMap.begin(); it != m_ActionToSliceDimensionMap.end(); it++)
    orientationPopup.addAction(it->first);

  connect(&orientationPopup, SIGNAL(triggered(QAction *)), this, SLOT(OnAcceptAllPopupActivated(QAction *)));

  orientationPopup.exec(QCursor::pos());
}

void QmitkSliceBasedInterpolatorWidget::OnAcceptAllPopupActivated(QAction *action)
{
  ActionToSliceDimensionMapType::const_iterator iter = m_ActionToSliceDimensionMap.find(action);
  if (iter != m_ActionToSliceDimensionMap.end())
  {
    mitk::SliceNavigationController *slicer = iter->second;
    this->AcceptAllInterpolations(slicer);
  }
}

void QmitkSliceBasedInterpolatorWidget::UpdateVisibleSuggestion()
{
  if (m_Activated && m_LastSNC)
  {
    // determine which one is the current view, try to do an initial interpolation
    mitk::BaseRenderer *renderer = m_LastSNC->GetRenderer();
    if (renderer && renderer->GetMapperID() == mitk::BaseRenderer::Standard2D)
    {
      const mitk::TimeGeometry *timeGeometry =
        dynamic_cast<const mitk::TimeGeometry *>(renderer->GetWorldTimeGeometry());
      if (timeGeometry)
      {
        mitk::SliceNavigationController::GeometrySliceEvent event(const_cast<mitk::TimeGeometry *>(timeGeometry),
                                                                  renderer->GetSlice());
        this->TranslateAndInterpolateChangedSlice(event, m_LastSNC);
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      }
    }
  }
}

void QmitkSliceBasedInterpolatorWidget::OnSliceInterpolationInfoChanged(const itk::EventObject & /*e*/)
{
  // something (e.g. undo) changed the interpolation info, we should refresh our display
  this->UpdateVisibleSuggestion();
}

void QmitkSliceBasedInterpolatorWidget::OnSliceNavigationControllerDeleted(const itk::Object *sender,
                                                                           const itk::EventObject & /*e*/)
{
  // Don't know how to avoid const_cast here?!
  mitk::SliceNavigationController *slicer =
    dynamic_cast<mitk::SliceNavigationController *>(const_cast<itk::Object *>(sender));
  if (slicer)
  {
    m_ControllerToTimeObserverTag.remove(slicer);
    m_ControllerToSliceObserverTag.remove(slicer);
    m_ControllerToDeleteObserverTag.remove(slicer);
  }
}

void QmitkSliceBasedInterpolatorWidget::WaitCursorOn()
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
}

void QmitkSliceBasedInterpolatorWidget::WaitCursorOff()
{
  this->RestoreOverrideCursor();
}

void QmitkSliceBasedInterpolatorWidget::RestoreOverrideCursor()
{
  QApplication::restoreOverrideCursor();
}
