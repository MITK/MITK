/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkConfigPreferencePage.h"
#include "ui_QmitkConfigPreferencePage.h"

#include <vector>

#include <berryIWorkbenchPage.h>

#include <ConfigManager.h>

namespace
{
  struct ConfigSaver : config::IParamSaver
  {
    explicit ConfigSaver(Ui::QmitkConfigPreferencePage* ui, const std::unordered_set<std::string>& deletedParams)
      : ui(ui)
      , m_deletedParams(deletedParams)
    {
      ui->configTableWidget->setColumnCount(2);
      ui->configTableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(QObject::tr("key")));
      ui->configTableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(QObject::tr("value")));
    }

    virtual ~ConfigSaver()
    {
      m_params.erase(std::remove_if(m_params.begin(), m_params.end(), [this](const std::pair<QString, QString>& item) -> bool {
        return !item.first.indexOf(INTERNAL_PARAM_PREFIX) || m_deletedParams.end() != m_deletedParams.find(item.first.toStdString());
      }), m_params.end());
      ui->configTableWidget->setRowCount(m_params.size());
      for (auto i = 0; i < m_params.size(); ++i) {
        set(i, 0, m_params[i].first);
        set(i, 1, m_params[i].second);
      }
      ui->deleteButton->setEnabled(!m_params.empty());
    }

    virtual void Put(const char* key, const QString& value)
    {
      m_params.push_back(std::make_pair(key, value));
    }

    virtual void Put(const char* key, double value)
    {
      m_params.push_back(std::make_pair(key, QString::number(value)));
    }

    virtual void Put(const char* key, int value)
    {
      m_params.push_back(std::make_pair(key, QString::number(value)));
    }

    virtual void Put(const char* key, bool value)
    {
      m_params.push_back(std::make_pair(key, value ? QObject::tr("true") : QObject::tr("false")));
    }

  private:
    void set(int row, int column, const QString& text)
    {
      QTableWidgetItem* item = new QTableWidgetItem;
      item->setText(text);
      ui->configTableWidget->setItem(row, column, item);
    }

    Ui::QmitkConfigPreferencePage* ui;
    const std::unordered_set<std::string>& m_deletedParams;

    std::vector<std::pair<QString, QString>> m_params;
  };
}

QmitkConfigPreferencePage::QmitkConfigPreferencePage()
  : ui(nullptr)
  , pageWidget(nullptr)
  , workbench(nullptr)
{
}

QmitkConfigPreferencePage::~QmitkConfigPreferencePage()
{
}

void QmitkConfigPreferencePage::Init(berry::IWorkbench::Pointer workbench)
{
  ui.reset(new Ui::QmitkConfigPreferencePage);

  this->workbench = workbench.GetPointer();
}

void QmitkConfigPreferencePage::CreateQtControl(QWidget* parent)
{
  pageWidget = new QWidget(parent);
  ui->setupUi(pageWidget);
  
  ui->configTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  ui->configTableWidget->setIconSize(QSize(16, 16));

  connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(DeleteSelectedParam()));
  
  this->Update();
}

QWidget* QmitkConfigPreferencePage::GetQtControl() const
{
  return pageWidget;
}

bool QmitkConfigPreferencePage::PerformOk()
{
  auto& configManager = config::ConfigManager::GetInstance();
  for (auto& key : m_deletedParams) {
    configManager.Delete(key.c_str());
  }
  return true;
}

void QmitkConfigPreferencePage::PerformCancel()
{
}

void QmitkConfigPreferencePage::Update()
{
  ConfigSaver uiListInitializer(ui.get(), m_deletedParams);
  config::ConfigManager::GetInstance().SaveParams(uiListInitializer);
}

void QmitkConfigPreferencePage::DeleteSelectedParam()
{
  auto selection = ui->configTableWidget->selectedItems();
  if (selection.isEmpty()) {
    return;
  }
  for (auto item : selection) {
    auto row = ui->configTableWidget->row(item);
    if (row > -1) {
      auto key = ui->configTableWidget->item(row, 0);
      m_deletedParams.emplace(key->text().toStdString());
    }
  }
  Update();
}
