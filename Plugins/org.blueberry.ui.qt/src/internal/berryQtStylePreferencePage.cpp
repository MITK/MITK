/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryQtStylePreferencePage.h"

#include "berryWorkbenchPlugin.h"
#include <berryQtPreferences.h>

#include <QFileDialog>
#include <QDirIterator>
#include <QFontDatabase>

#include <mitkIPreferences.h>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    return berry::WorkbenchPlugin::GetDefault()->GetPreferences()->Node(berry::QtPreferences::QT_STYLES_NODE);
  }
}

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

  ctkPluginContext* context = berry::WorkbenchPlugin::GetDefault()->GetPluginContext();
  ctkServiceReference styleManagerRef = context->getServiceReference<berry::IQtStyleManager>();
  if (styleManagerRef)
  {
    styleManager = context->getService<berry::IQtStyleManager>(styleManagerRef);
  }

  Update();

  connect(controls.m_StylesCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(StyleChanged(int)));
  connect(controls.m_FontComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(FontChanged(int)));
  connect(controls.m_FontSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(FontChanged(int)));
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

void QtStylePreferencePage::FillFontCombo(const QString& currentFont)
{
  controls.m_FontComboBox->clear();
  QStringList fonts;
  styleManager->GetFonts(fonts);

  for (int i = 0; i < fonts.size(); ++i)
  {
    controls.m_FontComboBox->addItem(fonts.at(i));
  }
  controls.m_FontComboBox->setCurrentIndex(fonts.indexOf(currentFont));
  if (currentFont == QString("<<system>>"))
  {
    controls.m_FontSizeSpinBox->setEnabled(false);
  }
  else
  {
    controls.m_FontSizeSpinBox->setEnabled(true);
  }
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

void QtStylePreferencePage::FontChanged(int /*index*/)
{
  QString fontName = controls.m_FontComboBox->currentText();
  int fontSize = controls.m_FontSizeSpinBox->value();

  if (fontName == QString("<<system>>"))
  {
    controls.m_FontSizeSpinBox->setEnabled(false);
  }
  else
  {
    controls.m_FontSizeSpinBox->setEnabled(true);
  }

  styleManager->SetFont(fontName);
  styleManager->SetFontSize(fontSize);
  styleManager->UpdateWorkbenchFont();
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
  auto* prefs = GetPreferences();

  prefs->Put(berry::QtPreferences::QT_STYLE_NAME, controls.m_StylesCombo->itemData(controls.m_StylesCombo->currentIndex()).toString().toStdString());

  QString paths;
  for (int i = 0; i < controls.m_PathList->count(); ++i)
  {
    QString path = controls.m_PathList->item(i)->text() + ";";
    paths += path;
  }

  prefs->Put(berry::QtPreferences::QT_STYLE_SEARCHPATHS, paths.toStdString());
  prefs->Put(berry::QtPreferences::QT_FONT_NAME, controls.m_FontComboBox->currentText().toStdString());
  prefs->Put(berry::QtPreferences::QT_FONT_SIZE, std::to_string(controls.m_FontSizeSpinBox->value()));

  prefs->PutBool(berry::QtPreferences::QT_SHOW_TOOLBAR_CATEGORY_NAMES,
    controls.m_ToolbarCategoryCheckBox->isChecked());

  return true;
}

void QtStylePreferencePage::PerformCancel()
{
  Update();
}

void QtStylePreferencePage::Update()
{
  styleManager->RemoveStyles();

  auto* prefs = GetPreferences();

  auto paths = QString::fromStdString(prefs->Get(berry::QtPreferences::QT_STYLE_SEARCHPATHS, ""));
  QStringList pathList = paths.split(";", QString::SkipEmptyParts);
  QStringListIterator it(pathList);
  while (it.hasNext())
  {
    AddPath(it.next(), false);
  }

  auto styleName = QString::fromStdString(prefs->Get(berry::QtPreferences::QT_STYLE_NAME, ""));
  styleManager->SetStyle(styleName);
  oldStyle = styleManager->GetStyle();
  FillStyleCombo(oldStyle);

  auto fontName = QString::fromStdString(prefs->Get(berry::QtPreferences::QT_FONT_NAME, "Open Sans"));
  styleManager->SetFont(fontName);

  auto fontSize = std::stoi(prefs->Get(berry::QtPreferences::QT_FONT_SIZE, "9"));
  styleManager->SetFontSize(fontSize);
  controls.m_FontSizeSpinBox->setValue(fontSize);
  styleManager->UpdateWorkbenchFont();

  FillFontCombo(styleManager->GetFont());

  controls.m_ToolbarCategoryCheckBox->setChecked(
    prefs->GetBool(berry::QtPreferences::QT_SHOW_TOOLBAR_CATEGORY_NAMES, true));
}

}
