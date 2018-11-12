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
#include <mitkDataNode.h>
#include <mitkLookupTableProperty.h>
#include <mitkLookupTable.h>
#include <mitkRenderingManager.h>
#include <mitkRenderingModeProperty.h>

// mitk gui common plugin
#include <mitkDataNodeSelection.h>

// berry
#include <berryISelectionService.h>

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

QmitkDataNodeColorMapAction::~QmitkDataNodeColorMapAction()
{
  // nothing here
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

  mitk::LookupTableProperty::Pointer lookupTableProperty = dynamic_cast<mitk::LookupTableProperty*>(dataNode->GetProperty("LookupTable"));
  if (lookupTableProperty.IsNull())
  {
    mitk::LookupTable::Pointer mitkLut = mitk::LookupTable::New();
    lookupTableProperty = mitk::LookupTableProperty::New();
    lookupTableProperty->SetLookupTable(mitkLut);
    dataNode->SetProperty("LookupTable", lookupTableProperty);
  }

  mitk::LookupTable::Pointer lookupTable = lookupTableProperty->GetValue();
  if (lookupTable.IsNull())
  {
    return;
  }

  menu()->clear();
  QAction* tmp;

  int i = 0;
  std::string lutType = lookupTable->typenameList[i];

  while (lutType != "END_OF_ARRAY")
  {
    tmp = menu()->addAction(QString::fromStdString(lutType));
    tmp->setCheckable(true);

    if (lutType == lookupTable->GetActiveTypeAsString())
    {
      tmp->setChecked(true);
    }

    connect(tmp, &QAction::triggered, this, &QmitkDataNodeColorMapAction::OnActionTriggered);

    lutType = lookupTable->typenameList[++i];
  }
}

void QmitkDataNodeColorMapAction::OnActionTriggered(bool checked)
{
  auto dataNode = GetSelectedNode();
  if (dataNode.IsNull())
  {
    return;
  }

  mitk::LookupTableProperty::Pointer lookupTableProperty = dynamic_cast<mitk::LookupTableProperty*>(dataNode->GetProperty("LookupTable"));
  if (lookupTableProperty.IsNull())
  {
    return;
  }

  mitk::LookupTable::Pointer lookupTable = lookupTableProperty->GetValue();
  if (lookupTable.IsNull())
  {
    return;
  }

  QAction* senderAction = qobject_cast<QAction*>(QObject::sender());
  if (nullptr == senderAction)
  {
    return;
  }

  std::string activatedItem = senderAction->text().toStdString();
  lookupTable->SetType(activatedItem);
  lookupTableProperty->SetValue(lookupTable);
  mitk::RenderingModeProperty::Pointer renderingMode = dynamic_cast<mitk::RenderingModeProperty*>(dataNode->GetProperty("Image Rendering.Mode"));
  renderingMode->SetValue(mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
