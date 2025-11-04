/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkNewSegmentationDialog.h"
#include <ui_QmitkNewSegmentationDialog.h>

#include <mitkLabelSuggestionHelper.h>
#include <mitkMultiLabelIOHelper.h>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

#include <algorithm>

#include <QColorDialog>
#include <QPushButton>

namespace
{
  struct Preferences
  {
    std::vector<std::byte> geometry;
    bool autoFilter;
  };

  Preferences GetPreferences()
  {
    auto* nodePrefs = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences()->Node("/org.mitk.views.segmentation");

    Preferences prefs;
    prefs.geometry = nodePrefs->GetByteArray("QmitkNewSegmentationDialog geometry", nullptr, 0);
    prefs.autoFilter = nodePrefs->GetBool("QmitkNewSegmentationDialog autoFilter", true);

    return prefs;
  }

  void SavePreferences(const QByteArray& geometry, std::optional<bool> autoFilter)
  {
    auto* nodePrefs = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences()->Node("/org.mitk.views.segmentation");

    nodePrefs->PutByteArray("QmitkNewSegmentationDialog geometry",
                           reinterpret_cast<const std::byte*>(geometry.data()),
                           geometry.size());
    if (autoFilter.has_value())
      nodePrefs->PutBool("QmitkNewSegmentationDialog autoFilter", autoFilter.value());
  }
}

QmitkNewSegmentationDialog::QmitkNewSegmentationDialog(
  const mitk::MultiLabelSegmentation* segmentation,
  const mitk::Label* label,
  Mode mode,
  QWidget* parent)
  : QDialog(parent),
  m_Ui(new Ui::QmitkNewSegmentationDialog),
  m_Mode(mode),
  m_EnforceSuggestions(false),
  m_TotalSuggestions(0),
  m_VisibleSuggestions(0)
{
  m_Ui->setupUi(this);

  // Get suggestion preferences
  auto suggestionPref = mitk::LabelSuggestionHelper::GetSuggestionPreferences();
  m_EnforceSuggestions = suggestionPref.enforceSuggestions;

  // Load suggestions
  auto suggestionHelper = mitk::LabelSuggestionHelper::New();
  suggestionHelper->LoadStandardSuggestions();

  if (!suggestionPref.labelSuggestionFile.empty())
  {
    suggestionHelper->ParseSuggestions(suggestionPref.labelSuggestionFile,
      suggestionPref.replaceStandardSuggestions);
  }

  m_Suggestions = (mode == Mode::NewLabel)
    ? suggestionHelper->GetValidSuggestionsForNewLabels(segmentation)
    : suggestionHelper->GetValidSuggestionsForRenamingLabels(segmentation,
      label ? label->GetName() : "");

  m_TotalSuggestions = static_cast<int>(m_Suggestions.size());
  m_VisibleSuggestions = m_TotalSuggestions;

  // Initialize with label data if provided
  if (nullptr != label)
  {
    m_Ui->nameLineEdit->setText(QString::fromStdString(label->GetName()));
    const auto& labelColor = label->GetColor();
    m_Color.setRgb(labelColor.GetRed() * 255,
      labelColor.GetGreen() * 255,
      labelColor.GetBlue() * 255);
  }

  InitializeDialog();
  SetupConnections();
  LoadAndApplyPreferences();
  UpdateUI();
}

QmitkNewSegmentationDialog::~QmitkNewSegmentationDialog()
{
}

void QmitkNewSegmentationDialog::InitializeDialog()
{
  // Set window title
  if (Mode::RenameLabel == m_Mode)
  {
    this->setWindowTitle("Rename Label");
    m_Ui->headerLabel->setText("Rename label");
  }
  else
  {
    this->setWindowTitle("Create Label");
    m_Ui->headerLabel->setText("Create new label");
  }

  // Set OK button text
  m_Ui->buttonBox->button(QDialogButtonBox::Ok)->setText(
    m_Mode == Mode::RenameLabel ? "Rename label" : "Create label");

  // Set focus
  if (m_EnforceSuggestions && !m_Suggestions.empty())
  {
    m_Ui->suggestionList->setFocus();
  }
  else
  {
    m_Ui->nameLineEdit->setFocus();
  }
}

void QmitkNewSegmentationDialog::SetupConnections()
{
  connect(this, &QDialog::finished, this, &QmitkNewSegmentationDialog::OnFinished);
  connect(m_Ui->colorButton, &QToolButton::clicked,
    this, &QmitkNewSegmentationDialog::OnColorButtonClicked);
  connect(m_Ui->nameLineEdit, &QLineEdit::textEdited,
    this, &QmitkNewSegmentationDialog::OnNameEdited);
  connect(m_Ui->filterLineEdit, &QLineEdit::textEdited,
    this, &QmitkNewSegmentationDialog::OnFilterEdited);
  connect(m_Ui->filterClearButton, &QToolButton::clicked,
    this, &QmitkNewSegmentationDialog::OnFilterClearClicked);
  connect(m_Ui->autoFilterCheckBox, &QCheckBox::toggled,
    this, &QmitkNewSegmentationDialog::OnAutoFilterToggled);
  connect(m_Ui->suggestionList, &QListWidget::itemSelectionChanged,
    this, &QmitkNewSegmentationDialog::OnSuggestionSelected);
  connect(m_Ui->suggestionList, &QListWidget::itemDoubleClicked,
    this, &QmitkNewSegmentationDialog::OnSuggestionDoubleClicked);
  connect(m_Ui->buttonBox, &QDialogButtonBox::accepted,
    this, &QmitkNewSegmentationDialog::OnAccept);
}

void QmitkNewSegmentationDialog::LoadAndApplyPreferences()
{
  auto prefs = GetPreferences();

  m_Ui->autoFilterCheckBox->setChecked(m_EnforceSuggestions? false : prefs.autoFilter);

  if (!prefs.geometry.empty())
  {
    this->restoreGeometry(QByteArray(
      reinterpret_cast<const char*>(prefs.geometry.data()),
      prefs.geometry.size()));
  }
}

void QmitkNewSegmentationDialog::UpdateUI()
{
  this->UpdateNameList();
  this->SelectSuggestionByName(m_Ui->nameLineEdit->text());

  this->UpdateColorButtonBackground();
  this->UpdateControlStates();
  this->UpdateSuggestionInfo();
  this->UpdateOKButton();
}

void QmitkNewSegmentationDialog::UpdateColorButtonBackground()
{
  if (m_Color.isValid())
  {
    m_Ui->colorButton->setStyleSheet("background-color:" + m_Color.name());
  }
  else
  {
    m_Ui->colorButton->setStyleSheet("");
  }
}

void QmitkNewSegmentationDialog::UpdateOKButton()
{
  bool enabled = false;

  if (m_EnforceSuggestions)
  {
    // Must have a valid suggestion selected
    enabled = m_Suggestion.IsNotNull();
  }
  else
  {
    // Either have a suggestion or a non-empty name
    enabled = m_Suggestion.IsNotNull() || !m_Ui->nameLineEdit->text().trimmed().isEmpty();
  }

  m_Ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enabled);
}

void QmitkNewSegmentationDialog::UpdateNameList()
{
  m_Ui->suggestionList->clear();

  for (size_t i = 0; i < m_Suggestions.size(); ++i)
  {
    auto* item = new QListWidgetItem(
      QString::fromStdString(m_Suggestions[i]->GetName()));

    // Store the index in the suggestions vector as item data
    item->setData(Qt::UserRole, static_cast<int>(i));

    // Create a color icon for the suggestion
    const auto& labelColor = m_Suggestions[i]->GetColor();
    QPixmap colorPixmap(16, 16);
    colorPixmap.fill(QColor(labelColor.GetRed() * 255,
      labelColor.GetGreen() * 255,
      labelColor.GetBlue() * 255));
    item->setIcon(QIcon(colorPixmap));

    m_Ui->suggestionList->addItem(item);
  }

  // Apply current filter
  if (this->GetAutoFilter())
  {
    ApplyFilter(m_Ui->nameLineEdit->text());
  }
  else
  {
    ApplyFilter(m_Ui->filterLineEdit->text());
  }
}

void QmitkNewSegmentationDialog::UpdateSuggestionInfo()
{
  m_Ui->suggestionInfoLabel->setText(
    QString("Showing %1 of %2 suggestions")
    .arg(m_VisibleSuggestions)
    .arg(m_TotalSuggestions));
}

void QmitkNewSegmentationDialog::UpdateControlStates()
{
  // Enforce suggestions mode: disable name and color editing
  m_Ui->nameLineEdit->setReadOnly(m_EnforceSuggestions);
  m_Ui->colorButton->setEnabled(!m_EnforceSuggestions);

  // Auto filter: hide/show filter controls
  m_Ui->autoFilterCheckBox->setVisible(!m_EnforceSuggestions);
  m_Ui->filterLineEdit->setVisible(!this->GetAutoFilter());
  m_Ui->filterClearButton->setVisible(!this->GetAutoFilter());

  // Show info hint if needed
  bool showHint = m_EnforceSuggestions && m_Suggestion.IsNull();
  m_Ui->infoHintLabel->setVisible(showHint);

  if (showHint)
  {
    if (m_Mode == Mode::NewLabel)
    {
      if (!m_Ui->nameLineEdit->text().isEmpty())
      {
        m_Ui->infoHintLabel->setText(
          "The provided name does not match a valid suggestion.\n"
          "Please select a suggestion from the list.");
      }
      else
      {
        m_Ui->infoHintLabel->setText(
          "Please select a suggestion from the list.");
      }
    }
    else // RenameLabel
    {
      m_Ui->infoHintLabel->setText(
        "The current label name is not valid.\n"
        "Please select a suggestion to rename the label.");
    }
  }
}

void QmitkNewSegmentationDialog::ApplyFilter(const QString& filterText)
{
  m_VisibleSuggestions = 0;
  const int itemCount = m_Ui->suggestionList->count();

  for (int i = 0; i < itemCount; ++i)
  {
    auto* item = m_Ui->suggestionList->item(i);
    bool matches = filterText.isEmpty() ||
      item->text().contains(filterText, Qt::CaseInsensitive);

    item->setHidden(!matches);
    if (matches)
    {
      ++m_VisibleSuggestions;
    }
  }

  UpdateSuggestionInfo();
}

void QmitkNewSegmentationDialog::SelectSuggestionByName(const QString& name)
{
  const int itemCount = m_Ui->suggestionList->count();

  for (int i = 0; i < itemCount; ++i)
  {
    auto* item = m_Ui->suggestionList->item(i);
    if (item->text().compare(name, Qt::CaseInsensitive) == 0 && !item->isHidden())
    {
      m_Ui->suggestionList->setCurrentItem(item); //this triggers implicitly OnSuggestionSelected()
      return;
    }
  }

  // No match found - clear selection
  m_Ui->suggestionList->clearSelection();
  m_Suggestion = nullptr;
}

QString QmitkNewSegmentationDialog::GetName() const
{
  return m_Name;
}

mitk::Color QmitkNewSegmentationDialog::GetColor() const
{
  mitk::Color color;
  if (m_Color.isValid())
  {
    color.SetRed(m_Color.redF());
    color.SetGreen(m_Color.greenF());
    color.SetBlue(m_Color.blueF());
  }
  else
  {
    color.Set(1.0f, 0.0f, 0.0f);
  }

  return color;
}

const mitk::Label* QmitkNewSegmentationDialog::GetSuggestion() const
{
  return m_Suggestion;
}

bool QmitkNewSegmentationDialog::GetAutoFilter() const
{
  return m_Ui->autoFilterCheckBox->isChecked();
}

void QmitkNewSegmentationDialog::OnAccept()
{
  m_Name.clear();

  if (m_Suggestion.IsNotNull())
  { //we have a concrete suggestion template that should be prepared.
    m_Suggestion = m_Suggestion->Clone();
    m_Suggestion->SetColor(this->GetColor());
    mitk::MultiLabelIOHelper::RemoveMetaPropertiesFromLabel(m_Suggestion);
  }
  else
  {
    m_Name = m_Ui->nameLineEdit->text();
  }

  this->accept();
}

void QmitkNewSegmentationDialog::OnFinished(int)
{
  auto autoFilter = m_EnforceSuggestions ? std::optional<bool>() : m_Ui->autoFilterCheckBox->isChecked();

  ::SavePreferences(this->saveGeometry(), autoFilter);
}

void QmitkNewSegmentationDialog::OnNameEdited(const QString& text)
{
  if (!m_EnforceSuggestions)
  {
    // Clear suggestion when manually editing
    m_Suggestion = nullptr;
  }

  if (this->GetAutoFilter())
  {
    this->ApplyFilter(text);
    this->SelectSuggestionByName(text);
  }

  this->UpdateOKButton();
  this->UpdateControlStates();
}

void QmitkNewSegmentationDialog::OnColorButtonClicked()
{
  auto color = QColorDialog::getColor(m_Color, this);

  if (color.isValid())
  {
    m_Color = color;
    this->UpdateColorButtonBackground();
  }
}

void QmitkNewSegmentationDialog::OnSuggestionSelected()
{
  const auto* currentItem = m_Ui->suggestionList->currentItem();

  m_Suggestion = nullptr;

  if (nullptr == currentItem)
  {
    UpdateOKButton();
    UpdateControlStates();
    return;
  }

  // Retrieve the suggestion index stored in the item data
  bool ok = false;
  int suggestionIndex = currentItem->data(Qt::UserRole).toInt(&ok);

  if (!ok || suggestionIndex < 0 ||
    suggestionIndex >= static_cast<int>(m_Suggestions.size()))
  {
    UpdateOKButton();
    UpdateControlStates();
    return;
  }

  m_Suggestion = m_Suggestions[suggestionIndex]->Clone();

  m_Ui->nameLineEdit->setText(QString::fromStdString(m_Suggestion->GetName()));
  const auto& labelColor = m_Suggestion->GetColor();
  m_Color.setRgb(labelColor.GetRed() * 255, labelColor.GetGreen() * 255, labelColor.GetBlue() * 255);

  this->UpdateColorButtonBackground();
  this->UpdateOKButton();
  this->UpdateControlStates();
}

void QmitkNewSegmentationDialog::OnSuggestionDoubleClicked()
{
  // Double-click immediately accepts the dialog if a suggestion is selected
  if (m_Suggestion.IsNotNull())
  {
    this->OnAccept();
  }
}

void QmitkNewSegmentationDialog::OnFilterEdited(const QString& text)
{
  if (!this->GetAutoFilter())
  {
    this->ApplyFilter(text);
  }
}

void QmitkNewSegmentationDialog::OnFilterClearClicked()
{
  m_Ui->filterLineEdit->clear();
  this->ApplyFilter("");
}

void QmitkNewSegmentationDialog::OnAutoFilterToggled(bool /*checked*/)
{
  m_Ui->filterLineEdit->clear();

  QString filterName = this->GetAutoFilter() ? m_Ui->nameLineEdit->text() : m_Ui->filterLineEdit->text();

  this->ApplyFilter(filterName);
  if (this->GetAutoFilter() || !filterName.isEmpty())
  {
    this->SelectSuggestionByName(filterName);
  }

  this->UpdateControlStates();
}

bool QmitkNewSegmentationDialog::DoRenameLabel(mitk::Label* label, mitk::MultiLabelSegmentation* segmentation, QWidget* parent, Mode mode)
{
  if (nullptr == label)
    mitkThrow() << "Invalid call of QmitkNewSegmentationDialog::RenameLabel. Passed label is null.";

  const auto labelValue = label->GetValue();

  if (Mode::RenameLabel==mode &&  nullptr != segmentation && !segmentation->ExistLabel(labelValue))
    mitkThrow() << "Invalid call of QmitkNewSegmentationDialog::RenameLabel. Passed label value does not exist in segmentation.";

  QmitkNewSegmentationDialog dialog(segmentation, label, mode, parent);

  if (dialog.exec() == QDialog::Rejected)
    return false;

  auto name = dialog.GetName();
  if (name.isEmpty())
    name = "Unnamed";

  if (nullptr != segmentation && mode == Mode::RenameLabel)
  { // the label belongs to the passed segmentation, should be updated via the methods
    // of the later.
    if (nullptr != dialog.GetSuggestion())
    {
      segmentation->UpdateLabel(labelValue, dialog.GetSuggestion());
    }
    else
    {
      segmentation->RenameLabel(labelValue, name.toStdString(), dialog.GetColor());
    }
  }
  else
  {
    if (nullptr != dialog.GetSuggestion())
    {
      label->Update(dialog.GetSuggestion(), true);
    }
    else
    {
      label->SetName(name.toStdString());
      label->SetColor(dialog.GetColor());
    }
  }

  return true;
}
