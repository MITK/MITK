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

#include <QmitkDataNodeColorMapAction.h>

// mitk core
#include <mitkImage.h>
#include <mitkLookupTableProperty.h>
#include <mitkLookupTable.h>
#include <mitkRenderingManager.h>
#include <mitkRenderingModeProperty.h>

// mitk gui common plugin
#include <mitkDataNodeSelection.h>

// qt
#include <QMenu>

QmitkDataNodeColorMapAction::QmitkDataNodeColorMapAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchpartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(workbenchpartSite)
{
  setText(tr("Colormap"));
  InitializeAction();
}

QmitkDataNodeColorMapAction::QmitkDataNodeColorMapAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchpartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer(workbenchpartSite))
{
  setText(tr("Colormap"));
  InitializeAction();
}

void QmitkDataNodeColorMapAction::InitializeAction()
{
  setCheckable(true);

  setMenu(new QMenu);
  connect(menu(), &QMenu::aboutToShow, this, &QmitkDataNodeColorMapAction::OnMenuAboutShow);
}

void QmitkDataNodeColorMapAction::OnMenuAboutShow()
{
  auto dataNode = GetSelectedNode();
  if (dataNode.IsNull())
  {
    return;
  }

  mitk::BaseRenderer::Pointer baseRenderer = GetBaseRenderer();

  mitk::LookupTableProperty::Pointer lookupTableProperty = dynamic_cast<mitk::LookupTableProperty*>(dataNode->GetProperty("LookupTable", baseRenderer));
  if (lookupTableProperty.IsNull())
  {
    mitk::LookupTable::Pointer mitkLut = mitk::LookupTable::New();
    lookupTableProperty = mitk::LookupTableProperty::New();
    lookupTableProperty->SetLookupTable(mitkLut);
    dataNode->SetProperty("LookupTable", lookupTableProperty, baseRenderer);
  }

  mitk::LookupTable::Pointer lookupTable = lookupTableProperty->GetValue();
  if (lookupTable.IsNull())
  {
    return;
  }

  menu()->clear();
  QAction* lutAction;
  for (const auto& lutTypeString : lookupTable->typenameList)
  {
    lutAction = menu()->addAction(QString::fromStdString(lutTypeString));
    lutAction->setCheckable(true);

    if (lutTypeString == lookupTable->GetActiveTypeAsString())
    {
      lutAction->setChecked(true);
    }

    connect(lutAction, &QAction::triggered, this, &QmitkDataNodeColorMapAction::OnActionTriggered);
  }
}

void QmitkDataNodeColorMapAction::OnActionTriggered(bool /*checked*/)
{
  mitk::BaseRenderer::Pointer baseRenderer = GetBaseRenderer();

  auto selectedNodes = GetSelectedNodes();
  for (auto& dataNode : selectedNodes)
  {
    if (dataNode.IsNull())
    {
      continue;
    }
    mitk::LookupTableProperty::Pointer lookupTableProperty = dynamic_cast<mitk::LookupTableProperty*>(dataNode->GetProperty("LookupTable", baseRenderer));
    if (lookupTableProperty.IsNull())
    {
      continue;
    }

    mitk::LookupTable::Pointer lookupTable = lookupTableProperty->GetValue();
    if (lookupTable.IsNull())
    {
      continue;
    }

    mitk::LookupTable::Pointer renderWindowSpecificLutTab = lookupTable->Clone();

    QAction* senderAction = qobject_cast<QAction*>(QObject::sender());
    if (nullptr == senderAction)
    {
      continue;
    }

    // set lookup table type defined by the action string
    std::string activatedItem = senderAction->text().toStdString();
    renderWindowSpecificLutTab->SetType(activatedItem);
    dataNode->SetProperty("LookupTable", mitk::LookupTableProperty::New(renderWindowSpecificLutTab), baseRenderer);

    if (mitk::LookupTable::LookupTableType::MULTILABEL == lookupTable->GetActiveType())
    {
      // special case: multilabel => set the level window to include the whole pixel range
      UseWholePixelRange(dataNode);
    }

    mitk::RenderingModeProperty::Pointer renderingMode = dynamic_cast<mitk::RenderingModeProperty*>(dataNode->GetProperty("Image Rendering.Mode", baseRenderer));
    renderingMode->SetValue(mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR);

    if (nullptr == baseRenderer)
    {
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
    else
    {
      mitk::RenderingManager::GetInstance()->RequestUpdate(baseRenderer->GetRenderWindow());
    }
  }
}

void QmitkDataNodeColorMapAction::UseWholePixelRange(mitk::DataNode* node)
{
  auto image = dynamic_cast<mitk::Image*>(node->GetData());
  if (nullptr != image)
  {
    mitk::LevelWindow levelWindow;
    levelWindow.SetToImageRange(image);
    node->SetLevelWindow(levelWindow);
  }
}
