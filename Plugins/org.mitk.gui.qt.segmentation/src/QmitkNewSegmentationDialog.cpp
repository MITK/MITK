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

#include <vtkNew.h>

#include <QColorDialog>
#include <QPushButton>

#include <nlohmann/json.hpp>

namespace
{
  struct Preferences
  {
    std::vector<std::byte> geometry;
  };

  // Get all relevant preferences and consider command-line arguments overrides.
  Preferences GetPreferences()
  {
    auto* nodePrefs = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences()->Node("/org.mitk.views.segmentation");

    Preferences prefs;

    prefs.geometry = nodePrefs->GetByteArray("QmitkNewSegmentationDialog geometry", nullptr, 0);

    return prefs;
  }

  void SaveGeometry(const QByteArray& geometry)
  {
    auto* nodePrefs = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences()->Node("/org.mitk.views.segmentation");
    nodePrefs->PutByteArray("QmitkNewSegmentationDialog geometry", reinterpret_cast<const std::byte*>(geometry.data()), geometry.size());
  }
}

QmitkNewSegmentationDialog::QmitkNewSegmentationDialog(const mitk::MultiLabelSegmentation* segmentation, const mitk::Label* label, Mode mode, QWidget* parent)
  : QDialog(parent),
    m_Ui(new Ui::QmitkNewSegmentationDialog)
{
  m_Ui->setupUi(this);

  if (Mode::RenameLabel == mode)
  {
    this->setWindowTitle("Rename Label");
    m_Ui->label->setText("New name and color of the label");
    m_Ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Rename label");
  }
  else
  {
    m_Ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Create label");
  }

  m_Ui->nameLineEdit->setFocus();

  connect(this, &QDialog::finished, this, &QmitkNewSegmentationDialog::OnFinished);
  connect(m_Ui->colorButton, &QToolButton::clicked, this, &QmitkNewSegmentationDialog::OnColorButtonClicked);
  connect(m_Ui->nameLineEdit, &QLineEdit::textEdited, this, &QmitkNewSegmentationDialog::OnTextEdited);
  connect(m_Ui->nameList, &QListWidget::itemSelectionChanged, this, &QmitkNewSegmentationDialog::OnSuggestionSelected);
  connect(m_Ui->nameList, &QListWidget::itemDoubleClicked, this, &QmitkNewSegmentationDialog::OnAccept);
  connect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QmitkNewSegmentationDialog::OnAccept);

  auto prefs = GetPreferences();
  auto suggestionHelper = mitk::LabelSuggestionHelper::New();
  suggestionHelper->LoadStandardSuggestions();
  auto suggestionPref = mitk::LabelSuggestionHelper::GetSuggestionPreferences();

  if (!suggestionPref.labelSuggestionFile.empty())
  {
    suggestionHelper->ParseSuggestions(suggestionPref.labelSuggestionFile, suggestionPref.replaceStandardSuggestions);
  }

  m_Suggestions = mode == Mode::NewLabel ?
    suggestionHelper->GetValidAddSuggestions(segmentation, suggestionPref.suggestionOnce) :
    suggestionHelper->GetValidRenameSuggestions(segmentation, label->GetName());
  this->UpdateNameList();

  if (nullptr != label)
  {
    m_Ui->nameLineEdit->setText(QString::fromStdString(label->GetName()));
    const auto& labelColor = label->GetColor();
    m_Color.setRgb(labelColor.GetRed() * 255, labelColor.GetGreen() * 255, labelColor.GetBlue() * 255);
  }

  this->UpdateColorButtonBackground();
  this->UpdateOKButton();

  if (Mode::NewLabel == mode
    && !m_Suggestions.empty()
    && (nullptr == label
      || (segmentation && !suggestionHelper->IsNewInstanceAllowed(segmentation, label->GetName()))))
  { // If no label provided in add mode or it has no name, preselect the first valid suggestion
    // by setting the text field and let the rest be handled by the events
    auto autoSelectedName = QString::fromStdString(m_Suggestions.front()->GetName());
    m_Ui->nameLineEdit->setText(autoSelectedName);
    this->OnTextEdited(autoSelectedName);
  }

  if (!(prefs.geometry.empty()))
    this->restoreGeometry(QByteArray(reinterpret_cast<const char*>(prefs.geometry.data()), prefs.geometry.size()));
}

QmitkNewSegmentationDialog::~QmitkNewSegmentationDialog()
{
}

void QmitkNewSegmentationDialog::OnFinished(int)
{
  SaveGeometry(this->saveGeometry());
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

void QmitkNewSegmentationDialog::UpdateColorButtonBackground()
{
  m_Ui->colorButton->setStyleSheet("background-color:" + m_Color.name());
}

void QmitkNewSegmentationDialog::UpdateOKButton()
{
  auto pref = mitk::LabelSuggestionHelper::GetSuggestionPreferences();
  m_Ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(m_Suggestion.IsNotNull() || !pref.enforceSuggestions);
}

void QmitkNewSegmentationDialog::UpdateNameList()
{
  QStringList names;

  for (const auto& suggestion : m_Suggestions)
    names << QString::fromStdString(suggestion->GetName());

  m_Ui->nameList->clear();
  m_Ui->nameList->addItems(names);
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

void QmitkNewSegmentationDialog::OnTextEdited(const QString& text)
{
  const int numberOfItems = m_Ui->nameList->count();
  bool suggestionSelected = false;

  for (int row = 0; row < numberOfItems; ++row)
  {
    auto item = m_Ui->nameList->item(row);
    bool match = item->text().contains(text, Qt::CaseInsensitive);
    item->setHidden(!match);

    if (match && item->text().compare(text, Qt::CaseInsensitive) == 0)
    {
      m_Ui->nameList->setCurrentItem(item); //this triggers implicitly OnSuggestionSelected()
      suggestionSelected = true;
    }
  }

  if (!suggestionSelected) m_Suggestion = nullptr;
  this->UpdateOKButton();
}

void QmitkNewSegmentationDialog::OnColorButtonClicked()
{
  auto color = QColorDialog::getColor(m_Color);

  if (color.isValid())
  {
    this->UpdateColorButtonBackground();
  }
}

void QmitkNewSegmentationDialog::OnSuggestionSelected()
{
  const auto* currentItem = m_Ui->nameList->currentItem();

  if (currentItem == nullptr)
    return;

  auto row = m_Ui->nameList->selectionModel()->selectedIndexes().first().row();

  m_Suggestion = m_Suggestions[row]->Clone();

  m_Ui->nameLineEdit->setText(QString::fromStdString(m_Suggestion->GetName()));
  const auto& labelColor = m_Suggestion->GetColor();
  m_Color.setRgb(labelColor.GetRed() * 255, labelColor.GetGreen() * 255, labelColor.GetBlue() * 255);

  this->UpdateColorButtonBackground();
  this->UpdateOKButton();
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
