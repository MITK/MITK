/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "Qmitk3DMultiSegVisStyleAction.h"

// mitk core
#include <mitkCoreServices.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <mitkImage.h>
#include <mitkLookupTable.h>
#include <mitkLookupTableProperty.h>
#include <mitkRenderingManager.h>
#include <mitkRenderingModeProperty.h>

// mitk gui common plugin
#include <mitkDataNodeSelection.h>

// qt
#include <QMenu>

namespace
{
  // The three rendering modes the context menu exposes for a multi-label segmentation.
  // Encoded as a single int stored in the QAction data so the triggered slot can
  // dispatch from the sender.
  enum class Vis3DMode : int
  {
    Off = 0,
    Exact = 1,    // Surface extracted at the literal voxel boundary, no smoothing.
    Smoothed = 2  // Surface with constrained smoothing applied.
  };

  // Mirrors mitk::MultiLabelSegmentationVtkMapper3D::ResolveSmoothed. Duplicated
  // rather than reused because org.mitk.gui.qt.application must not depend on
  // MitkMultilabel. Keep both copies in sync if defaults or preference keys change.
  bool ResolveSmoothed(mitk::DataNode* node, mitk::BaseRenderer* renderer)
  {
    bool smoothed = true;
    if (node->GetBoolProperty("org.mitk.multilabel.3D.smoothed", smoothed, renderer))
    {
      return smoothed;
    }
    if (auto* prefService = mitk::CoreServices::GetPreferencesService())
    {
      if (auto* systemPref = prefService->GetSystemPreferences())
      {
        return systemPref->Node("/org.mitk.views.segmentation")->GetBool("3D rendering smoothed", true);
      }
    }
    return true;
  }
}

Qmitk3DMultiSegVisStyleAction::Qmitk3DMultiSegVisStyleAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchpartSite)
  : Qmitk3DMultiSegVisStyleAction(parent, workbenchpartSite.GetPointer())
{
}

Qmitk3DMultiSegVisStyleAction::Qmitk3DMultiSegVisStyleAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchpartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer(workbenchpartSite))
{
  this->setText(tr("3D visualization"));
  this->InitializeAction();
}

void Qmitk3DMultiSegVisStyleAction::InitializeAction()
{
  this->setMenu(new QMenu);
  connect(this->menu(), &QMenu::aboutToShow, this, &Qmitk3DMultiSegVisStyleAction::OnMenuAboutShow);
}

void Qmitk3DMultiSegVisStyleAction::OnMenuAboutShow()
{
  auto dataNode = GetSelectedNode();
  if (dataNode.IsNull())
  {
    return;
  }

  mitk::BaseRenderer::Pointer baseRenderer = GetBaseRenderer();

  bool hide3Dvisualize = false;
  dataNode->GetBoolProperty("org.mitk.multilabel.3D.hide", hide3Dvisualize, baseRenderer);
  const bool smoothed = ResolveSmoothed(dataNode, baseRenderer);

  Vis3DMode currentMode = Vis3DMode::Off;
  if (!hide3Dvisualize)
  {
    currentMode = smoothed ? Vis3DMode::Smoothed : Vis3DMode::Exact;
  }

  this->menu()->clear();

  auto addItem = [this, currentMode](const QString& text, Vis3DMode mode)
  {
    auto* action = menu()->addAction(text);
    action->setCheckable(true);
    action->setChecked(currentMode == mode);
    action->setData(static_cast<int>(mode));
    connect(action, &QAction::triggered, this, &Qmitk3DMultiSegVisStyleAction::OnActionTriggered);
  };

  addItem(tr("Off"), Vis3DMode::Off);
  addItem(tr("Exact"), Vis3DMode::Exact);
  addItem(tr("Smoothed"), Vis3DMode::Smoothed);
}

void Qmitk3DMultiSegVisStyleAction::OnActionTriggered(bool /*checked*/)
{
  mitk::BaseRenderer::Pointer baseRenderer = GetBaseRenderer();

  auto selectedNodes = GetSelectedNodes();

  QAction* senderAction = qobject_cast<QAction*>(QObject::sender());
  if (nullptr == senderAction)
    return;

  const auto mode = static_cast<Vis3DMode>(senderAction->data().toInt());

  for (auto& dataNode : selectedNodes)
  {
    if (dataNode.IsNull())
    {
      continue;
    }

    switch (mode)
    {
      case Vis3DMode::Off:
        dataNode->SetBoolProperty("org.mitk.multilabel.3D.hide", true, baseRenderer);
        break;
      case Vis3DMode::Exact:
        dataNode->SetBoolProperty("org.mitk.multilabel.3D.hide", false, baseRenderer);
        dataNode->SetBoolProperty("org.mitk.multilabel.3D.smoothed", false, baseRenderer);
        break;
      case Vis3DMode::Smoothed:
        dataNode->SetBoolProperty("org.mitk.multilabel.3D.hide", false, baseRenderer);
        dataNode->SetBoolProperty("org.mitk.multilabel.3D.smoothed", true, baseRenderer);
        break;
    }
  }

  if (nullptr == baseRenderer)
  {
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  else
  {
    mitk::RenderingManager::GetInstance()->RequestUpdate(baseRenderer->GetRenderWindow());
  }

}
