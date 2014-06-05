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

#include "QmitkSliceWidget.h"
#include "QmitkStepperAdapter.h"
#include "mitkNodePredicateDataType.h"
#include "mitkImage.h"
#include <mitkProportionalTimeGeometry.h>
#include <QMenu>
#include <QMouseEvent>

QmitkSliceWidget::QmitkSliceWidget(QWidget* parent, const char* name,
    Qt::WindowFlags f) :
  QWidget(parent, f)
{
  this->setupUi(this);

  if (name != 0)
    this->setObjectName(name);

  popUp = new QMenu(this);
  popUp->addAction("Axial");
  popUp->addAction("Frontal");
  popUp->addAction("Sagittal");

  QObject::connect(popUp, SIGNAL(triggered(QAction*)), this, SLOT(ChangeView(QAction*)) );
  setPopUpEnabled(false);

  m_SlicedGeometry = 0;
  m_View = mitk::SliceNavigationController::Axial;

  QHBoxLayout *hlayout = new QHBoxLayout(container);
  hlayout->setMargin(0);

  // create widget
  QString composedName("QmitkSliceWidget::");
  if (!this->objectName().isEmpty())
    composedName += this->objectName();
  else
    composedName += "QmitkGLWidget";
  m_RenderWindow = new QmitkRenderWindow(container, composedName);
  m_Renderer = m_RenderWindow->GetRenderer();
  hlayout->addWidget(m_RenderWindow);

  new QmitkStepperAdapter(m_NavigatorWidget,
      m_RenderWindow->GetSliceNavigationController()->GetSlice(),
      "navigation");

  SetLevelWindowEnabled(true);
}

mitk::VtkPropRenderer* QmitkSliceWidget::GetRenderer()
{
  return m_Renderer;
}

QFrame* QmitkSliceWidget::GetSelectionFrame()
{
  return SelectionFrame;
}

void QmitkSliceWidget::SetDataStorage(
    mitk::StandaloneDataStorage::Pointer storage)
{
  m_DataStorage = storage;
  m_Renderer->SetDataStorage(m_DataStorage);
}

mitk::StandaloneDataStorage* QmitkSliceWidget::GetDataStorage()
{
  return m_DataStorage;
}

void QmitkSliceWidget::SetData(
    mitk::DataStorage::SetOfObjects::ConstIterator it)
{
  SetData(it->Value(), m_View);
}

void QmitkSliceWidget::SetData(
    mitk::DataStorage::SetOfObjects::ConstIterator it,
    mitk::SliceNavigationController::ViewDirection view)
{
  SetData(it->Value(), view);
}

void QmitkSliceWidget::SetData(mitk::DataNode::Pointer node)
{
  try
  {
    if (m_DataStorage.IsNotNull())
    {
      m_DataStorage->Add(node);
    }
  } catch (...)
  {
  }
  SetData(node, m_View);
}

void QmitkSliceWidget::SetData(mitk::DataNode::Pointer node,
    mitk::SliceNavigationController::ViewDirection view)
{
  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

  if (image.IsNull())
  {
    MITK_WARN << "QmitkSliceWidget data is not an image!";
    return;
  }

  m_SlicedGeometry = image->GetSlicedGeometry();
  this->InitWidget(view);
}

void QmitkSliceWidget::InitWidget(
    mitk::SliceNavigationController::ViewDirection viewDirection)
{
  m_View = viewDirection;

  mitk::SliceNavigationController* controller =
      m_RenderWindow->GetSliceNavigationController();

  if (viewDirection == mitk::SliceNavigationController::Axial)
  {
    controller->SetViewDirection(
        mitk::SliceNavigationController::Axial);
  }
  else if (viewDirection == mitk::SliceNavigationController::Frontal)
  {
    controller->SetViewDirection(mitk::SliceNavigationController::Frontal);
  }
  // init sagittal view
  else
  {
    controller->SetViewDirection(mitk::SliceNavigationController::Sagittal);
  }

  if (m_SlicedGeometry.IsNull())
  {
    return;
  }


  mitk::BaseGeometry::Pointer geometry =
          static_cast<mitk::BaseGeometry*> (m_SlicedGeometry->Clone().GetPointer());

  const mitk::BoundingBox::Pointer boundingbox =
    m_DataStorage->ComputeVisibleBoundingBox(GetRenderer(), NULL);

  if (boundingbox->GetPoints()->Size() > 0)
  {
    //let's see if we have data with a limited live-span ...
    mitk::TimeBounds timebounds = m_DataStorage->ComputeTimeBounds(
        GetRenderer(), NULL);

    mitk::ProportionalTimeGeometry::Pointer timeGeometry = mitk::ProportionalTimeGeometry::New();
    timeGeometry->Initialize(geometry, 1);

    {
      timeGeometry->SetFirstTimePoint(timebounds[0]);
      timeGeometry->SetStepDuration(1.0);
    }

    if (const_cast<mitk::BoundingBox*> (timeGeometry->GetBoundingBoxInWorld())->GetDiagonalLength2()
        >= mitk::eps)
    {
      controller->SetInputWorldTimeGeometry(timeGeometry);
      controller->Update();
    }
  }

  GetRenderer()->GetDisplayGeometry()->Fit();
  mitk::RenderingManager::GetInstance()->RequestUpdate(
      GetRenderer()->GetRenderWindow());
}

void QmitkSliceWidget::UpdateGL()
{
  GetRenderer()->GetDisplayGeometry()->Fit();
  mitk::RenderingManager::GetInstance()->RequestUpdate(
      GetRenderer()->GetRenderWindow());
}

void QmitkSliceWidget::mousePressEvent(QMouseEvent * e)
{
  if (e->button() == Qt::RightButton && popUpEnabled)
  {
    popUp->popup(QCursor::pos());
  }
}

void QmitkSliceWidget::wheelEvent(QWheelEvent * e)
{
  int val = m_NavigatorWidget->GetPos();

  if (e->orientation() * e->delta() > 0)
  {
    m_NavigatorWidget->SetPos(val + 1);
  }
  else
  {
    if (val > 0)
      m_NavigatorWidget->SetPos(val - 1);
  }
}

void QmitkSliceWidget::ChangeView(QAction* val)
{
  if (val->text() == "Axial")
  {
    InitWidget(mitk::SliceNavigationController::Axial);
  }
  else if (val->text() == "Frontal")
  {
    InitWidget(mitk::SliceNavigationController::Frontal);
  }
  else if (val->text() == "Sagittal")
  {
    InitWidget(mitk::SliceNavigationController::Sagittal);
  }
}

void QmitkSliceWidget::setPopUpEnabled(bool b)
{
  popUpEnabled = b;
}

QmitkSliderNavigatorWidget* QmitkSliceWidget::GetNavigatorWidget()
{
  return m_NavigatorWidget;
}

void QmitkSliceWidget::SetLevelWindowEnabled(bool enable)
{
  levelWindow->setEnabled(enable);
  if (!enable)
  {
    levelWindow->setMinimumWidth(0);
    levelWindow->setMaximumWidth(0);
  }
  else
  {
    levelWindow->setMinimumWidth(28);
    levelWindow->setMaximumWidth(28);
  }
}

bool QmitkSliceWidget::IsLevelWindowEnabled()
{
  return levelWindow->isEnabled();
}

QmitkRenderWindow* QmitkSliceWidget::GetRenderWindow()
{
  return m_RenderWindow;
}

mitk::SliceNavigationController*
QmitkSliceWidget::GetSliceNavigationController() const
{
  return m_RenderWindow->GetSliceNavigationController();
}

mitk::CameraRotationController*
QmitkSliceWidget::GetCameraRotationController() const
{
  return m_RenderWindow->GetCameraRotationController();
}

mitk::BaseController*
QmitkSliceWidget::GetController() const
{
  return m_RenderWindow->GetController();
}
