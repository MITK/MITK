#include "QmitkUSZoneManagementWidget.h"
#include "ui_QmitkUSZoneManagementWidget.h"

#include "QmitkUSZonesDataModel.h"
#include "QmitkUSZoneManagementSpinBoxDelegate.h"
#include "QmitkUSZoneManagementComboBoxDelegate.h"

#include "mitkUSZonesInteractor.h"
#include "usModuleRegistry.h"
#include "usModule.h"
#include "mitkGlobalInteraction.h"

QmitkUSZoneManagementWidget::QmitkUSZoneManagementWidget(QWidget *parent) :
    QWidget(parent), m_ZonesDataModel(new QmitkUSZonesDataModel(this)),
    m_SelectedRow(-1), ui(new Ui::QmitkUSZoneManagementWidget)
{
  ui->setupUi(this);
  ui->CurrentZonesTable->setModel(m_ZonesDataModel);

  ui->CurrentZonesTable->setItemDelegateForColumn(1, new QmitkUSZoneManagementSpinBoxDelegate(this));
  ui->CurrentZonesTable->setItemDelegateForColumn(2, new QmitkUSZoneManagementComboBoxDelegate(this));

  connect (ui->CurrentZonesTable->selectionModel(), SIGNAL(selectionChanged(const QItemSelection& , const QItemSelection&)),
           this, SLOT(OnSelectionChanged(const QItemSelection&, const QItemSelection&)));
  connect (m_ZonesDataModel, SIGNAL(rowsInserted(const QModelIndex&, int, int )),
           this, SLOT(OnRowInsertion(QModelIndex,int,int)));
}

QmitkUSZoneManagementWidget::~QmitkUSZoneManagementWidget()
{
  delete ui;
}

void QmitkUSZoneManagementWidget::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  mitk::DataNode::Pointer baseNode = mitk::DataNode::New();
  baseNode->SetName("Zones");
  baseNode->SetData(mitk::Surface::New());
  dataStorage->Add(baseNode);

  m_ZonesDataModel->SetDataStorage(dataStorage, baseNode);

  m_BaseNode = baseNode;
  m_DataStorage = dataStorage;
}

mitk::DataStorage::SetOfObjects::ConstPointer QmitkUSZoneManagementWidget::GetZoneNodes()
{
  return m_DataStorage->GetDerivations(m_BaseNode);
}

/*void QmitkUSZoneManagementWidget::AddRow()
{
  m_ZonesDataModel->insertRow(m_ZonesDataModel->rowCount());
  m_ZonesDataModel->setData(m_ZonesDataModel->index(m_ZonesDataModel->rowCount()-1,1), 5);
  m_ZonesDataModel->setData(m_ZonesDataModel->index(m_ZonesDataModel->rowCount()-1,2), "Red");
}*/

void QmitkUSZoneManagementWidget::RemoveSelectedRows()
{
  QItemSelectionModel* selectionModel = ui->CurrentZonesTable->selectionModel();
  if ( ! selectionModel->hasSelection() )
  {
    MITK_WARN("QmitkUSZoneManagementWidget")
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
}

void QmitkUSZoneManagementWidget::OnStartAddingZone()
{
  m_Interactor = mitk::USZonesInteractor::New();
  m_Interactor->LoadStateMachine("USZoneInteractions.xml", us::ModuleRegistry::GetModule("MitkUSNavigation"));
  m_Interactor->SetEventConfig("globalConfig.xml");

  mitk::DataNode::Pointer dataNode = mitk::DataNode::New();
  dataNode->SetName("Zone");
  dataNode->SetColor(1, 0, 0);
  m_DataStorage->Add(dataNode, m_BaseNode);
  m_Interactor->SetDataNode(dataNode);
}

void QmitkUSZoneManagementWidget::OnResetZones()
{
  // remove all zone nodes from the data storage
  m_DataStorage->Remove(m_DataStorage->GetDerivations(m_BaseNode));
}

void QmitkUSZoneManagementWidget::OnSelectionChanged(const QItemSelection & selected, const QItemSelection & /*deselected*/)
{
  bool somethingSelected = ! selected.empty();
  ui->ZoneSizeLabel->setEnabled(somethingSelected);
  ui->ZoneSizeSlider->setEnabled(somethingSelected);
  ui->DeleteZoneButton->setEnabled(somethingSelected);

  if (somethingSelected)
  {
    m_SelectedRow = selected.at(0).top();
    ui->ZoneSizeSlider->setValue(
          m_ZonesDataModel->data(m_ZonesDataModel->index(m_SelectedRow, 1)).toInt());

  }
  else
  {
    m_SelectedRow = -1;
  }
}

void QmitkUSZoneManagementWidget::OnZoneSizeSliderValueChanged(int value)
{
  if (m_SelectedRow < 0) { return; }

  m_ZonesDataModel->setData(m_ZonesDataModel->index(m_SelectedRow, 1), value);
}

void QmitkUSZoneManagementWidget::OnRowInsertion( const QModelIndex & /*parent*/, int /*start*/, int end )
{
  //ui->CurrentZonesTable->selectRow(end);
}
