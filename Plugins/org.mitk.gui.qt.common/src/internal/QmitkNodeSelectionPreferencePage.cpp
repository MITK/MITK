/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "QmitkNodeSelectionPreferencePage.h"

#include "QmitkNodeSelectionPreferenceHelper.h"

#include <QmitkDataStorageSelectionHistoryInspector.h>
#include <QmitkDataStorageFavoriteNodesInspector.h>

//-----------------------------------------------------------------------------
QmitkNodeSelectionPreferencePage::QmitkNodeSelectionPreferencePage()
  : m_MainControl(nullptr), m_Controls(nullptr)
{

}


//-----------------------------------------------------------------------------
QmitkNodeSelectionPreferencePage::~QmitkNodeSelectionPreferencePage()
{
  delete m_Controls;
}


//-----------------------------------------------------------------------------
void QmitkNodeSelectionPreferencePage::Init(berry::IWorkbench::Pointer )
{

}


//-----------------------------------------------------------------------------
void QmitkNodeSelectionPreferencePage::CreateQtControl(QWidget* parent)
{
  m_MainControl = new QWidget(parent);
  m_Controls = new Ui::QmitkNodeSelectionPreferencePage;
  m_Controls->setupUi( m_MainControl );

  connect(m_Controls->comboPreferred, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateWidgets()));
  connect(m_Controls->btnUp, SIGNAL(clicked(bool)), this, SLOT(MoveUp()));
  connect(m_Controls->btnDown, SIGNAL(clicked(bool)), this, SLOT(MoveDown()));
  connect(m_Controls->listInspectors, &QListWidget::itemSelectionChanged, this, &QmitkNodeSelectionPreferencePage::UpdateWidgets);

  this->Update();
}


//-----------------------------------------------------------------------------
QWidget* QmitkNodeSelectionPreferencePage::GetQtControl() const
{
  return m_MainControl;
}

//-----------------------------------------------------------------------------
bool QmitkNodeSelectionPreferencePage::PerformOk()
{
  //store preferred
  auto id = m_Controls->comboPreferred->currentData().toString();
  mitk::PutPreferredDataStorageInspector(id.toStdString());

  //store visible
  mitk::VisibleDataStorageInspectorMapType visibles;

  unsigned int visiblePos = 0;

  for (int i = 0; i < m_Controls->listInspectors->count(); ++i)
  {
    auto item = m_Controls->listInspectors->item(i);
    if (item->checkState() == Qt::Checked)
    {
      visibles.insert(std::make_pair(visiblePos++, item->data(Qt::UserRole).toString().toStdString()));
    }
  }
  mitk::PutVisibleDataStorageInspectors(visibles);
  mitk::PutShowFavoritesInspector(m_Controls->checkShowFav->isChecked());
  mitk::PutShowHistoryInspector(m_Controls->checkShowHistory->isChecked());

  return true;
}


//-----------------------------------------------------------------------------
void QmitkNodeSelectionPreferencePage::PerformCancel()
{
}


//-----------------------------------------------------------------------------
void QmitkNodeSelectionPreferencePage::Update()
{
  m_Providers = mitk::DataStorageInspectorGenerator::GetProviders();

  auto visibleProviders = mitk::GetVisibleDataStorageInspectors();
  auto allProviders = mitk::DataStorageInspectorGenerator::GetProviders();
  auto preferredInspectorID = mitk::GetPreferredDataStorageInspector();

  //fill preferred combo
  int index = 0;
  int currentIndex = 0;
  m_Controls->comboPreferred->clear();
  for (const auto &iter : m_Providers)
  {
    m_Controls->comboPreferred->addItem(QString::fromStdString(iter.second->GetInspectorDisplayName()), QVariant::fromValue(QString::fromStdString(iter.first)));
    if (iter.first == preferredInspectorID)
    {
      currentIndex = index;
    };
    ++index;
  }
  m_Controls->comboPreferred->setCurrentIndex(currentIndex);

  //fill inspector list
  m_Controls->listInspectors->clear();
  for (const auto& iter : allProviders)
  {
    if (iter.first != QmitkDataStorageFavoriteNodesInspector::INSPECTOR_ID() && iter.first != QmitkDataStorageSelectionHistoryInspector::INSPECTOR_ID())
    {
      auto currentID = iter.first;
      QListWidgetItem* item = new QListWidgetItem;
      item->setText(QString::fromStdString(iter.second->GetInspectorDisplayName()));
      item->setData(Qt::UserRole, QVariant::fromValue(QString::fromStdString(currentID)));
      item->setToolTip(QString::fromStdString(iter.second->GetInspectorDescription()));

      auto finding = std::find_if(visibleProviders.cbegin(), visibleProviders.cend(), [&currentID](auto v) {return v.second == currentID; });
      if (finding == visibleProviders.cend())
      {
        item->setCheckState(Qt::Unchecked);
        m_Controls->listInspectors->addItem(item);
      }
      else
      {
        item->setCheckState(Qt::Checked);
        m_Controls->listInspectors->insertItem(finding->first, item);
      }
    }
  }

  m_Controls->checkShowFav->setChecked(mitk::GetShowFavoritesInspector());
  m_Controls->checkShowHistory->setChecked(mitk::GetShowHistoryInspector());

  this->UpdateWidgets();
}

void QmitkNodeSelectionPreferencePage::UpdateWidgets()
{
  int currentIndex = m_Controls->listInspectors->currentRow();
  m_Controls->btnUp->setEnabled(!m_Controls->listInspectors->selectedItems().empty() && currentIndex > 0);
  m_Controls->btnDown->setEnabled(!m_Controls->listInspectors->selectedItems().empty() && currentIndex + 1 < m_Controls->listInspectors->count());

  for (int i = 0; i < m_Controls->listInspectors->count(); ++i)
  {
    auto item = m_Controls->listInspectors->item(i);
    if (item->data(Qt::UserRole).toString() == m_Controls->comboPreferred->currentData().toString())
    {
      //preferred inspector is always visible.
      item->setCheckState(Qt::Checked);
      item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
    }
    else
    {
      item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsUserCheckable);
    }
  }

  auto isFavSelected =
    QmitkDataStorageFavoriteNodesInspector::INSPECTOR_ID() == m_Controls->comboPreferred->currentData().toString();
  if (isFavSelected)
  {
    m_Controls->checkShowFav->setChecked(true);
  }
  m_Controls->checkShowFav->setEnabled(!isFavSelected);

  auto isHistorySelected =
    QmitkDataStorageSelectionHistoryInspector::INSPECTOR_ID() == m_Controls->comboPreferred->currentData().toString();
  if (isHistorySelected)
  {
    m_Controls->checkShowHistory->setChecked(true);
  }
  m_Controls->checkShowHistory->setEnabled(!isHistorySelected);
}

void QmitkNodeSelectionPreferencePage::MoveDown()
{
  int currentIndex = m_Controls->listInspectors->currentRow();
  if (currentIndex+1 < m_Controls->listInspectors->count())
  {
    QListWidgetItem *currentItem = m_Controls->listInspectors->takeItem(currentIndex);
    m_Controls->listInspectors->insertItem(currentIndex + 1, currentItem);
    m_Controls->listInspectors->setCurrentRow(currentIndex + 1);
  }
  this->UpdateWidgets();
}

void QmitkNodeSelectionPreferencePage::MoveUp()
{
  int currentIndex = m_Controls->listInspectors->currentRow();
  if (currentIndex > 0)
  {
    QListWidgetItem *currentItem = m_Controls->listInspectors->takeItem(currentIndex);
    m_Controls->listInspectors->insertItem(currentIndex - 1, currentItem);
    m_Controls->listInspectors->setCurrentRow(currentIndex - 1);
  }
  this->UpdateWidgets();
}
