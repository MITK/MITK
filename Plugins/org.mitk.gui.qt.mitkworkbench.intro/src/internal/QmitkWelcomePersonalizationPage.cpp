/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkWelcomePersonalizationPage.h"

#include "QmitkMitkWorkbenchIntroPlugin.h"
#include "QmitkWelcomeText.h"

#include <QmitkApplicationConstants.h>
#include <QmitkIconTheme.h>
#include <QmitkPreferencesDialog.h>

#include <berryIQtStyleManager.h>
#include <berryQtPreferences.h>

#include <mitkCoreServices.h>
#include <mitkExceptionMacro.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <mitkLog.h>

#include <QAbstractButton>
#include <QApplication>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>

#include <algorithm>
#include <utility>

namespace
{
  // Presets use their index as id.
  constexpr int ALL_PLUGINS_ID = 1000;
  constexpr int CUSTOM_PRESET_ID = 1001;

  const QString ALL_PLUGINS_NAME = QStringLiteral("All plugins");
  const QString CUSTOM_PRESET_NAME = QStringLiteral("Custom");

  constexpr int DARK_THEME_ID = 0;
  constexpr int LIGHT_THEME_ID = 1;

  constexpr int MITK_INTERACTION_ID = 0;
  constexpr int PACS_INTERACTION_ID = 1;

  // Preference nodes and key of the display editors, spelled out like in their
  // preference pages to not depend on the editor plugins. The standard display
  // is the reference for the current choice.
  const std::string STD_MULTI_WIDGET_EDITOR_NODE = "org.mitk.editors.stdmultiwidget";
  const std::string MXN_MULTI_WIDGET_EDITOR_NODE = "org.mitk.editors.mxnmultiwidget";
  const std::string PACS_INTERACTION_KEY = "PACS like mouse interaction";

  const QString TOOL_BARS_PREFERENCE_PAGE_ID = QStringLiteral("org.mitk.ToolBarsPreferencePage");

  mitk::IPreferences* GetPreferences(const std::string& node)
  {
    return mitk::CoreServices::GetPreferencesService()->GetSystemPreferences()->Node(node);
  }

  std::pair<QWidget*, QVBoxLayout*> CreateCard(const QString& title)
  {
    auto* card = new QWidget;
    card->setObjectName("welcomeCard");
    card->setAttribute(Qt::WA_StyledBackground, true);

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 12, 16, 16);
    layout->setSpacing(8);

    auto* titleLabel = new QLabel(title);
    titleLabel->setObjectName("cardTitle");
    layout->addWidget(titleLabel);

    return { card, layout };
  }

  // The dark theme draws radio buttons for its default widget background,
  // on which the hovered indicators would vanish into the lighter cards.
  QVBoxLayout* AddOptionList(QVBoxLayout* cardLayout)
  {
    auto* optionList = new QWidget;
    optionList->setObjectName("optionList");
    optionList->setAttribute(Qt::WA_StyledBackground, true);

    auto* layout = new QVBoxLayout(optionList);
    layout->setContentsMargins(12, 8, 12, 8);
    layout->setSpacing(6);

    cardLayout->addWidget(optionList);

    return layout;
  }

  QLabel* CreateText(const QString& html = QString())
  {
    auto* label = new QLabel(QmitkWelcomeTextWithLineHeight(html));
    label->setTextFormat(Qt::RichText);
    label->setWordWrap(true);

    return label;
  }

  QLabel* CreateDetailText(const QString& html = QString())
  {
    auto* label = CreateText(html);
    label->setObjectName("mutedLabel");

    return label;
  }

  void UncheckAll(QButtonGroup* group)
  {
    group->setExclusive(false);

    for (auto* button : group->buttons())
      button->setChecked(false);

    group->setExclusive(true);
  }
}

QmitkWelcomePersonalizationPage::QmitkWelcomePersonalizationPage(QWidget* parent)
  : QWidget(parent),
    m_PresetGroup(nullptr),
    m_PresetInfo(nullptr),
    m_ThemeGroup(nullptr),
    m_MouseInteractionGroup(nullptr),
    m_IsApplyingPreset(false),
    m_IsCustomPresetChosen(false)
{
  try
  {
    m_Presets = QmitkToolBarPresets::Load();
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << e.GetDescription();
  }

  auto* rightColumn = new QVBoxLayout;
  rightColumn->setSpacing(16);
  rightColumn->addWidget(this->CreateThemeCard());
  rightColumn->addWidget(this->CreateMouseInteractionCard());
  rightColumn->addStretch(1);

  auto* columns = new QHBoxLayout;
  columns->setSpacing(16);
  columns->addWidget(this->CreatePresetCard(), 1);
  columns->addLayout(rightColumn, 1);

  // The tabs are as high as their highest page, which the cards would fill.
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addLayout(columns);
  layout->addStretch(1);

  this->UpdatePreset();
  this->UpdateTheme();
  this->UpdateMouseInteraction();

  GetPreferences(QmitkApplicationConstants::TOOL_BARS_PREFERENCES)->OnChanged.AddListener(
    mitk::MessageDelegate1<QmitkWelcomePersonalizationPage, const mitk::IPreferences*>(this, &QmitkWelcomePersonalizationPage::OnToolBarPreferencesChanged));

  GetPreferences(STD_MULTI_WIDGET_EDITOR_NODE)->OnChanged.AddListener(
    mitk::MessageDelegate1<QmitkWelcomePersonalizationPage, const mitk::IPreferences*>(this, &QmitkWelcomePersonalizationPage::OnEditorPreferencesChanged));

  connect(QmitkIconTheme::GetInstance(), &QmitkIconTheme::Changed, this, &QmitkWelcomePersonalizationPage::UpdateTheme);
}

QmitkWelcomePersonalizationPage::~QmitkWelcomePersonalizationPage()
{
  GetPreferences(QmitkApplicationConstants::TOOL_BARS_PREFERENCES)->OnChanged.RemoveListener(
    mitk::MessageDelegate1<QmitkWelcomePersonalizationPage, const mitk::IPreferences*>(this, &QmitkWelcomePersonalizationPage::OnToolBarPreferencesChanged));

  GetPreferences(STD_MULTI_WIDGET_EDITOR_NODE)->OnChanged.RemoveListener(
    mitk::MessageDelegate1<QmitkWelcomePersonalizationPage, const mitk::IPreferences*>(this, &QmitkWelcomePersonalizationPage::OnEditorPreferencesChanged));
}

QWidget* QmitkWelcomePersonalizationPage::CreatePresetCard()
{
  auto [card, layout] = CreateCard("Plugin preset");
  layout->addWidget(CreateText("Choose which plugins are offered in the tool bars and in the View Navigator."));

  auto* optionLayout = AddOptionList(layout);

  m_PresetGroup = new QButtonGroup(this);

  auto addOption = [this, optionLayout](const QString& text, int id)
  {
    auto* radioButton = new QRadioButton(text);
    m_PresetGroup->addButton(radioButton, id);
    optionLayout->addWidget(radioButton);
  };

  addOption(ALL_PLUGINS_NAME, ALL_PLUGINS_ID);

  for (int i = 0; i < static_cast<int>(m_Presets.size()); ++i)
    addOption(m_Presets[i].Name, i);

  addOption(CUSTOM_PRESET_NAME + "...", CUSTOM_PRESET_ID);

  m_PresetInfo = CreateDetailText();
  layout->addSpacing(8);
  layout->addWidget(m_PresetInfo);
  layout->addStretch(1);

  connect(m_PresetGroup, &QButtonGroup::idClicked, this, &QmitkWelcomePersonalizationPage::OnPresetClicked);

  return card;
}

QWidget* QmitkWelcomePersonalizationPage::CreateThemeCard()
{
  auto [card, layout] = CreateCard("Theme");

  m_ThemeGroup = new QButtonGroup(this);

  auto* darkRadioButton = new QRadioButton("Dark");
  darkRadioButton->setIcon(QmitkIconTheme::GetIcon(QStringLiteral(":/org.mitk.gui.qt.welcomescreen/img/theme/moon.svg")));
  darkRadioButton->setIconSize(QSize(20, 20));

  auto* lightRadioButton = new QRadioButton("Light");
  lightRadioButton->setIcon(QmitkIconTheme::GetIcon(QStringLiteral(":/org.mitk.gui.qt.welcomescreen/img/theme/sun.svg")));
  lightRadioButton->setIconSize(QSize(20, 20));

  m_ThemeGroup->addButton(darkRadioButton, DARK_THEME_ID);
  m_ThemeGroup->addButton(lightRadioButton, LIGHT_THEME_ID);

  auto* row = new QHBoxLayout;
  row->setSpacing(24);
  row->addWidget(darkRadioButton);
  row->addWidget(lightRadioButton);
  row->addStretch(1);

  auto* optionLayout = AddOptionList(layout);
  optionLayout->setContentsMargins(12, 16, 12, 16);
  optionLayout->addLayout(row);

  connect(m_ThemeGroup, &QButtonGroup::idClicked, this, &QmitkWelcomePersonalizationPage::OnThemeClicked);

  return card;
}

QWidget* QmitkWelcomePersonalizationPage::CreateMouseInteractionCard()
{
  auto [card, layout] = CreateCard("Mouse interaction");

  auto* optionLayout = AddOptionList(layout);

  m_MouseInteractionGroup = new QButtonGroup(this);

  auto addOption = [this, optionLayout](const QString& text, const QString& description, int id)
  {
    auto* radioButton = new QRadioButton(text);
    m_MouseInteractionGroup->addButton(radioButton, id);
    optionLayout->addWidget(radioButton);

    auto* descriptionLabel = CreateDetailText(description);
    descriptionLabel->setContentsMargins(24, 0, 0, 0);
    optionLayout->addWidget(descriptionLabel);
  };

  addOption("MITK",
    "The left mouse button sets the crosshair, the right one zooms, and the middle one pans.",
    MITK_INTERACTION_ID);

  addOption("PACS",
    "The left mouse button uses the tool chosen in a tool bar next to the views, and the right one adjusts the contrast.",
    PACS_INTERACTION_ID);

  connect(m_MouseInteractionGroup, &QButtonGroup::idClicked, this, &QmitkWelcomePersonalizationPage::OnMouseInteractionClicked);

  return card;
}

void QmitkWelcomePersonalizationPage::OnPresetClicked(int id)
{
  // Choosing "Custom..." is respected even if the tool bars still match a
  // preset, e.g. when the preferences are closed without changes.
  m_IsCustomPresetChosen = id == CUSTOM_PRESET_ID;

  if (m_IsCustomPresetChosen)
  {
    QmitkPreferencesDialog dialog(QApplication::activeWindow());
    dialog.SetSelectedPage(TOOL_BARS_PREFERENCE_PAGE_ID);
    dialog.exec();

    this->UpdatePreset();
    return;
  }

  // Applying a preset changes one preference per category. Update the page
  // once afterwards instead of for each intermediate state.
  m_IsApplyingPreset = true;

  QmitkToolBarPresets::SetVisibleCategories(id == ALL_PLUGINS_ID
    ? QmitkToolBarPresets::GetCategories()
    : m_Presets[id].Categories);

  m_IsApplyingPreset = false;

  this->UpdatePreset();
}

void QmitkWelcomePersonalizationPage::OnThemeClicked(int id)
{
  auto* styleManager = QmitkMitkWorkbenchIntroPlugin::GetDefault()->GetStyleManager();

  if (styleManager == nullptr)
    return;

  const QString name = id == DARK_THEME_ID ? "Dark" : "Light";

  berry::IQtStyleManager::StyleList styles;
  styleManager->GetStyles(styles);

  auto style = std::find_if(styles.cbegin(), styles.cend(), [&name](const berry::IQtStyleManager::Style& s) {
    return s.name == name;
  });

  if (style == styles.cend())
  {
    MITK_WARN << "Theme \"" << name.toStdString() << "\" is not available!";
    this->UpdateTheme();
    return;
  }

  // The style manager applies the style but leaves storing it to its callers.
  styleManager->SetStyle(style->fileName);

  auto* prefs = GetPreferences(berry::QtPreferences::QT_STYLES_NODE);
  prefs->Put(berry::QtPreferences::QT_STYLE_NAME, style->fileName.toStdString());
  prefs->Flush();
}

void QmitkWelcomePersonalizationPage::OnMouseInteractionClicked(int id)
{
  const bool pacsInteraction = id == PACS_INTERACTION_ID;

  for (const auto& node : { STD_MULTI_WIDGET_EDITOR_NODE, MXN_MULTI_WIDGET_EDITOR_NODE })
  {
    auto* prefs = GetPreferences(node);
    prefs->PutBool(PACS_INTERACTION_KEY, pacsInteraction);
    prefs->Flush();
  }
}

void QmitkWelcomePersonalizationPage::OnToolBarPreferencesChanged(const mitk::IPreferences*)
{
  if (!m_IsApplyingPreset)
    this->UpdatePreset();
}

void QmitkWelcomePersonalizationPage::OnEditorPreferencesChanged(const mitk::IPreferences*)
{
  this->UpdateMouseInteraction();
}

int QmitkWelcomePersonalizationPage::GetMatchingPresetId() const
{
  const auto categories = QmitkToolBarPresets::GetCategories();
  const auto visibleCategories = QmitkToolBarPresets::GetVisibleCategories();

  if (visibleCategories == categories)
    return ALL_PLUGINS_ID;

  for (int i = 0; i < static_cast<int>(m_Presets.size()); ++i)
  {
    // Presets may name categories that are missing in this application.
    QStringList presetCategories;

    for (const auto& category : categories)
    {
      if (m_Presets[i].Categories.contains(category))
        presetCategories.append(category);
    }

    if (presetCategories == visibleCategories)
      return i;
  }

  return CUSTOM_PRESET_ID;
}

void QmitkWelcomePersonalizationPage::UpdatePreset()
{
  // No preset is stored. Unless "Custom..." was chosen, it is derived from the
  // visible tool bars, so that changes in the Tool Bars preferences show up.
  const int id = m_IsCustomPresetChosen
    ? CUSTOM_PRESET_ID
    : this->GetMatchingPresetId();

  m_PresetGroup->button(id)->setChecked(true);

  QString name;
  QString info;

  switch (id)
  {
    case ALL_PLUGINS_ID:
      name = ALL_PLUGINS_NAME;
      info = "Nothing is filtered out.";
      break;

    case CUSTOM_PRESET_ID:
      name = CUSTOM_PRESET_NAME;
      info = "Choose the plugins yourself in the <i>Tool Bars</i> preferences.";
      break;

    default:
      name = m_Presets[id].Name;
      info = m_Presets[id].Info;
      break;
  }

  m_PresetInfo->setText(QmitkWelcomeTextWithLineHeight(QString("%1: %2").arg(name, info)));
}

void QmitkWelcomePersonalizationPage::UpdateTheme()
{
  const auto* styleManager = QmitkMitkWorkbenchIntroPlugin::GetDefault()->GetStyleManager();
  const QString name = styleManager != nullptr
    ? styleManager->GetStyle().name
    : QString();

  if (name == "Dark")
  {
    m_ThemeGroup->button(DARK_THEME_ID)->setChecked(true);
  }
  else if (name == "Light")
  {
    m_ThemeGroup->button(LIGHT_THEME_ID)->setChecked(true);
  }
  else
  {
    UncheckAll(m_ThemeGroup);
  }
}

void QmitkWelcomePersonalizationPage::UpdateMouseInteraction()
{
  const bool pacsInteraction = GetPreferences(STD_MULTI_WIDGET_EDITOR_NODE)->GetBool(PACS_INTERACTION_KEY, false);
  m_MouseInteractionGroup->button(pacsInteraction ? PACS_INTERACTION_ID : MITK_INTERACTION_ID)->setChecked(true);
}
