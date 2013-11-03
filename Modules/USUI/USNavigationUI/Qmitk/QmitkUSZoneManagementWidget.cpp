#include "QmitkUSZoneManagementWidget.h"
#include "ui_QmitkUSZoneManagementWidget.h"

#include "QmitkUSZonesDataModel.h"
#include "QmitkUSZoneManagementSliderDelegate.h"
#include "QmitkUSZoneManagementComboBoxDelegate.h"

#include "mitkUSZonesInteractor.h"
#include "usModuleRegistry.h"
#include "usModule.h"
#include "mitkGlobalInteraction.h"

QmitkUSZoneManagementWidget::QmitkUSZoneManagementWidget(QWidget *parent) :
    QWidget(parent), m_ZonesDataModel(new QmitkUSZonesDataModel(this)),
    ui(new Ui::QmitkUSZoneManagementWidget)
{
  ui->setupUi(this);
  ui->CurrentZonesTable->setModel(m_ZonesDataModel);

  ui->CurrentZonesTable->setItemDelegateForColumn(1, new QmitkUSZoneManagementSliderDelegate(this));
  ui->CurrentZonesTable->setItemDelegateForColumn(2, new QmitkUSZoneManagementComboBoxDelegate(this));

  connect (ui->CurrentZonesTable->selectionModel(), SIGNAL(selectionChanged(const QItemSelection& , const QItemSelection&)),
           this, SLOT(OnSelectionChanged(const QItemSelection&, const QItemSelection&)));
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

  m_Interactor = mitk::USZonesInteractor::New(dataStorage, baseNode);
  m_Interactor->LoadStateMachine("USZoneInteractions.xml", us::ModuleRegistry::GetModule("MitkUSNavigation"));
  m_Interactor->SetDataNode(baseNode);


  //  mitk::BaseRenderer::GetInstance()
}

void QmitkUSZoneManagementWidget::AddRow()
{
  m_ZonesDataModel->insertRow(m_ZonesDataModel->rowCount());
  m_ZonesDataModel->setData(m_ZonesDataModel->index(m_ZonesDataModel->rowCount()-1,1), 5);
  m_ZonesDataModel->setData(m_ZonesDataModel->index(m_ZonesDataModel->rowCount()-1,2), "Red");

}

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

void QmitkUSZoneManagementWidget::OnSelectionChanged(const QItemSelection & selected, const QItemSelection & /*deselected*/)
{
  ui->DeleteZoneButton->setDisabled(selected.empty());
}
