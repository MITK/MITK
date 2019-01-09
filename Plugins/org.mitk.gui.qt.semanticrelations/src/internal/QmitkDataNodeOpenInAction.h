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

#ifndef QMITKDATANODEOPENINACTION_H
#define QMITKDATANODEOPENINACTION_H

// mitk core
#include <mitkDataNode.h>
#include <mitkWeakPointer.h>

// mitk render window manager module
#include <mitkRenderWindowLayerUtilities.h>

// berry
#include <berryIWorkbenchPartSite.h>

// qt
#include <QAction.h>

class QmitkDataNodeOpenInAction : public QAction
{
  Q_OBJECT

public:

  QmitkDataNodeOpenInAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkDataNodeOpenInAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

  virtual ~QmitkDataNodeOpenInAction() override;

  void SetControlledRenderer(RenderWindowLayerUtilities::RendererVector controlledRenderer);

private Q_SLOTS:

  void OnMenuAboutToShow();
  void OnActionTriggered(bool);

protected:

  void InitializeAction();
  void SetControlledRenderer();

  QList<mitk::DataNode::Pointer> GetSelectedNodes();
  mitk::DataNode::Pointer GetSelectedNode();

  berry::IWorkbenchPartSite::WeakPtr m_WorkbenchPartSite;

  RenderWindowLayerUtilities::RendererVector m_ControlledRenderer;

};

#endif // QMITKDATANODEOPENINACTION_H
