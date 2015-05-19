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

#include "berryQtStylePreferencePage.h"

#include <berryIPreferencesService.h>
#include <berryQtPreferences.h>

#include "berryWorkbenchPlugin.h"

#include <QFileDialog>
#include <QDirIterator>

namespace berry
{

QtStylePreferencePage::QtStylePreferencePage()
{

}

void QtStylePreferencePage::Init(IWorkbench::Pointer )
{

}

void QtStylePreferencePage::CreateQtControl(QWidget* parent)
{
  mainWidget = new QWidget(parent);
  controls.setupUi(mainWidget);

  berry::IPreferencesService* prefService = berry::WorkbenchPlugin::GetDefault()->GetPreferencesService();

  ctkPluginContext* context = berry::WorkbenchPlugin::GetDefault()->GetPluginContext();
  ctkServiceReference styleManagerRef = context->getServiceReference<berry::IQtStyleManager>();
  if (styleManagerRef)
  {
    styleManager = context->getService<berry::IQtStyleManager>(styleManagerRef);
  }

  m_StylePref = prefService->GetSystemPreferences()->Node(berry::QtPreferences::QT_STYLES_NODE);

  Update();

  connect(controls.m_StylesCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(StyleChanged(int)));
  connect(controls.m_IconThemeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(IconThemeChanged(int)));
  connect(controls.m_PathList, SIGNAL(itemSelectionChanged()), this, SLOT(UpdatePathListButtons()));
  connect(controls.m_AddButton, SIGNAL(clicked(bool)), this, SLOT(AddPathClicked(bool)));
  connect(controls.m_EditButton, SIGNAL(clicked(bool)), this, SLOT(EditPathClicked(bool)));
  connect(controls.m_RemoveButton, SIGNAL(clicked(bool)), this, SLOT(RemovePathClicked(bool)));
}

void QtStylePreferencePage::FillStyleCombo(const berry::IQtStyleManager::Style& currentStyle)
{
  controls.m_StylesCombo->clear();
  styles.clear();
  styleManager->GetStyles(styles);

  qSort(styles);
  for (int i = 0; i < styles.size(); ++i)
  {
    controls.m_StylesCombo->addItem(styles.at(i).name, QVariant(styles.at(i).fileName));
  }
  controls.m_StylesCombo->setCurrentIndex(styles.indexOf(currentStyle));
}

void QtStylePreferencePage::FillIconThemeComboBox(const QString currentIconTheme)
{
  controls.m_IconThemeComboBox->clear();
  berry::IQtStyleManager::IconThemeList iconThemes;
  styleManager->GetIconThemes(iconThemes);

  qSort(iconThemes);
  for (int i = 0; i < iconThemes.size(); ++i)
  {
    controls.m_IconThemeComboBox->addItem(iconThemes.at(i).name);
  }

  QString currentTheme = currentIconTheme;

  if(currentTheme == QString(""))
  {
    currentTheme = QString("<<default>>");
  }

  controls.m_IconThemeComboBox->setCurrentIndex(iconThemes.indexOf( currentTheme ));
}

void QtStylePreferencePage::AddPath(const QString& path, bool updateCombo)
{
  if (!controls.m_PathList->findItems(path, Qt::MatchCaseSensitive).isEmpty()) return;

  new QListWidgetItem(path, controls.m_PathList);

  styleManager->AddStyles(path);

  if (updateCombo)
    FillStyleCombo(oldStyle);
}

void QtStylePreferencePage::StyleChanged(int /*index*/)
{
  QString fileName = controls.m_StylesCombo->itemData(controls.m_StylesCombo->currentIndex()).toString();
  styleManager->SetStyle(fileName);
}

void QtStylePreferencePage::IconThemeChanged(int /*index*/)
{
  QString themeName = controls.m_IconThemeComboBox->currentText();
  styleManager->SetIconTheme(themeName);
}

void QtStylePreferencePage::AddPathClicked(bool /*checked*/)
{
  QListWidgetItem* item = controls.m_PathList->currentItem();
  QString initialDir;
  if (item) initialDir = item->text();
  QString dir = QFileDialog::getExistingDirectory(mainWidget, "", initialDir);

  if (!dir.isEmpty()) this->AddPath(dir, true);
}

void QtStylePreferencePage::RemovePathClicked(bool /*checked*/)
{
  QList<QListWidgetItem*> selection = controls.m_PathList->selectedItems();
  QListIterator<QListWidgetItem*> it(selection);
  while (it.hasNext())
  {
    QListWidgetItem* item = it.next();
    QString dir = item->text();

    controls.m_PathList->takeItem(controls.m_PathList->row(item));
    delete item;
    styleManager->RemoveStyles(dir);
  }

  if (!styleManager->Contains(oldStyle.fileName))
  {
    oldStyle = styleManager->GetDefaultStyle();
  }

  FillStyleCombo(oldStyle);
}

void QtStylePreferencePage::EditPathClicked(bool checked)
{
  QListWidgetItem* item = controls.m_PathList->currentItem();
  QString initialDir = item->text();
  QString dir = QFileDialog::getExistingDirectory(mainWidget, "", initialDir);

  if (!dir.isEmpty())
  {
    this->RemovePathClicked(checked);
    this->AddPath(dir, true);
  }
}

void QtStylePreferencePage::UpdatePathListButtons()
{
  int s = controls.m_PathList->selectedItems().size();

  if (s == 0)
  {
    controls.m_EditButton->setEnabled(false);
    controls.m_RemoveButton->setEnabled(false);
  }
  else if (s == 1)
  {
    controls.m_EditButton->setEnabled(true);
    controls.m_RemoveButton->setEnabled(true);
  }
  else
  {
    controls.m_EditButton->setEnabled(false);
    controls.m_RemoveButton->setEnabled(true);
  }
}

QWidget* QtStylePreferencePage::GetQtControl() const
{
  return mainWidget;
}

bool QtStylePreferencePage::PerformOk()
{
  m_StylePref->Put(berry::QtPreferences::QT_STYLE_NAME,
      controls.m_StylesCombo->itemData(controls.m_StylesCombo->currentIndex()).toString());

  QString paths;
  for (int i = 0; i < controls.m_PathList->count(); ++i)
  {
    QString path = controls.m_PathList->item(i)->text() + ";";
    paths += path;
  }

  QString currentTheme = QIcon::themeName();
  if(currentTheme.isEmpty())
  {
    currentTheme = QString("<<default>>");
  }
  m_StylePref->Put(berry::QtPreferences::QT_STYLE_SEARCHPATHS, paths);
  m_StylePref->Put(berry::QtPreferences::QT_ICON_THEME, currentTheme);
  return true;
}

void QtStylePreferencePage::PerformCancel()
{
  Update();
}

void QtStylePreferencePage::Update()
{
  styleManager->RemoveStyles();

  QString paths = m_StylePref->Get(berry::QtPreferences::QT_STYLE_SEARCHPATHS, "");
  QStringList pathList = paths.split(";", QString::SkipEmptyParts);
  QStringListIterator it(pathList);
  while (it.hasNext())
  {
    AddPath(it.next(), false);
  }

  QString iconTheme = m_StylePref->Get(berry::QtPreferences::QT_ICON_THEME, "<<default>>");
  styleManager->SetIconTheme( iconTheme );

  QString name = m_StylePref->Get(berry::QtPreferences::QT_STYLE_NAME, "");
  styleManager->SetStyle(name);
  oldStyle = styleManager->GetStyle();

  FillStyleCombo(oldStyle);
  FillIconThemeComboBox( iconTheme );
}

}
