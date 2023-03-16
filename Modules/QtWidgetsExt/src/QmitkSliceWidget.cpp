/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSliceWidget.h"
#include "QmitkStepperAdapter.h"
#include "mitkCameraController.h"
#include "mitkImage.h"
#include "mitkNodePredicateDataType.h"
#include <QMenu>
#include <QMouseEvent>
#include <mitkCameraController.h>
#include <mitkProportionalTimeGeometry.h>

QmitkSliceWidget::QmitkSliceWidget(QWidget *parent, const char *name, Qt::WindowFlags f) : QWidget(parent, f)
{
  this->setupUi(this);

  if (name != nullptr)
    this->setObjectName(name);

  popUp = new QMenu(this);
  popUp->addAction("Axial");
  popUp->addAction("Coronal");
  popUp->addAction("Sagittal");

  QObject::connect(popUp, SIGNAL(triggered(QAction *)), this, SLOT(ChangeView(QAction *)));
  setPopUpEnabled(false);

  m_SlicedGeometry = nullptr;
  m_View = mitk::AnatomicalPlane::Axial;

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

  new QmitkStepperAdapter(sliceNavigationWidget, m_RenderWindow->GetSliceNavigationController()->GetSlice());

  SetLevelWindowEnabled(true);
}

mitk::VtkPropRenderer *QmitkSliceWidget::GetRenderer()
{
  return m_Renderer;
}

QFrame *QmitkSliceWidget::GetSelectionFrame()
{
  return SelectionFrame;
}

void QmitkSliceWidget::SetDataStorage(mitk::StandaloneDataStorage::Pointer storage)
{
  m_DataStorage = storage;
  m_Renderer->SetDataStorage(m_DataStorage);
}

mitk::StandaloneDataStorage *QmitkSliceWidget::GetDataStorage()
{
  return m_DataStorage;
}

void QmitkSliceWidget::SetData(mitk::DataStorage::SetOfObjects::ConstIterator it)
{
  SetData(it->Value(), m_View);
}

void QmitkSliceWidget::SetData(mitk::DataStorage::SetOfObjects::ConstIterator it,
                               mitk::AnatomicalPlane view)
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
  }
  catch (...)
  {
  }
  SetData(node, m_View);
}

void QmitkSliceWidget::SetData(mitk::DataNode::Pointer node, mitk::AnatomicalPlane view)
{
  mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(node->GetData());

  if (image.IsNull())
  {
    MITK_WARN << "QmitkSliceWidget data is not an image!";
    return;
  }

  m_SlicedGeometry = image->GetSlicedGeometry();
  this->InitWidget(view);
}

void QmitkSliceWidget::InitWidget(mitk::AnatomicalPlane view)
{
  m_View = view;

  mitk::SliceNavigationController *controller = m_RenderWindow->GetSliceNavigationController();

  if (view == mitk::AnatomicalPlane::Axial)
  {
    controller->SetViewDirection(mitk::AnatomicalPlane::Axial);
  }
  else if (view == mitk::AnatomicalPlane::Coronal)
  {
    controller->SetViewDirection(mitk::AnatomicalPlane::Coronal);
  }
  // init sagittal view for all other cases ('original' is covered here as well)
  else
  {
    controller->SetViewDirection(mitk::AnatomicalPlane::Sagittal);
  }

  if (m_SlicedGeometry.IsNull())
  {
    return;
  }

  mitk::BaseGeometry::Pointer geometry = static_cast<mitk::BaseGeometry *>(m_SlicedGeometry->Clone().GetPointer());

  const mitk::BoundingBox::Pointer boundingbox = m_DataStorage->ComputeVisibleBoundingBox(GetRenderer(), nullptr);

  if (boundingbox->GetPoints()->Size() > 0)
  {
    // let's see if we have data with a limited live-span ...
    mitk::TimeBounds timebounds = m_DataStorage->ComputeTimeBounds(GetRenderer(), nullptr);

    mitk::ProportionalTimeGeometry::Pointer timeGeometry = mitk::ProportionalTimeGeometry::New();
    timeGeometry->Initialize(geometry, 1);

    {
      timeGeometry->SetFirstTimePoint(timebounds[0]);
      timeGeometry->SetStepDuration(1.0);
    }

    if (timeGeometry->GetBoundingBoxInWorld()->GetDiagonalLength2() >= mitk::eps)
    {
      controller->SetInputWorldTimeGeometry(timeGeometry);
      controller->Update();
    }
  }

  GetRenderer()->GetCameraController()->Fit();
  mitk::RenderingManager::GetInstance()->RequestUpdate(GetRenderer()->GetRenderWindow());
}

void QmitkSliceWidget::UpdateGL()
{
  GetRenderer()->GetCameraController()->Fit();
  mitk::RenderingManager::GetInstance()->RequestUpdate(GetRenderer()->GetRenderWindow());
}

void QmitkSliceWidget::mousePressEvent(QMouseEvent *e)
{
  if (e->button() == Qt::RightButton && popUpEnabled)
  {
    popUp->popup(QCursor::pos());
  }
}

void QmitkSliceWidget::wheelEvent(QWheelEvent *e)
{
  int val = sliceNavigationWidget->GetPos();

  if (e->orientation() * e->delta() > 0)
  {
    sliceNavigationWidget->SetPos(val + 1);
  }
  else
  {
    if (val > 0)
      sliceNavigationWidget->SetPos(val - 1);
  }
}

void QmitkSliceWidget::ChangeView(QAction *val)
{
  if (val->text() == "Axial")
  {
    InitWidget(mitk::AnatomicalPlane::Axial);
  }
  else if (val->text() == "Coronal")
  {
    InitWidget(mitk::AnatomicalPlane::Coronal);
  }
  else if (val->text() == "Sagittal")
  {
    InitWidget(mitk::AnatomicalPlane::Sagittal);
  }
}

void QmitkSliceWidget::setPopUpEnabled(bool b)
{
  popUpEnabled = b;
}

QmitkSliceNavigationWidget* QmitkSliceWidget::GetSliceNavigationWidget()
{
  return sliceNavigationWidget;
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

QmitkRenderWindow *QmitkSliceWidget::GetRenderWindow()
{
  return m_RenderWindow;
}

mitk::SliceNavigationController *QmitkSliceWidget::GetSliceNavigationController() const
{
  return m_RenderWindow->GetSliceNavigationController();
}

mitk::CameraRotationController *QmitkSliceWidget::GetCameraRotationController() const
{
  return m_RenderWindow->GetCameraRotationController();
}

mitk::BaseController *QmitkSliceWidget::GetController() const
{
  return m_RenderWindow->GetController();
}
