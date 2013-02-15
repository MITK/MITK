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
#include <sofa/helper/system/PluginManager.h>
#include <berryIPreferencesService.h>
#include <QFileDialog>
#include <QMessageBox>
#include <cassert>

typedef sofa::helper::system::Plugin Plugin;
typedef sofa::helper::system::PluginManager PluginManager;
typedef sofa::helper::system::PluginManager::PluginIterator PluginIterator;
typedef sofa::helper::system::PluginManager::PluginMap PluginMap;

berry::IPreferences::Pointer getSimulationPreferences()
{
  berry::ServiceRegistry& serviceRegistry = berry::Platform::GetServiceRegistry();
  berry::IPreferencesService::Pointer preferencesService = serviceRegistry.GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);
  berry::IPreferences::Pointer preferences = preferencesService->GetSystemPreferences();
  return preferences->Node("/org.mitk.views.simulation");
}

void initSOFAPlugins(berry::IPreferences::Pointer preferences)
{
  if (preferences.IsNull())
    return;

  QString pluginPaths = preferences->GetByteArray(QmitkSimulationPreferencePage::PLUGIN_PATHS, "").c_str();

  if (pluginPaths.isEmpty())
    return;

  QStringList pluginPathList = pluginPaths.split(';', QString::SkipEmptyParts);
  QStringListIterator it(pluginPathList);

  typedef sofa::helper::system::PluginManager PluginManager;
  PluginManager& pluginManager = PluginManager::getInstance();

  while (it.hasNext())
  {
    std::string path = it.next().toStdString();
    std::ostringstream errlog;

    pluginManager.loadPlugin(path, &errlog);

    if (errlog.str().empty())
      pluginManager.getPluginMap()[path].initExternalModule();
  }
}

const std::string QmitkSimulationPreferencePage::PLUGIN_PATHS = "plugin paths";

QmitkSimulationPreferencePage::QmitkSimulationPreferencePage()
  : m_Preferences(getSimulationPreferences()),
    m_Control(NULL)
{
  initSOFAPlugins(m_Preferences);
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

  connect(m_Controls.pluginsTreeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(OnPluginTreeWidgetItemSelectionChanged()));
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

  std::string path = QFileDialog::getOpenFileName(m_Control, "Add SOFA Library", "", filter).toStdString();

  PluginManager &pluginManager = PluginManager::getInstance();
  std::ostringstream errlog;

  if (pluginManager.loadPlugin(path, &errlog))
  {
    if (!errlog.str().empty())
    {
      QMessageBox* messageBox = new QMessageBox(m_Control);
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
    QMessageBox* messageBox = new QMessageBox(m_Control);
    messageBox->setIcon(QMessageBox::Critical);
    messageBox->setStandardButtons(QMessageBox::Ok);
    messageBox->setText(errlog.str().c_str());
    messageBox->setWindowTitle("Error");
    messageBox->show();
  }
}

void QmitkSimulationPreferencePage::OnPluginTreeWidgetItemSelectionChanged()
{
  QList<QTreeWidgetItem*> selectedItems = m_Controls.pluginsTreeWidget->selectedItems();

  if (!selectedItems.isEmpty())
  {
    PluginMap& pluginMap = sofa::helper::system::PluginManager::getInstance().getPluginMap();
    std::string path = selectedItems[0]->text(3).toStdString();

    m_Controls.descriptionPlainTextEdit->setPlainText(pluginMap[path].getModuleDescription());
    m_Controls.removeButton->setEnabled(true);
  }
  else
  {
    m_Controls.descriptionPlainTextEdit->clear();
    m_Controls.componentsListWidget->clear();
    m_Controls.removeButton->setDisabled(true);
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
    QMessageBox* messageBox = new QMessageBox(m_Control);
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
  std::string pluginPaths;

  for (PluginIterator it = pluginMap.begin(); it != pluginMap.end(); ++it)
  {
    if (!pluginPaths.empty())
      pluginPaths += ";";

    pluginPaths += it->first;
  }

  m_Preferences->PutByteArray(PLUGIN_PATHS, pluginPaths);
  return true;
}

void QmitkSimulationPreferencePage::Update()
{
  PluginManager& pluginManager = PluginManager::getInstance();
  PluginMap& pluginMap = pluginManager.getPluginMap();

  for (PluginIterator it = pluginMap.begin(); it != pluginMap.end(); ++it)
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
