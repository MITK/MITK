/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkNewSegmentationDialog.h"
#include <ui_QmitkNewSegmentationDialog.h>

#include <mitkAnatomicalStructureColorPresets.h>
#include <mitkBaseApplication.h>
#include <mitkLabelSetImage.h>

#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryPlatform.h>

#include <algorithm>
#include <iterator>

#include <vtkNew.h>

#include <QColorDialog>
#include <QPushButton>

#include <nlohmann/json.hpp>

namespace
{
  // Get standard label name and color suggestions from embedded XML preset file for anatomical structures.
  QmitkNewSegmentationDialog::SuggestionsType GetStandardSuggestions()
  {
    QmitkNewSegmentationDialog::SuggestionsType standardSuggestions;

    vtkNew<mitk::AnatomicalStructureColorPresets> presets;
    presets->LoadPreset();

    for (const auto& preset : presets->GetColorPresets())
    {
      auto name = QString::fromStdString(preset.first);
      auto color = QColor::fromRgbF(preset.second.GetRed(), preset.second.GetGreen(), preset.second.GetBlue());
      standardSuggestions.emplace(name, color);
    }

    return standardSuggestions;
  }

  // Parse label name and color suggestions from a JSON file. An array of objects is expected, each consisting
  // of a "name" string and an optional "color" string. If present, the "color" string must follow the conventions
  // of QColor::setNamedColor(), i.e., #rrggbb or any SVG color keyword name like CornflowerBlue. Everything else
  // in the JSON file is simply ignored. In case of any error, an empty map is returned.
  QmitkNewSegmentationDialog::SuggestionsType ParseSuggestions(const std::string& filename)
  {
    std::ifstream file(filename);

    if (!file.is_open())
    {
      MITK_ERROR << "Could not open \"" << filename << "\"!";
      return {};
    }

    auto json = nlohmann::json::parse(file, nullptr, false);

    if (json.is_discarded() || !json.is_array())
    {
      MITK_ERROR << "Could not parse \"" << filename << "\" as JSON array!";
      return {};
    }

    QmitkNewSegmentationDialog::SuggestionsType parsedSuggestions;

    for (const auto& obj : json)
    {
      if (!obj.is_object() || !obj.contains("name"))
        continue;

      auto name = QString::fromStdString(obj["name"]);

      QColor color(QColor::Invalid);

      if (obj.contains("color"))
        color.setNamedColor(QString::fromStdString(obj["color"]));

      parsedSuggestions.emplace(name, color);
    }

    if (parsedSuggestions.empty())
      MITK_WARN << "Could not parse any suggestions from \"" << filename << "\"!";

    return parsedSuggestions;
  }

  struct Preferences
  {
    QString labelSuggestions;
    bool replaceStandardSuggestions;
    bool suggestOnce;
  };

  // Get all relevant preferences and consider command-line arguments overrides.
  Preferences GetPreferences()
  {
    auto nodePrefs = berry::Platform::GetPreferencesService()->GetSystemPreferences()->Node("/org.mitk.views.segmentation");

    Preferences prefs;
    
    prefs.labelSuggestions = QString::fromStdString(mitk::BaseApplication::instance().config().getString(mitk::BaseApplication::ARG_SEGMENTATION_LABEL_SUGGESTIONS.toStdString(), ""));

    if (prefs.labelSuggestions.isEmpty())
      prefs.labelSuggestions = nodePrefs->Get("label suggestions", "");

    prefs.replaceStandardSuggestions = nodePrefs->GetBool("replace standard suggestions", true);
    prefs.suggestOnce = nodePrefs->GetBool("suggest once", true);

    return prefs;
  }

  // Get names of all labels in all layers of a LabelSetImage.
  QStringList GetExistingLabelNames(mitk::LabelSetImage* labelSetImage)
  {
    QStringList existingLabelNames;
    existingLabelNames.reserve(labelSetImage->GetTotalNumberOfLabels());

    const auto numLayers = labelSetImage->GetNumberOfLayers();
    for (std::remove_const_t<decltype(numLayers)> layerIndex = 0; layerIndex < numLayers; ++layerIndex)
    {
      const auto* labelSet = labelSetImage->GetLabelSet(layerIndex);

      for (auto labelIter = labelSet->IteratorConstBegin(); labelIter != labelSet->IteratorConstEnd(); ++labelIter)
      {
        if (0 == labelIter->first)
          continue; // Ignore background label

        auto name = QString::fromStdString(labelIter->second->GetName());

        if (!name.isEmpty()) // Potential duplicates do not matter for our purpose
          existingLabelNames.push_back(name);
      }
    }

    return existingLabelNames;
  }

  // Remove blacklisted suggestions.
  QmitkNewSegmentationDialog::SuggestionsType FilterSuggestions(const QmitkNewSegmentationDialog::SuggestionsType& suggestions, const QStringList& blacklist)
  {
    QmitkNewSegmentationDialog::SuggestionsType filteredSuggestions;

    std::remove_copy_if(suggestions.begin(), suggestions.end(), std::inserter(filteredSuggestions, filteredSuggestions.end()), [&blacklist](const auto& suggestion) {
      return blacklist.contains(suggestion.first);
    });

    return filteredSuggestions;
  }
}

QmitkNewSegmentationDialog::QmitkNewSegmentationDialog(QWidget *parent, mitk::LabelSetImage* labelSetImage, Mode mode)
  : QDialog(parent),
    m_Ui(new Ui::QmitkNewSegmentationDialog),
    m_SuggestOnce(true),
    m_Color(Qt::red)
{
  m_Ui->setupUi(this);

  if (RenameLabel == mode)
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

  connect(m_Ui->colorButton, &QToolButton::clicked, this, &QmitkNewSegmentationDialog::OnColorButtonClicked);
  connect(m_Ui->nameLineEdit, &QLineEdit::textChanged, this, &QmitkNewSegmentationDialog::OnTextChanged);
  connect(m_Ui->nameList, &QListWidget::itemSelectionChanged, this, &QmitkNewSegmentationDialog::OnSuggestionSelected);
  connect(m_Ui->nameList, &QListWidget::itemDoubleClicked, this, &QmitkNewSegmentationDialog::OnAccept);
  connect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QmitkNewSegmentationDialog::OnAccept);

  this->UpdateColorButtonBackground();

  auto prefs = GetPreferences();

  if (!prefs.labelSuggestions.isEmpty())
  {
    auto suggestions = ParseSuggestions(prefs.labelSuggestions.toStdString());
    this->SetSuggestions(suggestions, prefs.replaceStandardSuggestions && !suggestions.empty());
  }
  else
  {
    this->SetSuggestions(GetStandardSuggestions(), true);
  }

  if (nullptr != labelSetImage && prefs.suggestOnce)
  {
    auto existingLabelNames = GetExistingLabelNames(labelSetImage);
    m_Suggestions = FilterSuggestions(m_Suggestions, existingLabelNames);

    this->UpdateNameList();
  }
}

QmitkNewSegmentationDialog::~QmitkNewSegmentationDialog()
{
}

void QmitkNewSegmentationDialog::UpdateColorButtonBackground()
{
  m_Ui->colorButton->setStyleSheet("background-color:" + m_Color.name());
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

void QmitkNewSegmentationDialog::SetName(const QString& name)
{
  m_Ui->nameLineEdit->setText(name);
}

void QmitkNewSegmentationDialog::SetColor(const mitk::Color& color)
{
  m_Color.setRgbF(color.GetRed(), color.GetGreen(), color.GetBlue());
  this->UpdateColorButtonBackground();
}

void QmitkNewSegmentationDialog::SetSuggestions(const SuggestionsType& suggestions, bool replaceStandardSuggestions)
{
  if (replaceStandardSuggestions)
  {
    m_Suggestions = suggestions;
  }
  else
  {
    m_Suggestions = GetStandardSuggestions();

    for (const auto& [name, color] : suggestions)
    {
      if (m_Suggestions.end() == m_Suggestions.find(name))
        m_Suggestions[name] = color;
    }
  }

  this->UpdateNameList();
}

void QmitkNewSegmentationDialog::UpdateNameList()
{
  QStringList names;

  for (const auto& suggestion : m_Suggestions)
    names << suggestion.first;

  m_Ui->nameList->clear();
  m_Ui->nameList->addItems(names);
}

void QmitkNewSegmentationDialog::OnAccept()
{
  m_Name = m_Ui->nameLineEdit->text();
  this->accept();
}

void QmitkNewSegmentationDialog::OnTextChanged(const QString& text)
{
  auto finding = m_Ui->nameList->findItems(text, Qt::MatchFlag::MatchExactly);

  if (!finding.isEmpty())
    m_Ui->nameList->setCurrentItem(finding.first());
}

void QmitkNewSegmentationDialog::OnColorButtonClicked()
{
  auto color = QColorDialog::getColor(m_Color);

  if (color.isValid())
  {
    m_Color = color;
    this->UpdateColorButtonBackground();
  }
}

void QmitkNewSegmentationDialog::OnSuggestionSelected()
{
  const auto* currentItem = m_Ui->nameList->currentItem();

  if (currentItem == nullptr)
    return;

  const auto name = currentItem->text();
  m_Ui->nameLineEdit->setText(name);
  auto color = m_Suggestions[name];

  if (color.isValid())
  {
    m_Color = color;
    this->UpdateColorButtonBackground();
  }
}
