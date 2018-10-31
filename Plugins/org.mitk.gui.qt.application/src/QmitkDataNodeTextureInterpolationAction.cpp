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

#include <QmitkDataNodeTextureInterpolationAction.h>

// mitk core
#include <mitkRenderingManager.h>

QmitkDataNodeTextureInterpolationAction::QmitkDataNodeTextureInterpolationAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchpartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(workbenchpartSite)
{
  setText(tr("Texture Interpolation"));
  InitializeAction();
}

QmitkDataNodeTextureInterpolationAction::QmitkDataNodeTextureInterpolationAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchpartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer(workbenchpartSite))
{
  setText(tr("Texture Interpolation"));
  InitializeAction();
}

QmitkDataNodeTextureInterpolationAction::~QmitkDataNodeTextureInterpolationAction()
{
  // nothing here
}

void QmitkDataNodeTextureInterpolationAction::InitializeAction()
{
  setCheckable(true);

  connect(this, &QmitkDataNodeTextureInterpolationAction::toggled, this, &QmitkDataNodeTextureInterpolationAction::OnActionToggled);
  connect(this, &QmitkDataNodeTextureInterpolationAction::changed, this, &QmitkDataNodeTextureInterpolationAction::OnActionChanged);
}

void QmitkDataNodeTextureInterpolationAction::InitializeWithDataNode(const mitk::DataNode* dataNode)
{
  bool textureInterpolation = false;
  dataNode->GetBoolProperty("texture interpolation", textureInterpolation);
  setChecked(textureInterpolation);
}

void QmitkDataNodeTextureInterpolationAction::OnActionToggled(bool checked)
{
  auto dataNode = GetSelectedNode();
  if (dataNode.IsNull())
  {
    return;
  }

  dataNode->SetBoolProperty("texture interpolation", checked);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDataNodeTextureInterpolationAction::OnActionChanged()
{
  auto dataNode = GetSelectedNode();
  if (dataNode.IsNull())
  {
    return;
  }

  InitializeWithDataNode(dataNode);
}
