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

#include <QByteArray>
#include <QColorDialog>
#include <QCompleter>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPushButton>
#include <QStringListModel>

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
  QmitkNewSegmentationDialog::SuggestionsType ParseSuggestions(const QString& filename)
  {
    QmitkNewSegmentationDialog::SuggestionsType parsedSuggestions;
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly))
    {
      MITK_ERROR << "Could not open \"" << filename.toStdString() << "\"!";
      return parsedSuggestions;
    }

    auto json = file.readAll();
    auto doc = QJsonDocument::fromJson(json);

    if (doc.isNull() || !doc.isArray())
    {
      MITK_ERROR << "Could not parse \"" << filename.toStdString() << "\" as JSON array!";
      return parsedSuggestions;
    }

    auto array = doc.array();

    for (auto valueRef : array)
    {
      if (valueRef.isObject())
      {
        auto object = valueRef.toObject();

        if (object.contains("name") && object["name"].isString())
        {
          auto name = object["name"].toString();
          auto color = object.contains("color") && object["color"].isString()
            ? QColor(object["color"].toString())
            : QColor(QColor::Invalid);

          parsedSuggestions.emplace(name, color);
        }
      }
    }

    if (parsedSuggestions.empty())
      MITK_WARN << "Could not parse any suggestions from \"" << filename.toStdString() << "\"!";

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
    QStringList result;
    result.reserve(labelSetImage->GetTotalNumberOfLabels());

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
          result.push_back(name);
      }
    }

    return result;
  }

  // Remove blacklisted suggestions.
  QmitkNewSegmentationDialog::SuggestionsType FilterSuggestions(const QmitkNewSegmentationDialog::SuggestionsType& suggestions, const QStringList& blacklist)
  {
    QmitkNewSegmentationDialog::SuggestionsType result;

    std::remove_copy_if(suggestions.begin(), suggestions.end(), std::inserter(result, result.end()), [&blacklist](const auto& suggestion) {
      return blacklist.contains(suggestion.first);
      });

    return result;
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

  auto* completer = new QCompleter(QStringList());
  completer->setCaseSensitivity(Qt::CaseInsensitive);

  m_Ui->nameLineEdit->setCompleter(completer);
  m_Ui->nameLineEdit->setFocus();

  connect(completer, qOverload<const QString&>(&QCompleter::activated), this, qOverload<const QString&>(&QmitkNewSegmentationDialog::OnSuggestionSelected));
  connect(m_Ui->colorButton, &QToolButton::clicked, this, &QmitkNewSegmentationDialog::OnColorButtonClicked);
  connect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QmitkNewSegmentationDialog::OnAccept);

  this->UpdateColorButtonBackground();

  auto prefs = GetPreferences();

  if (!prefs.labelSuggestions.isEmpty())
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

    this->UpdateCompleterModel();
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

  this->UpdateCompleterModel();
}

void QmitkNewSegmentationDialog::UpdateCompleterModel()
{
  QStringList names;

  for (const auto& [name, color] : m_Suggestions)
    names << name;

  auto* completerModel = static_cast<QStringListModel*>(m_Ui->nameLineEdit->completer()->model());
  completerModel->setStringList(names);
}

void QmitkNewSegmentationDialog::OnAccept()
{
  m_Name = m_Ui->nameLineEdit->text();
  this->accept();
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

void QmitkNewSegmentationDialog::OnSuggestionSelected(const QString &name)
{
  auto color = m_Suggestions[name];

  if (color.isValid())
  {
    m_Color = color;
    this->UpdateColorButtonBackground();
  }
}
