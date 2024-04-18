/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkStartupDialog.h>
#include <ui_QmitkStartupDialog.h>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

#include <QDirIterator>

#include <nlohmann/json.hpp>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.startupdialog");
  }
}

QT_BEGIN_NAMESPACE

  void from_json(const nlohmann::json& j, QString& result)
  {
    result = QString::fromStdString(j.get<std::string>());
  }

QT_END_NAMESPACE

class Preset : public QListWidgetItem
{
public:
  static constexpr int NameRole = Qt::DisplayRole;
  static constexpr int InfoRole = Qt::UserRole + 1;
  static constexpr int CategoriesRole = Qt::UserRole + 2;

  explicit Preset(const QString& filename)
  {
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
      mitkThrow() << "Couldn't open \"" << filename.toLatin1() << "\"!";

    auto fileContents = QTextStream(&file).readAll().toStdString();
    auto j = nlohmann::json::parse(fileContents, nullptr, false, true);

    if (j.is_discarded())
      mitkThrow() << "Couldn't parse \"" << filename.toLatin1() << "\"!";

    this->Deserialize(j);
  }

  void Inherit(QList<Preset*>& presets)
  {
    if (m_Ancestor.isEmpty())
      return;

    auto isAncestor = [this](const Preset* p) { return p->GetName() == m_Ancestor; };
    auto ancestor = std::find_if(presets.begin(), presets.end(), isAncestor);

    if (ancestor == presets.end())
    {
      mitkThrow() << "Preset \"" << this->GetName().toLatin1() << "\" cannot inherit from \""
                  << m_Ancestor.toLatin1() << "\" because it couldn't be found.";
    }

    (*ancestor)->Inherit(presets);
    this->MergeCategories(*ancestor);

    m_Ancestor.clear();
  }

  QString GetName() const
  {
    return this->data(NameRole).toString();
  }

  QString GetInfo() const
  {
    return this->data(InfoRole).toString();
  }

  QStringList GetCategories() const
  {
    return this->data(CategoriesRole).toStringList();
  }

private:
  void Deserialize(const nlohmann::json& j)
  {
    this->setData(Preset::NameRole, j["name"].get<QString>());
    this->setData(Preset::InfoRole, j.value("info", QString()));
    this->m_Ancestor = j.value("ancestor", QString());

    if (j.contains("categories"))
      this->setData(Preset::CategoriesRole, j["categories"].get<QStringList>());
  }

  void MergeCategories(const Preset* other)
  {
    auto categories = this->GetCategories();
    categories.append(other->GetCategories());
    categories.removeDuplicates();
    this->setData(CategoriesRole, categories);
  }

  QString m_Ancestor;
};

QmitkStartupDialog::QmitkStartupDialog(QWidget* parent)
  : QDialog(parent),
    m_Ui(new Ui::QmitkStartupDialog)
{
  m_Ui->setupUi(this);

  connect(m_Ui->presetRadioButton, &QAbstractButton::toggled, this, &QmitkStartupDialog::OnPresetRadioButtonToggled);
  connect(m_Ui->presetListWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QmitkStartupDialog::OnSelectedPresetChanged);
  connect(this, &QDialog::finished, this, &QmitkStartupDialog::OnFinished);
}

QmitkStartupDialog::~QmitkStartupDialog()
{
  delete m_Ui;
}

bool QmitkStartupDialog::UsePreset() const
{
  return m_Ui->presetRadioButton->isChecked() && m_Ui->presetListWidget->currentItem() != nullptr;
}

QString QmitkStartupDialog::GetPresetName() const
{
  return m_Ui->presetListWidget->currentItem()->data(Preset::NameRole).toString();
}

QStringList QmitkStartupDialog::GetPresetCategories() const
{
  return m_Ui->presetListWidget->currentItem()->data(Preset::CategoriesRole).toStringList();
}

bool QmitkStartupDialog::SkipDialog() const
{
  auto prefs = GetPreferences();
  return prefs->GetBool("skip", false);
}

void QmitkStartupDialog::LoadPresets()
{
  QList<Preset*> presets;
  QDirIterator it(":/org_mitk_presets");

  while (it.hasNext())
    presets.append(new Preset(it.next()));

  for (auto preset : presets)
  {
    preset->Inherit(presets);
    m_Ui->presetListWidget->addItem(preset);
  }
}

void QmitkStartupDialog::showEvent(QShowEvent*)
{
  if (this->SkipDialog())
    this->reject();

  this->LoadPresets();

  auto prefs = GetPreferences();
  bool usePreset = prefs->GetBool("use preset", false);

  if (usePreset)
  {
    m_Ui->presetRadioButton->setChecked(true);

    QString preset = QString::fromStdString(prefs->Get("preset", ""));

    if (preset.isEmpty())
      return;

    auto items = m_Ui->presetListWidget->findItems(preset, Qt::MatchExactly);

    if (!items.isEmpty())
      m_Ui->presetListWidget->setCurrentItem(items.first());
  }
}

void QmitkStartupDialog::OnPresetRadioButtonToggled(bool checked)
{
  m_Ui->presetListWidget->setEnabled(checked);

  if (!checked)
  {
    m_Ui->presetListWidget->clearSelection();
  }
  else if (m_Ui->presetListWidget->count() > 0)
  {
    m_Ui->presetListWidget->setCurrentRow(0);
  }
}

void QmitkStartupDialog::OnSelectedPresetChanged(const QItemSelection& selected, const QItemSelection&)
{
  if (selected.empty())
  {
    m_Ui->presetLabel->clear();
    return;
  }

  m_Ui->presetLabel->setText(m_Ui->presetListWidget->selectedItems().front()->data(Preset::InfoRole).toString());
}

void QmitkStartupDialog::OnFinished(int result)
{
  auto prefs = GetPreferences();

  bool skipDialog = m_Ui->skipDialogCheckBox->isChecked();
  prefs->PutBool("skip", skipDialog);

  if (result == QDialog::Accepted)
  {
    bool usePreset = m_Ui->presetRadioButton->isChecked();
    prefs->PutBool("use preset", usePreset);

    if (usePreset)
    {
      auto preset = m_Ui->presetListWidget->currentItem()->text().toStdString();
      prefs->Put("preset", preset);
    }
    else
    {
      prefs->Remove("preset");
    }
  }

  prefs->Flush();
}
