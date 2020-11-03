/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKSEMANTICRELATIONSCONTEXTMENU_H
#define QMITKSEMANTICRELATIONSCONTEXTMENU_H

// semantic relations plugin
#include "QmitkDataNodeSetControlPointAction.h"
#include "QmitkDataNodeSetInformationTypeAction.h"
#include "QmitkDataNodeUnlinkFromLesionAction.h"
#include "QmitkDataNodeRemoveFromSemanticRelationsAction.h"
#include "QmitkDataSetOpenInAction.h"

// mitk core
#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>

// mitk render window manager module
#include <mitkRenderWindowLayerUtilities.h>

// blueberry ui qt plugin
#include <berryIWorkbenchPartSite.h>

//qt
#include <QMenu>

class QmitkSemanticRelationsContextMenu : public QMenu
{
  Q_OBJECT

public:

  QmitkSemanticRelationsContextMenu(berry::IWorkbenchPartSite::Pointer workbenchPartSite, QWidget* parent = nullptr);

  void SetDataStorage(mitk::DataStorage* dataStorage);
  void SetControlledRenderer(mitk::RenderWindowLayerUtilities::RendererVector controlledRenderer);

public Q_SLOTS:

  void OnContextMenuRequested(const QPoint&);

private:

  void InitDefaultActions();

  QWidget* m_Parent;
  berry::IWorkbenchPartSite::WeakPtr m_WorkbenchPartSite;
  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
  mitk::RenderWindowLayerUtilities::RendererVector m_ControlledRenderer;

  QmitkDataNodeSetControlPointAction* m_ControlPointAction;
  QmitkDataNodeSetInformationTypeAction* m_InformationTypeAction;
  QmitkDataNodeUnlinkFromLesionAction* m_UnlinkFromLesionAction;
  QmitkDataNodeRemoveFromSemanticRelationsAction* m_RemoveFromSemanticRelationsAction;
  QmitkDataSetOpenInAction* m_DataSetOpenInAction;
};

#endif // QMITKSEMANTICRELATIONSCONTEXTMENU_H
