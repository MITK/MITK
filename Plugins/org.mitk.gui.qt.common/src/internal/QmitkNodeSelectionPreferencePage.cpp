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


#include "QmitkNodeSelectionPreferencePage.h"

#include "QmitkNodeSelectionPreferenceHelper.h"

//-----------------------------------------------------------------------------
QmitkNodeSelectionPreferencePage::QmitkNodeSelectionPreferencePage()
  : m_MainControl(0), m_Controls(0)
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

  connect(m_Controls->comboFavorite, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateWidgets()));
  connect(m_Controls->btnUp, SIGNAL(clicked(bool)), this, SLOT(MoveUp()));
  connect(m_Controls->btnDown, SIGNAL(clicked(bool)), this, SLOT(MoveDown()));
  connect(m_Controls->listInspectors, SIGNAL(itemSelectionChanged()), this, SLOT(UpdateWidgets()));

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
  //store favorite
  auto id = m_Controls->comboFavorite->currentData().toString();
  mitk::PutFavoriteDataStorageInspector(id.toStdString());

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
  auto favorite = mitk::GetFavoriteDataStorageInspector();

  auto finding = m_Providers.find(favorite);
  if (finding == m_Providers.cend())
  {
    favorite = m_Providers.begin()->first;
  }

  //fill favorite combo
  int index = 0;
  int currentIndex = 0;
  m_Controls->comboFavorite->clear();
  for (auto iter : m_Providers)
  {
    m_Controls->comboFavorite->addItem(QString::fromStdString(iter.second->GetInspectorDisplayName()),QVariant::fromValue(QString::fromStdString(iter.first)));
    if (iter.first == favorite)
    {
      currentIndex = index;
    };
    ++index;
  }
  m_Controls->comboFavorite->setCurrentIndex(currentIndex);

  //fill inspector list
  m_Controls->listInspectors->clear();
  for (const auto iter : allProviders)
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
    if (item->data(Qt::UserRole).toString() == m_Controls->comboFavorite->currentData().toString())
    {
      //favorites are always visible.
      item->setCheckState(Qt::Checked);
      item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
    }
    else
    {
      item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsUserCheckable);
    }
  }
};

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
};

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
};
