/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryPerspectivesPreferencePage.h"
#include "ui_berryPerspectivesPreferencePage.h"

#include <berryIPreferencesService.h>
#include <berryIPreferences.h>
#include <berryIWorkbenchPage.h>

#include "internal/berryPerspective.h"
#include "internal/berryPerspectiveRegistry.h"
#include "internal/berryPreferenceConstants.h"
#include "internal/berryWorkbenchPage.h"
#include "berryWorkbenchPlugin.h"

#include <QListWidgetItem>
#include <QMessageBox>

namespace berry {

bool PerspectiveComparator(const PerspectiveDescriptor::Pointer& p1, const PerspectiveDescriptor::Pointer& p2)
{
  return p1->GetLabel() < p2->GetLabel();
}

PerspectivesPreferencePage::PerspectivesPreferencePage()
  : ui(nullptr)
  , pageWidget(nullptr)
  , workbench(nullptr)
  , perspRegistry(nullptr)
{
}

PerspectivesPreferencePage::~PerspectivesPreferencePage()
{
  delete ui;
}


void PerspectivesPreferencePage::Init(berry::IWorkbench::Pointer workbench)
{
  ui = new Ui::PerspectivesPreferencePage;

  this->workbench = workbench.GetPointer();
  perspRegistry = dynamic_cast<PerspectiveRegistry*>(workbench->GetPerspectiveRegistry());
}

void PerspectivesPreferencePage::CreateQtControl(QWidget* parent)
{
  pageWidget = new QWidget(parent);
  ui->setupUi(pageWidget);

  ui->perspectivesListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  ui->perspectivesListWidget->setIconSize(QSize(16, 16));

  connect(ui->sameWindowButton, SIGNAL(clicked()), this, SLOT(OpenPerspInSameWindow()));
  connect(ui->newWindowButton, SIGNAL(clicked()), this, SLOT(OpenPerspInNewWindow()));
  connect(ui->perspectivesListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(PerspectiveSelectionChanged()));

  connect(ui->revertButton, SIGNAL(clicked()), this, SLOT(RevertPerspective()));
  connect(ui->makeDefaultButton, SIGNAL(clicked()), this, SLOT(MakeDefaultPerspective()));
  connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(DeletePerspective()));

  this->Update();
}

QWidget* PerspectivesPreferencePage::GetQtControl() const
{
  return pageWidget;
}

bool PerspectivesPreferencePage::PerformOk()
{
  // Set the default perspective
  if (defaultPerspectiveId != perspRegistry->GetDefaultPerspective())
  {
    perspRegistry->SetDefaultPerspective(defaultPerspectiveId);
  }

  //Delete the perspective
  if(perspectives.size() < perspRegistry->GetPerspectives().size())
  {
    QList<IWorkbenchWindow::Pointer> windows = workbench->GetWorkbenchWindows();

    // close any perspectives that are about to be deleted
    for (int i = 0; i < windows.size(); i++)
    {
      QList<IWorkbenchPage::Pointer> pages = windows[i]->GetPages();
      for (int j = 0; j < pages.size(); j++)
      {
        WorkbenchPage::Pointer page = pages[j].Cast<WorkbenchPage>();
        for (int k = 0; k < perspToDelete.size(); k++)
        {
          IPerspectiveDescriptor::Pointer desc(perspToDelete[k].GetPointer());
          if (page->FindPerspective(desc).IsNotNull())
          {
            page->ClosePerspective(desc, true, true);
          }
        }
      }
    }
    perspRegistry->DeletePerspectives(perspToDelete);
  }

  // Revert the perspectives
  perspRegistry->RevertPerspectives(perspToRevert);

  // store the open perspective mode setting
  preferences->PutInt(PreferenceConstants::OPEN_PERSP_MODE, openPerspMode);

  return true;
}

void PerspectivesPreferencePage::PerformCancel()
{

}

void PerspectivesPreferencePage::Update()
{
  preferences = WorkbenchPlugin::GetDefault()->GetPreferences();

  openPerspMode = preferences->GetInt(PreferenceConstants::OPEN_PERSP_MODE, PreferenceConstants::OPM_ACTIVE_PAGE);

  ui->sameWindowButton->setChecked(openPerspMode == PreferenceConstants::OPM_ACTIVE_PAGE);
  ui->newWindowButton->setChecked(openPerspMode == PreferenceConstants::OPM_NEW_WINDOW);

  // Populate the perspectivesTable
  perspectives.clear();
  perspToRevert.clear();
  perspToDelete.clear();
  QList<IPerspectiveDescriptor::Pointer> persps = perspRegistry->GetPerspectives();
  for (int i = 0; i < persps.size(); i++)
  {
    perspectives.push_back(persps[i].Cast<PerspectiveDescriptor>());
  }
  qSort(perspectives.begin(), perspectives.end(), PerspectiveComparator);
  defaultPerspectiveId = perspRegistry->GetDefaultPerspective();
  UpdatePerspectivesTable();
}

void PerspectivesPreferencePage::OpenPerspInSameWindow()
{
  openPerspMode = PreferenceConstants::OPM_ACTIVE_PAGE;
}

void PerspectivesPreferencePage::OpenPerspInNewWindow()
{
  openPerspMode = PreferenceConstants::OPM_NEW_WINDOW;
}

void PerspectivesPreferencePage::PerspectiveSelectionChanged()
{
  UpdateButtons();
}

void PerspectivesPreferencePage::RevertPerspective()
{
  PerspectiveDescriptor::Pointer desc = GetSelectedPerspective();
  if (desc.IsNotNull() && !perspToRevert.contains(desc))
  {
    perspToRevert.push_back(desc);
  }
  UpdateButtons();
}

void PerspectivesPreferencePage::DeletePerspective()
{
  PerspectiveDescriptor::Pointer desc = GetSelectedPerspective();
  if (desc.IsNotNull() && !perspToDelete.contains(desc))
  {
    if (!FindOpenInstance(desc))
    {
      perspToDelete.push_back(desc);
      perspToRevert.removeAll(desc);
      perspectives.removeAll(desc);
      UpdatePerspectivesTable();
    }
  }
  UpdateButtons();
}

void PerspectivesPreferencePage::MakeDefaultPerspective()
{
  PerspectiveDescriptor::Pointer desc = GetSelectedPerspective();
  if (desc.IsNotNull() && !perspToDelete.contains(desc))
  {
    int row = perspectives.indexOf(desc);
    defaultPerspectiveId = desc->GetId();
    UpdatePerspectivesTable();
    ui->perspectivesListWidget->item(row)->setSelected(true);
  }
  UpdateButtons();
}

void PerspectivesPreferencePage::UpdateButtons()
{
  PerspectiveDescriptor::Pointer desc = GetSelectedPerspective();
  if (desc)
  {
    ui->revertButton->setEnabled(desc->IsPredefined() && desc->HasCustomDefinition() && !perspToRevert.contains(desc));
    ui->deleteButton->setEnabled(!desc->IsPredefined());
    ui->makeDefaultButton->setEnabled(true);
  }
  else
  {
    ui->revertButton->setEnabled(false);
    ui->deleteButton->setEnabled(false);
    ui->makeDefaultButton->setEnabled(false);
  }
}

void PerspectivesPreferencePage::UpdatePerspectivesTable()
{
  ui->perspectivesListWidget->clear();
  for (PerspectiveDescriptor::Pointer desc : perspectives)
  {
    NewPerspectivesTableItem(desc);
  }
}

void PerspectivesPreferencePage::NewPerspectivesTableItem(const SmartPointer<PerspectiveDescriptor>& desc)
{
  QString label = desc->GetLabel();
  if (desc->GetId() == defaultPerspectiveId)
  {
    label += " (default)";
  }
  new QListWidgetItem(desc->GetImageDescriptor(), label, ui->perspectivesListWidget);
}

bool PerspectivesPreferencePage::FindOpenInstance(const PerspectiveDescriptor::Pointer& desc)
{
  QList<IWorkbenchWindow::Pointer> windows = workbench->GetWorkbenchWindows();

  //find all active perspectives currently
  for (int i = 0; i < windows.size(); i++)
  {
    QList<IWorkbenchPage::Pointer> pages = windows[i]->GetPages();
    for (int j = 0; j < pages.size(); j++)
    {
      WorkbenchPage::Pointer page = pages[j].Cast<WorkbenchPage>();
      if (page->FindPerspective(desc).IsNotNull())
      {
        QMessageBox::StandardButton returnCode =
            QMessageBox::question(workbench->GetActiveWorkbenchWindow()->GetShell()->GetControl(),
                                  "Delete Perspective",
                                  QString("Are you sure you want to delete the \"%1\" perspective? It has open instances.").arg(desc->GetLabel()),
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        return (returnCode != QMessageBox::Yes);
      }
    }
  }

  return false;
}

SmartPointer<PerspectiveDescriptor> PerspectivesPreferencePage::GetSelectedPerspective() const
{
  PerspectiveDescriptor::Pointer desc;

  QList<QListWidgetItem*> selection = ui->perspectivesListWidget->selectedItems();
  if (!selection.isEmpty())
  {
    int row = ui->perspectivesListWidget->row(selection.back());
    if (row > -1)
    {
      desc = perspectives.at(row);
    }
  }
  return desc;
}

}
