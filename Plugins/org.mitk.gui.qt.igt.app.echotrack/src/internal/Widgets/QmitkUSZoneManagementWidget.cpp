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
#include "QmitkUSZoneManagementWidget.h"
#include "ui_QmitkUSZoneManagementWidget.h"

#include "../QmitkUSZonesDataModel.h"
#include "../QmitkUSZoneManagementColorDialogDelegate.h"

#include "../Interactors/mitkUSZonesInteractor.h"
#include "usModuleRegistry.h"
#include "usModule.h"
//#include "mitkGlobalInteraction.h"
#include "mitkSurface.h"

#include <QLatin1Char>

QmitkUSZoneManagementWidget::QmitkUSZoneManagementWidget(QWidget *parent) :
QWidget(parent), m_ZonesDataModel(new QmitkUSZonesDataModel(this)),
m_Interactor(mitk::USZonesInteractor::New()),
m_StateMachineFileName("USZoneInteractions.xml"),
ui(new Ui::QmitkUSZoneManagementWidget), m_CurMaxNumOfZones(0)
{
  ui->setupUi(this);

  ui->CurrentZonesTable->setModel(m_ZonesDataModel);
  ui->CurrentZonesTable->setItemDelegateForColumn(2, new QmitkUSZoneManagementColorDialogDelegate(this));

  connect(ui->CurrentZonesTable->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
    this, SLOT(OnSelectionChanged(const QItemSelection&, const QItemSelection&)));
  connect(m_ZonesDataModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
    this, SLOT(OnRowInsertion(QModelIndex, int, int)));
  connect(m_ZonesDataModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
    this, SLOT(OnDataChanged(const QModelIndex&, const QModelIndex&)));

  // load state machine and event config for data interactor
  m_Interactor->LoadStateMachine(m_StateMachineFileName, us::ModuleRegistry::GetModule("MitkUS"));
  m_Interactor->SetEventConfig("globalConfig.xml");
}

QmitkUSZoneManagementWidget::~QmitkUSZoneManagementWidget()
{
  delete ui;

  if (m_DataStorage.IsNotNull() && m_BaseNode.IsNotNull()) { m_DataStorage->Remove(m_BaseNode); }
}

void QmitkUSZoneManagementWidget::SetStateMachineFilename(const std::string& filename)
{
  m_StateMachineFileName = filename;
  m_Interactor->LoadStateMachine(filename, us::ModuleRegistry::GetModule("MitkUS"));
}

void QmitkUSZoneManagementWidget::SetDataStorage(mitk::DataStorage::Pointer dataStorage, const char* baseNodeName)
{
  if (dataStorage.IsNull())
  {
    MITK_ERROR("QWidget")("QmitkUSZoneManagementWidget")
      << "DataStorage must not be null.";
    mitkThrow() << "DataStorage must not be null.";
  }

  mitk::DataNode::Pointer baseNode = dataStorage->GetNamedNode(baseNodeName);
  if (baseNode.IsNull())
  {
    baseNode = mitk::DataNode::New();
    baseNode->SetName(baseNodeName);
    dataStorage->Add(baseNode);
  }

  baseNode->SetData(mitk::Surface::New());

  m_ZonesDataModel->SetDataStorage(dataStorage, baseNode);

  m_BaseNode = baseNode;
  m_DataStorage = dataStorage;
}

void QmitkUSZoneManagementWidget::SetDataStorage(mitk::DataStorage::Pointer dataStorage, mitk::DataNode::Pointer baseNode)
{
  if (dataStorage.IsNull() || baseNode.IsNull())
  {
    MITK_ERROR("QWidget")("QmitkUSZoneManagementWidget")
      << "DataStorage and BaseNode must not be null.";
    mitkThrow() << "DataStorage and BaseNode must not be null.";
  }

  if (!baseNode->GetData()) { baseNode->SetData(mitk::Surface::New()); }

  m_ZonesDataModel->SetDataStorage(dataStorage, baseNode);

  m_BaseNode = baseNode;
  m_DataStorage = dataStorage;
}

mitk::DataStorage::SetOfObjects::ConstPointer QmitkUSZoneManagementWidget::GetZoneNodes()
{
  if (m_DataStorage.IsNotNull() && m_BaseNode.IsNotNull())
  {
    return m_DataStorage->GetDerivations(m_BaseNode);
  }
  else
  {
    MITK_WARN("QWidget")("QmitkUSZoneManagementWidget")
      << "Data storage or base node is null. Returning empty zone nodes set.";
    return mitk::DataStorage::SetOfObjects::New().GetPointer();
  }
}

void QmitkUSZoneManagementWidget::RemoveSelectedRows()
{
  QItemSelectionModel* selectionModel = ui->CurrentZonesTable->selectionModel();
  if (!selectionModel->hasSelection())
  {
    MITK_WARN("QWidget")("QmitkUSZoneManagementWidget")
      << "RemoveSelectedRows() called without any row being selected.";
    return;
  }

  QModelIndexList selectedRows = selectionModel->selectedRows();

  // sorted indices are assumed
  QListIterator<QModelIndex> i(selectedRows);
  i.toBack();
  while (i.hasPrevious())
  {
    m_ZonesDataModel->removeRow(i.previous().row());
  }

  emit ZoneRemoved();
}

void QmitkUSZoneManagementWidget::OnStartAddingZone()
{
  if (m_DataStorage.IsNull())
  {
    MITK_ERROR("QWidget")("QmitkUSZoneManagementWidget")
      << "DataStorage must be set before adding the first zone.";
    mitkThrow() << "DataStorage must be set before adding the first zone.";
  }

  // workaround for bug 16407
  m_Interactor = mitk::USZonesInteractor::New();
  m_Interactor->LoadStateMachine(m_StateMachineFileName, us::ModuleRegistry::GetModule("MitkUS"));
  m_Interactor->SetEventConfig("globalConfig.xml");

  mitk::DataNode::Pointer dataNode = mitk::DataNode::New();
  // zone number is added to zone name (padding one zero)
  dataNode->SetName((QString("Zone ") + QString("%1").arg(m_CurMaxNumOfZones + 1, 2, 10, QLatin1Char('0'))).toStdString());
  dataNode->SetColor(0.9, 0.9, 0);
  m_DataStorage->Add(dataNode, m_BaseNode);
  m_Interactor->SetDataNode(dataNode);
}

void QmitkUSZoneManagementWidget::OnAbortAddingZone()
{
  if (m_DataStorage.IsNull())
  {
    MITK_ERROR("QWidget")("QmitkUSZoneManagementWidget")
      << "DataStorage must be set before aborting adding a zone.";
    mitkThrow() << "DataStorage must be set before aborting adding a zone.";
  }

  m_DataStorage->Remove(m_Interactor->GetDataNode());
}

void QmitkUSZoneManagementWidget::OnResetZones()
{
  // remove all zone nodes from the data storage
  if (m_DataStorage.IsNotNull() && m_BaseNode.IsNotNull())
  {
    m_DataStorage->Remove(m_DataStorage->GetDerivations(m_BaseNode));
  }

  m_CurMaxNumOfZones = 0;
}

void QmitkUSZoneManagementWidget::OnSelectionChanged(const QItemSelection & selected, const QItemSelection & /*deselected*/)
{
  bool somethingSelected = !selected.empty() && m_ZonesDataModel->rowCount() > 0;
  ui->ZoneSizeLabel->setEnabled(somethingSelected);
  ui->ZoneSizeSlider->setEnabled(somethingSelected);
  ui->DeleteZoneButton->setEnabled(somethingSelected);

  if (somethingSelected)
  {
    ui->ZoneSizeSlider->setValue(
      m_ZonesDataModel->data(m_ZonesDataModel->index(selected.at(0).top(), 1)).toInt());
  }
}

void QmitkUSZoneManagementWidget::OnZoneSizeSliderValueChanged(int value)
{
  QItemSelectionModel* selection = ui->CurrentZonesTable->selectionModel();
  if (!selection->hasSelection()) { return; }

  m_ZonesDataModel->setData(m_ZonesDataModel->index(selection->selectedRows().at(0).row(), 1), value);
}

void QmitkUSZoneManagementWidget::OnRowInsertion(const QModelIndex & /*parent*/, int /*start*/, int end)
{
  // increase zone number for unique names for every zone
  m_CurMaxNumOfZones++;

  ui->CurrentZonesTable->selectRow(end);
  emit ZoneAdded();
}

void QmitkUSZoneManagementWidget::OnDataChanged(const QModelIndex& topLeft, const QModelIndex& /*bottomRight*/)
{
  QItemSelectionModel* selection = ui->CurrentZonesTable->selectionModel();
  if (!selection->hasSelection() || selection->selectedRows().size() < 1) { return; }

  if (selection->selectedRows().at(0) == topLeft)
  {
    ui->ZoneSizeSlider->setValue(
      m_ZonesDataModel->data(m_ZonesDataModel->index(topLeft.row(), 1)).toInt());
  }
}
