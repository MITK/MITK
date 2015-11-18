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

#include "QmlMitkFourRenderWindowWidget.h"

#include "mitkDisplayInteractor.h"

#include "usGetModuleContext.h"

#include <QtQuick>

#include <stdexcept>

QmlMitkFourRenderWindowWidget::QmlMitkFourRenderWindowWidget(QQuickItem* parent)
: QQuickItem(parent)
, m_ChildrenContainer(NULL)
, m_RenderItemAxial(NULL)
, m_RenderItemSagittal(NULL)
, m_RenderItemFrontal(NULL)
, m_RenderItem3D(NULL)
{
  QQmlEngine engine;
  QQmlComponent component(&engine, QUrl("qrc:///MITK/Modules/QmlMitk/QmlMitkFourRenderWindowWidget.qml"));
  m_ChildrenContainer = qobject_cast<QQuickItem*>( component.create() );

  if (m_ChildrenContainer)
  {
    QQmlProperty::write(m_ChildrenContainer, "parent", QVariant::fromValue<QObject*>(this));
    QQmlEngine::setObjectOwnership(m_ChildrenContainer, QQmlEngine::CppOwnership);

    SetupWidget(m_ChildrenContainer);
  }
  else
  {
    throw std::logic_error("Initialization of QmlMitkFourRenderWindowWidget went dead wrong. Check code...");
  }
}

QmlMitkFourRenderWindowWidget::~QmlMitkFourRenderWindowWidget()
{
  if (m_ChildrenContainer)
  {
    m_ChildrenContainer->deleteLater();
  }
}

void QmlMitkFourRenderWindowWidget::SetupWidget( QQuickItem* parent )
{
  m_RenderItemAxial = parent->findChild<QmlMitkRenderWindowItem*>("mitkRenderItemAxial");
  m_RenderItemSagittal = parent->findChild<QmlMitkRenderWindowItem*>("mitkRenderItemSagittal");
  m_RenderItemFrontal = parent->findChild<QmlMitkRenderWindowItem*>("mitkRenderItemFrontal");
  m_RenderItem3D = parent->findChild<QmlMitkRenderWindowItem*>("mitkRenderItem3D");

  if (m_RenderItemAxial && m_RenderItemSagittal && m_RenderItemFrontal && m_RenderItem3D)
  {
    m_RenderItemAxial->GetRenderer()->SetMapperID( mitk::BaseRenderer::Standard2D );
    m_RenderItemAxial->GetRenderer()->GetSliceNavigationController()->SetDefaultViewDirection( mitk::SliceNavigationController::Axial );

    m_RenderItemSagittal->GetRenderer()->SetMapperID( mitk::BaseRenderer::Standard2D );
    m_RenderItemSagittal->GetRenderer()->GetSliceNavigationController()->SetDefaultViewDirection( mitk::SliceNavigationController::Sagittal );

    m_RenderItemFrontal->GetRenderer()->SetMapperID( mitk::BaseRenderer::Standard2D );
    m_RenderItemFrontal->GetRenderer()->GetSliceNavigationController()->SetDefaultViewDirection( mitk::SliceNavigationController::Frontal );

    m_RenderItem3D->GetRenderer()->SetMapperID( mitk::BaseRenderer::Standard3D );
    m_RenderItem3D->GetRenderer()->GetSliceNavigationController()->SetDefaultViewDirection( mitk::SliceNavigationController::Original );

    InitializeMoveZoomInteraction();
  }
}

void QmlMitkFourRenderWindowWidget::InitializeMoveZoomInteraction()
{
  static mitk::DisplayInteractor::Pointer m_ZoomScroller = mitk::DisplayInteractor::New();
  m_ZoomScroller->LoadStateMachine("DisplayInteraction.xml");
  m_ZoomScroller->SetEventConfig("DisplayConfigMITK.xml");

  us::ModuleContext* context = us::GetModuleContext();
  context->RegisterService<mitk::InteractionEventObserver>( m_ZoomScroller.GetPointer() );
}


void QmlMitkFourRenderWindowWidget::SetDataStorage( mitk::DataStorage::Pointer storage )
{
  m_DataStorage = storage;

  // TODO make this conditional, only add if not yet preset...
  m_RenderItemAxial->GetRenderer()->SetDataStorage( m_DataStorage );
  m_RenderItemSagittal->GetRenderer()->SetDataStorage( m_DataStorage );
  m_RenderItemFrontal->GetRenderer()->SetDataStorage( m_DataStorage );
  m_RenderItem3D->GetRenderer()->SetDataStorage( m_DataStorage );
}

