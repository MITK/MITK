/*=========================================================================
 
 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#include "cherryQtStylePreferencePage.h"

#include <cherryIPreferencesService.h>
#include <cherryQtPreferences.h>

#include <QFileDialog>
#include <QDirIterator>

namespace cherry
{

void QtStylePreferencePage::Init(IWorkbench::Pointer )
{

}

void QtStylePreferencePage::CreateQtControl(QWidget* parent)
{
  mainWidget = new QWidget(parent);
  controls.setupUi(mainWidget);

  cherry::IPreferencesService::Pointer prefService
    = cherry::Platform::GetServiceRegistry()
    .GetServiceById<cherry::IPreferencesService>(cherry::IPreferencesService::ID);

  styleManager = cherry::Platform::GetServiceRegistry()
    .GetServiceById<cherry::IQtStyleManager>(cherry::IQtStyleManager::ID);

  m_StylePref = prefService->GetSystemPreferences()->Node(cherry::QtPreferences::QT_STYLES_NODE);

  Update();

  connect(controls.m_StylesCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(StyleChanged(int)));
  connect(controls.m_PathList, SIGNAL(itemSelectionChanged()), this, SLOT(UpdatePathListButtons()));
  connect(controls.m_AddButton, SIGNAL(clicked(bool)), this, SLOT(AddPathClicked(bool)));
  connect(controls.m_EditButton, SIGNAL(clicked(bool)), this, SLOT(EditPathClicked(bool)));
  connect(controls.m_RemoveButton, SIGNAL(clicked(bool)), this, SLOT(RemovePathClicked(bool)));
}

void QtStylePreferencePage::FillStyleCombo(const cherry::IQtStyleManager::Style& currentStyle)
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

void QtStylePreferencePage::AddPath(const QString& path, bool updateCombo)
{
  if (!controls.m_PathList->findItems(path, Qt::MatchCaseSensitive).isEmpty()) return;

  new QListWidgetItem(path, controls.m_PathList);

  styleManager->AddStyles(path);

  if (updateCombo)
    FillStyleCombo(oldStyle);
}

void QtStylePreferencePage::StyleChanged(int index)
{
  QString fileName = controls.m_StylesCombo->itemData(controls.m_StylesCombo->currentIndex()).toString();
  styleManager->SetStyle(fileName);
}

void QtStylePreferencePage::AddPathClicked(bool checked)
{
  QListWidgetItem* item = controls.m_PathList->currentItem();
  QString initialDir;
  if (item) initialDir = item->text();
  QString dir = QFileDialog::getExistingDirectory(mainWidget, "", initialDir);

  if (!dir.isEmpty()) this->AddPath(dir, true);
}

void QtStylePreferencePage::RemovePathClicked(bool checked)
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
  m_StylePref->Put(cherry::QtPreferences::QT_STYLE_NAME,
      controls.m_StylesCombo->itemData(controls.m_StylesCombo->currentIndex()).toString().toStdString());

  std::string paths;
  for (int i = 0; i < controls.m_PathList->count(); ++i)
  {
    QString path = controls.m_PathList->item(i)->text() + ";";
    paths += path.toStdString();
  }

  m_StylePref->Put(cherry::QtPreferences::QT_STYLE_SEARCHPATHS, paths);
  return true;
}

void QtStylePreferencePage::PerformCancel()
{
  Update();
}

void QtStylePreferencePage::Update()
{
  styleManager->RemoveStyles();

  QString paths = QString::fromStdString(m_StylePref->Get(cherry::QtPreferences::QT_STYLE_SEARCHPATHS, ""));
  QStringList pathList = paths.split(";", QString::SkipEmptyParts);
  QStringListIterator it(pathList);
  while (it.hasNext())
  {
    AddPath(it.next(), false);
  }

  std::string name = m_StylePref->Get(cherry::QtPreferences::QT_STYLE_NAME, "");
  styleManager->SetStyle(QString::fromStdString(name));
  oldStyle = styleManager->GetStyle();

  FillStyleCombo(oldStyle);
}

}
