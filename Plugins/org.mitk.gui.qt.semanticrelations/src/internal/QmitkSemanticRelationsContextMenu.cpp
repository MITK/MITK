/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// semantic relations plugin
#include "QmitkSemanticRelationsContextMenu.h"

QmitkSemanticRelationsContextMenu::QmitkSemanticRelationsContextMenu(berry::IWorkbenchPartSite::Pointer workbenchPartSite, QWidget* parent)
  : QMenu(parent)
  , m_Parent(parent)
  , m_WorkbenchPartSite(workbenchPartSite)
{
  InitDefaultActions();
}

void QmitkSemanticRelationsContextMenu::SetDataStorage(mitk::DataStorage* dataStorage)
{
  if (m_DataStorage != dataStorage)
  {
    // set the new data storage - also for all actions
    m_DataStorage = dataStorage;
    m_ControlPointAction->SetDataStorage(m_DataStorage.Lock());
    m_InformationTypeAction->SetDataStorage(m_DataStorage.Lock());
    m_RemoveFromSemanticRelationsAction->SetDataStorage(m_DataStorage.Lock());
    m_DataSetOpenInAction->SetDataStorage(m_DataStorage.Lock());
    m_UnlinkFromLesionAction->SetDataStorage(m_DataStorage.Lock());
  }
}

void QmitkSemanticRelationsContextMenu::SetControlledRenderer(mitk::RenderWindowLayerUtilities::RendererVector controlledRenderer)
{
  if (m_ControlledRenderer != controlledRenderer)
  {
    // set the new set of controlled renderer
    m_ControlledRenderer = controlledRenderer;
    m_DataSetOpenInAction->SetControlledRenderer(m_ControlledRenderer);
  }
}

void QmitkSemanticRelationsContextMenu::OnContextMenuRequested(const QPoint& /*pos*/)
{
  popup(QCursor::pos());
}

void QmitkSemanticRelationsContextMenu::InitDefaultActions()
{
  m_ControlPointAction = new QmitkDataNodeSetControlPointAction(m_Parent, m_WorkbenchPartSite.Lock());
  addAction(m_ControlPointAction);

  m_InformationTypeAction = new QmitkDataNodeSetInformationTypeAction(m_Parent, m_WorkbenchPartSite.Lock());
  addAction(m_InformationTypeAction);

  m_UnlinkFromLesionAction = new QmitkDataNodeUnlinkFromLesionAction(m_Parent, m_WorkbenchPartSite.Lock());
  addAction(m_UnlinkFromLesionAction);

  m_RemoveFromSemanticRelationsAction = new QmitkDataNodeRemoveFromSemanticRelationsAction(m_Parent, m_WorkbenchPartSite.Lock());
  addAction(m_RemoveFromSemanticRelationsAction);

  m_DataSetOpenInAction = new QmitkDataSetOpenInAction(m_Parent, m_WorkbenchPartSite.Lock());
  addAction(m_DataSetOpenInAction);
}
