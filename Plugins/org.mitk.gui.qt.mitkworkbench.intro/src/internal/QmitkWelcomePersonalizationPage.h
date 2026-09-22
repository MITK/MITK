/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkWelcomePersonalizationPage_h
#define QmitkWelcomePersonalizationPage_h

#include <QmitkToolBarPresets.h>

#include <QWidget>

#include <vector>

class QButtonGroup;
class QLabel;

namespace mitk
{
  class IPreferences;
}

/**
 * \brief Welcome screen page for the most common application settings.
 *
 * Offers the plugin preset, the theme, and the mouse interaction scheme.
 * Choices take effect immediately and are stored in the same preferences as
 * the corresponding preference pages, so changes made in either place are
 * reflected in the other.
 */
class QmitkWelcomePersonalizationPage : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkWelcomePersonalizationPage(QWidget* parent = nullptr);
  ~QmitkWelcomePersonalizationPage() override;

private:
  QWidget* CreatePresetCard();
  QWidget* CreateThemeCard();
  QWidget* CreateMouseInteractionCard();

  void OnPresetClicked(int id);
  void OnThemeClicked(int id);
  void OnMouseInteractionClicked(int id);

  void OnToolBarPreferencesChanged(const mitk::IPreferences*);
  void OnEditorPreferencesChanged(const mitk::IPreferences*);

  int GetMatchingPresetId() const;

  void UpdatePreset();
  void UpdateTheme();
  void UpdateMouseInteraction();

  std::vector<QmitkToolBarPreset> m_Presets;
  QButtonGroup* m_PresetGroup;
  QLabel* m_PresetInfo;
  QButtonGroup* m_ThemeGroup;
  QButtonGroup* m_MouseInteractionGroup;
  bool m_IsApplyingPreset;
  bool m_IsCustomPresetChosen;
};

#endif
