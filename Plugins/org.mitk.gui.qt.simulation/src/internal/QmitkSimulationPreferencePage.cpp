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

#include "QmitkSimulationPreferencePage.h"
#include <mitkGetSimulationPreferences.h>
#include <QFileDialog>
#include <QMessageBox>
#include <sofa/helper/system/PluginManager.h>

typedef sofa::helper::system::Plugin Plugin;
typedef sofa::helper::system::PluginManager PluginManager;
typedef sofa::helper::system::PluginManager::PluginIterator PluginIterator;
typedef sofa::helper::system::PluginManager::PluginMap PluginMap;

QmitkSimulationPreferencePage::QmitkSimulationPreferencePage()
  : m_Preferences(mitk::GetSimulationPreferences()),
    m_Control(nullptr)
{
}

QmitkSimulationPreferencePage::~QmitkSimulationPreferencePage()
{
}

void QmitkSimulationPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Control = new QWidget(parent);
  m_Controls.setupUi(m_Control);

  QStringList headerLabels;
  headerLabels << "Name" << "License" << "Version" << "Path";
  m_Controls.pluginsTreeWidget->setHeaderLabels(headerLabels);

  connect(m_Controls.pluginsTreeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(OnSelectedPluginChanged()));
  connect(m_Controls.addButton, SIGNAL(clicked()), this, SLOT(OnAddButtonClicked()));
  connect(m_Controls.removeButton, SIGNAL(clicked()), this, SLOT(OnRemoveButtonClicked()));

  this->Update();
}

QWidget* QmitkSimulationPreferencePage::GetQtControl() const
{
  return m_Control;
}

void QmitkSimulationPreferencePage::Init(berry::IWorkbench::Pointer)
{
}

void QmitkSimulationPreferencePage::OnAddButtonClicked()
{
  QString filter = "SOFA Plugins ";

#if defined(__APPLE__)
  filter += "(*.dylib*)";
#elif defined(WIN32)
  filter += "(*.dll)";
#else
  filter += "(*.so)";
#endif

  std::string path = QFileDialog::getOpenFileName(m_Control, "Add SOFA Plugin", "", filter).toStdString();

  if (path.empty())
    return;

  PluginManager &pluginManager = PluginManager::getInstance();
  std::ostringstream errlog;

  if (pluginManager.loadPlugin(path, &errlog))
  {
    if (!errlog.str().empty())
    {
      auto  messageBox = new QMessageBox(m_Control);
      messageBox->setIcon(QMessageBox::Warning);
      messageBox->setStandardButtons(QMessageBox::Ok);
      messageBox->setText(errlog.str().c_str());
      messageBox->setWindowTitle("Warning");
      messageBox->show();
    }

    Plugin& plugin = pluginManager.getPluginMap()[path];
    plugin.initExternalModule();

    QStringList pluginItem;

    pluginItem
      << plugin.getModuleName()
      << plugin.getModuleLicense()
      << plugin.getModuleVersion()
      << path.c_str();

    m_Controls.pluginsTreeWidget->addTopLevelItem(new QTreeWidgetItem(pluginItem));
  }
  else
  {
    auto  messageBox = new QMessageBox(m_Control);
    messageBox->setIcon(QMessageBox::Critical);
    messageBox->setStandardButtons(QMessageBox::Ok);
    messageBox->setText(errlog.str().c_str());
    messageBox->setWindowTitle("Error");
    messageBox->show();
  }
}

void QmitkSimulationPreferencePage::OnSelectedPluginChanged()
{
  QList<QTreeWidgetItem*> selectedItems = m_Controls.pluginsTreeWidget->selectedItems();

  m_Controls.componentsListWidget->clear();

  if (!selectedItems.isEmpty())
  {
    PluginMap& pluginMap = sofa::helper::system::PluginManager::getInstance().getPluginMap();
    std::string path = selectedItems[0]->text(3).toStdString();

    m_Controls.descriptionPlainTextEdit->setPlainText(pluginMap[path].getModuleDescription());
    m_Controls.componentsListWidget->addItems(QString(pluginMap[path].getModuleComponentList()).split(' ', QString::SkipEmptyParts));
    m_Controls.removeButton->setEnabled(true);
  }
  else
  {
    m_Controls.descriptionPlainTextEdit->clear();
    m_Controls.removeButton->setEnabled(false);
  }
}

void QmitkSimulationPreferencePage::OnRemoveButtonClicked()
{
  QList<QTreeWidgetItem*> selectedItems = m_Controls.pluginsTreeWidget->selectedItems();

  if (selectedItems.isEmpty())
    return;

  std::string path = selectedItems[0]->text(3).toStdString();

  PluginManager& pluginManager = PluginManager::getInstance();
  std::ostringstream errlog;

  if (pluginManager.unloadPlugin(path, &errlog))
  {
    delete selectedItems[0];
  }
  else
  {
    auto  messageBox = new QMessageBox(m_Control);
    messageBox->setIcon(QMessageBox::Critical);
    messageBox->setStandardButtons(QMessageBox::Ok);
    messageBox->setText(errlog.str().c_str());
    messageBox->setWindowTitle("Error");
    messageBox->show();
  }
}

void QmitkSimulationPreferencePage::PerformCancel()
{
}

bool QmitkSimulationPreferencePage::PerformOk()
{
  PluginManager& pluginManager = PluginManager::getInstance();
  PluginMap& pluginMap = pluginManager.getPluginMap();
  QString plugins;

  for (auto it = pluginMap.begin(); it != pluginMap.end(); ++it)
  {
    if (!plugins.isEmpty())
      plugins += ";";

    plugins += QString::fromStdString(it->first);
  }

  m_Preferences->Put("plugins", plugins);
  return true;
}

void QmitkSimulationPreferencePage::Update()
{
  PluginManager& pluginManager = PluginManager::getInstance();
  PluginMap& pluginMap = pluginManager.getPluginMap();

  for (auto it = pluginMap.begin(); it != pluginMap.end(); ++it)
  {
    Plugin& plugin = it->second;

    QStringList pluginItem;

    pluginItem
      << plugin.getModuleName()
      << plugin.getModuleLicense()
      << plugin.getModuleVersion()
      << it->first.c_str();

    m_Controls.pluginsTreeWidget->addTopLevelItem(new QTreeWidgetItem(pluginItem));
  }
}
