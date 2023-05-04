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
  // Get standard label name and color suggestions from embedded XML preset file for anatomical structures.
  QmitkNewSegmentationDialog::SuggestionsType GetStandardSuggestions()
  {
    vtkNew<mitk::AnatomicalStructureColorPresets> presets;
    presets->LoadPreset();

    auto colorPresets = presets->GetColorPresets();

    QmitkNewSegmentationDialog::SuggestionsType standardSuggestions;
    standardSuggestions.reserve(colorPresets.size());

    for (const auto& preset : colorPresets)
    {
      auto name = QString::fromStdString(preset.first);
      auto color = QColor::fromRgbF(preset.second.GetRed(), preset.second.GetGreen(), preset.second.GetBlue());
      standardSuggestions.emplace_back(name, color);
    }

    std::sort(begin(standardSuggestions), end(standardSuggestions), [](const auto& lhs, const auto& rhs) {
      return lhs.first < rhs.first;
    });

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
      QColor color;

      if (obj.contains("color"))
        color.setNamedColor(QString::fromStdString(obj["color"]));

      auto it = std::find_if(begin(parsedSuggestions), end(parsedSuggestions), [&name](const auto& suggestion) {
        return name == suggestion.first;
      });

      // Ignore duplicates...
      if (it != parsedSuggestions.end())
      {
        // unless we can complete the original suggestion with a valid color.
        if (!it->second.isValid() && color.isValid())
        {
          it->second = color;
        }
        else
        {
          MITK_WARN << "Ignoring duplicate of suggestion \"" << name.toStdString() << "\"!";
        }

        continue;
      }

      parsedSuggestions.emplace_back(name, color);
    }

    if (parsedSuggestions.empty())
      MITK_WARN << "Could not parse any suggestions from \"" << filename << "\"!";

    return parsedSuggestions;
  }

  struct Preferences
  {
    std::string labelSuggestions;
    bool replaceStandardSuggestions;
    bool suggestOnce;
    std::vector<std::byte> geometry;
  };

  // Get all relevant preferences and consider command-line arguments overrides.
  Preferences GetPreferences()
  {
    auto* nodePrefs = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences()->Node("/org.mitk.views.segmentation");

    Preferences prefs;
    
    prefs.labelSuggestions = mitk::BaseApplication::instance().config().getString(mitk::BaseApplication::ARG_SEGMENTATION_LABEL_SUGGESTIONS.toStdString(), "");

    if (prefs.labelSuggestions.empty())
      prefs.labelSuggestions = nodePrefs->Get("label suggestions", "");

    prefs.replaceStandardSuggestions = nodePrefs->GetBool("replace standard suggestions", true);
    prefs.suggestOnce = nodePrefs->GetBool("suggest once", true);
    prefs.geometry = nodePrefs->GetByteArray("QmitkNewSegmentationDialog geometry", nullptr, 0);

    return prefs;
  }

  void SaveGeometry(const QByteArray& geometry)
  {
    auto* nodePrefs = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences()->Node("/org.mitk.views.segmentation");
    nodePrefs->PutByteArray("QmitkNewSegmentationDialog geometry", reinterpret_cast<const std::byte*>(geometry.data()), geometry.size());
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

    std::remove_copy_if(begin(suggestions), end(suggestions), std::inserter(filteredSuggestions, end(filteredSuggestions)), [&blacklist](const auto& suggestion) {
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

  connect(this, &QDialog::finished, this, &QmitkNewSegmentationDialog::OnFinished);
  connect(m_Ui->colorButton, &QToolButton::clicked, this, &QmitkNewSegmentationDialog::OnColorButtonClicked);
  connect(m_Ui->nameLineEdit, &QLineEdit::textChanged, this, &QmitkNewSegmentationDialog::OnTextChanged);
  connect(m_Ui->nameList, &QListWidget::itemSelectionChanged, this, &QmitkNewSegmentationDialog::OnSuggestionSelected);
  connect(m_Ui->nameList, &QListWidget::itemDoubleClicked, this, &QmitkNewSegmentationDialog::OnAccept);
  connect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QmitkNewSegmentationDialog::OnAccept);

  this->UpdateColorButtonBackground();

  auto prefs = GetPreferences();

  if (!prefs.labelSuggestions.empty())
  {
    auto suggestions = ParseSuggestions(prefs.labelSuggestions);
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
  m_Suggestions = suggestions;

  if (!replaceStandardSuggestions)
  {
    auto standardSuggestions = GetStandardSuggestions();

    // Append all standard suggestions with unique names to the list of custom suggestions
    std::remove_copy_if(begin(standardSuggestions), end(standardSuggestions), std::inserter(m_Suggestions, end(m_Suggestions)), [this](const auto& suggestion) {
      return m_Suggestions.end() != std::find_if(begin(m_Suggestions), end(m_Suggestions), [&suggestion](const auto& otherSuggestion) {
        return suggestion.first == otherSuggestion.first;
      });
    });
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

  auto row = m_Ui->nameList->selectionModel()->selectedIndexes().first().row();

  m_Ui->nameLineEdit->setText(m_Suggestions[row].first);
  auto color = m_Suggestions[row].second;

  if (color.isValid())
  {
    m_Color = color;
    this->UpdateColorButtonBackground();
  }
}

bool QmitkNewSegmentationDialog::DoRenameLabel(mitk::Label* label, mitk::LabelSetImage* segmentation, QWidget* parent, Mode mode)
{
  if (nullptr == label)
    mitkThrow() << "Invalid call of QmitkNewSegmentationDialog::RenameLabel. Passed label is null.";

  const auto labelValue = label->GetValue();
  mitk::LabelSetImage::GroupIndexType groupIndex;

  if (nullptr != segmentation && !segmentation->IsLabelInGroup(labelValue, groupIndex))
    mitkThrow() << "Invalid call of QmitkNewSegmentationDialog::RenameLabel. Passed label value does not exist in segmentation.";

  QmitkNewSegmentationDialog dialog(parent, segmentation, mode);

  dialog.SetColor(label->GetColor());
  dialog.SetName(QString::fromStdString(label->GetName()));

  if (dialog.exec() == QDialog::Rejected)
    return false;

  auto name = dialog.GetName();

  if (name.isEmpty())
    name = "Unnamed";

  if (nullptr != segmentation)
  {
    auto group = segmentation->GetLabelSet(groupIndex);
    group->RenameLabel(labelValue, name.toStdString(), dialog.GetColor());
    group->UpdateLookupTable(labelValue);
  }
  else
  {
    label->SetName(name.toStdString());
    label->SetColor(dialog.GetColor());
  }

  return true;
}
