/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "QmitkStdMultiWidgetEditorPreferencePage.h"
#include <ui_QmitkStdMultiWidgetEditorPreferencePage.h>
#include "QmitkStdMultiWidgetEditor.h"
#include "QmitkRenderWindowColorWidget.h"

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

#include <QCheckBox>
#include <QColor>
#include <QGridLayout>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

#include <algorithm>
#include <string>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node(QmitkStdMultiWidgetEditor::EDITOR_ID.toStdString());
  }

  // Mirrors the logo resolution in QmitkMultiWidgetDecorationManager: the first
  // "DepartmentLogo" found while walking up the nodes decides; an empty value
  // hides the logo, an absent key falls back to the default logo.
  bool IsDepartmentLogoVisible(const mitk::IPreferences* preferences)
  {
    for (const auto* node = preferences; node != nullptr; node = node->Parent())
    {
      const auto keys = node->Keys();
      if (std::find(keys.begin(), keys.end(), "DepartmentLogo") != keys.end())
        return !node->Get("DepartmentLogo", "").empty();
    }

    return true;
  }
}

QmitkStdMultiWidgetEditorPreferencePage::QmitkStdMultiWidgetEditorPreferencePage()
  : m_Ui(std::make_unique<Ui::QmitkStdMultiWidgetEditorPreferencePage>()),
    m_ColorWidgets{},
    m_SyncCheckBox(nullptr),
    m_Control(nullptr)
{
}

QmitkStdMultiWidgetEditorPreferencePage::~QmitkStdMultiWidgetEditorPreferencePage()
{
}

void QmitkStdMultiWidgetEditorPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Control = new QWidget(parent);

  m_Ui->setupUi(m_Control);

  auto* groupLayout = new QVBoxLayout(m_Ui->m_ColorGroup);

  m_SyncCheckBox = new QCheckBox("Sync 2D window background colors", m_Ui->m_ColorGroup);
  m_SyncCheckBox->setToolTip("Apply a background color or gradient change in one 2D window to the other 2D windows.");
  groupLayout->addWidget(m_SyncCheckBox);

  // The four render windows in the default layout of the standard display:
  //   axial    | sagittal
  //   coronal  | 3D
  auto* colorLayout = new QGridLayout;
  for (auto*& colorWidget : m_ColorWidgets)
  {
    colorWidget = new QmitkRenderWindowColorWidget(m_Ui->m_ColorGroup);
  }
  colorLayout->addWidget(m_ColorWidgets[0], 0, 0);
  colorLayout->addWidget(m_ColorWidgets[1], 0, 1);
  colorLayout->addWidget(m_ColorWidgets[2], 1, 0);
  colorLayout->addWidget(m_ColorWidgets[3], 1, 1);
  groupLayout->addLayout(colorLayout);

  // Only the 3D window carries the department/MITK logo in its bottom-right corner.
  m_ColorWidgets[3]->SetLogo(QPixmap(QStringLiteral(":/org.mitk.gui.qt.stdmultiwidgeteditor/defaultWatermark.png")));

  // The 2D windows (indices 0..2) can share their background; the 3D window
  // (index 3) keeps its own gradient and is excluded from the sync.
  for (int i = 0; i < 3; ++i)
  {
    connect(m_ColorWidgets[i], &QmitkRenderWindowColorWidget::BackgroundChanged, this, [this, i]() { this->OnBackgroundChanged(i); });
  }

  // Propagate the previews' height-for-width up through the group box, so the
  // outer layout reserves enough height to keep their 4:3 aspect ratio.
  auto colorGroupPolicy = m_Ui->m_ColorGroup->sizePolicy();
  colorGroupPolicy.setHeightForWidth(true);
  m_Ui->m_ColorGroup->setSizePolicy(colorGroupPolicy);

  connect(m_Ui->m_ResetButton, &QPushButton::clicked, this, &QmitkStdMultiWidgetEditorPreferencePage::ResetPreferencesAndGUI);

  this->Update();
}

QWidget* QmitkStdMultiWidgetEditorPreferencePage::GetQtControl() const
{
  return m_Control;
}

void QmitkStdMultiWidgetEditorPreferencePage::Init(berry::IWorkbench::Pointer)
{
}

void QmitkStdMultiWidgetEditorPreferencePage::PerformCancel()
{
}

bool QmitkStdMultiWidgetEditorPreferencePage::PerformOk()
{
  auto* prefs = GetPreferences();

  for (std::size_t i = 0; i < m_ColorWidgets.size(); ++i)
  {
    const auto* colorWidget = m_ColorWidgets[i];
    const std::string widgetName = "stdmulti.widget" + std::to_string(i);

    const QColor upper = colorWidget->GetUpperColor();

    // Without a gradient the window is a single color; since the editor always
    // applies both background colors, "no gradient" means storing them equal.
    const QColor lower = colorWidget->IsGradientEnabled() ? colorWidget->GetLowerColor() : upper;

    prefs->Put(widgetName + " first background color", upper.name().toStdString());
    prefs->Put(widgetName + " second background color", lower.name().toStdString());
    prefs->Put(widgetName + " decoration color", colorWidget->GetDecorationColor().name().toStdString());
    prefs->Put(widgetName + " corner annotation", colorWidget->GetAnnotation().toStdString());
  }

  prefs->PutInt("crosshair gap size", m_Ui->m_CrosshairGapSize->value());
  prefs->PutBool("Show level/window widget", m_Ui->m_ShowLevelWindowWidget->isChecked());
  prefs->PutBool("PACS like mouse interaction", m_Ui->m_PACSLikeMouseMode->isChecked());
  prefs->PutBool("sync 2D background colors", m_SyncCheckBox->isChecked());

  // Public toggle for the department/MITK logo: removing our override falls back
  // to the default logo (or a deployment-configured one in a parent node); an
  // empty value hides it. Mirrors IsDepartmentLogoVisible and the manager.
  const bool logoWasVisible = IsDepartmentLogoVisible(prefs);
  if (m_ColorWidgets[3]->IsLogoVisible())
  {
    // Remove() emits OnChanged when the key existed, so showing applies live.
    prefs->Remove("DepartmentLogo");
  }
  else
  {
    prefs->Put("DepartmentLogo", "");

    // Put() with an empty value on a not-yet-present key is a no-op change, so
    // Preferences emits no OnChanged and the editor would keep showing the logo
    // until the next restart. Notify listeners explicitly in that case.
    if (logoWasVisible)
      prefs->OnChanged.Send(prefs);
  }

  return true;
}

void QmitkStdMultiWidgetEditorPreferencePage::Update()
{
  auto* prefs = GetPreferences();

  // Defaults mirror QmitkStdMultiWidgetEditor: the three 2D windows are flat
  // black, the 3D window has a dark-gray gradient. They only take effect when
  // the editor has not yet seeded the preferences node.
  static const std::array<const char*, 4> defaultFirstBackground = {"#000000", "#000000", "#000000", "#191919"};
  static const std::array<const char*, 4> defaultSecondBackground = {"#000000", "#000000", "#000000", "#7f7f7f"};
  static const std::array<const char*, 4> defaultDecoration = {"#c00000", "#00b000", "#0080ff", "#ffff00"};
  static const std::array<const char*, 4> defaultAnnotation = {"Axial", "Sagittal", "Coronal", "3D"};

  for (std::size_t i = 0; i < m_ColorWidgets.size(); ++i)
  {
    auto* colorWidget = m_ColorWidgets[i];
    const std::string widgetName = "stdmulti.widget" + std::to_string(i);

    const QColor upper(QString::fromStdString(prefs->Get(widgetName + " first background color", defaultFirstBackground[i])));
    const QColor lower(QString::fromStdString(prefs->Get(widgetName + " second background color", defaultSecondBackground[i])));
    const QColor decoration(QString::fromStdString(prefs->Get(widgetName + " decoration color", defaultDecoration[i])));
    const QString annotation = QString::fromStdString(prefs->Get(widgetName + " corner annotation", defaultAnnotation[i]));

    colorWidget->SetUpperColor(upper);
    colorWidget->SetLowerColor(lower);
    colorWidget->SetDecorationColor(decoration);
    colorWidget->SetAnnotation(annotation);
    colorWidget->SetGradientEnabled(upper != lower);
  }

  m_ColorWidgets[3]->SetLogoVisible(IsDepartmentLogoVisible(prefs));

  m_Ui->m_ShowLevelWindowWidget->setChecked(prefs->GetBool("Show level/window widget", true));
  m_Ui->m_PACSLikeMouseMode->setChecked(prefs->GetBool("PACS like mouse interaction", false));
  m_Ui->m_CrosshairGapSize->setValue(prefs->GetInt("crosshair gap size", 32));
  m_SyncCheckBox->setChecked(prefs->GetBool("sync 2D background colors", true));
}

void QmitkStdMultiWidgetEditorPreferencePage::ResetPreferencesAndGUI()
{
  auto* prefs = GetPreferences();
  prefs->Clear();

  this->Update();
  this->PerformOk();
}

void QmitkStdMultiWidgetEditorPreferencePage::OnBackgroundChanged(int sourceIndex)
{
  if (m_SyncCheckBox == nullptr || !m_SyncCheckBox->isChecked())
    return;

  const auto* source = m_ColorWidgets[sourceIndex];

  // Copy the background (both colors and the gradient state) to the other 2D
  // windows; the 3D window (index 3) is left untouched. The setters do not emit
  // BackgroundChanged, so this does not recurse.
  for (int i = 0; i < 3; ++i)
  {
    if (i == sourceIndex)
      continue;

    auto* target = m_ColorWidgets[i];
    target->SetUpperColor(source->GetUpperColor());
    target->SetLowerColor(source->GetLowerColor());
    target->SetGradientEnabled(source->IsGradientEnabled());
  }
}
