/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkStdMultiWidgetEditorPreferencePage_h
#define QmitkStdMultiWidgetEditorPreferencePage_h

#include <berryIQtPreferencePage.h>

#include <array>
#include <memory>

namespace Ui
{
  class QmitkStdMultiWidgetEditorPreferencePage;
}

class QCheckBox;
class QmitkRenderWindowColorWidget;

class QmitkStdMultiWidgetEditorPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  QmitkStdMultiWidgetEditorPreferencePage();
  ~QmitkStdMultiWidgetEditorPreferencePage() override;

  void CreateQtControl(QWidget* parent) override;
  QWidget* GetQtControl() const override;
  void Init(berry::IWorkbench::Pointer) override;
  void PerformCancel() override;
  bool PerformOk() override;
  void Update() override;

public slots:
  /**
   * @brief Clear all preferences of this page and refresh the GUI with the defaults.
   */
  void ResetPreferencesAndGUI();

private:
  /** @brief When sync is enabled, copy the changed 2D window's background to the other 2D windows. */
  void OnBackgroundChanged(int sourceIndex);

  std::unique_ptr<Ui::QmitkStdMultiWidgetEditorPreferencePage> m_Ui;

  /** @brief One color editor per render window, ordered widget0..widget3. */
  std::array<QmitkRenderWindowColorWidget*, 4> m_ColorWidgets;

  /** @brief Toggles whether background changes propagate across the 2D windows (widget0..2). */
  QCheckBox* m_SyncCheckBox;

  QWidget* m_Control;
};

#endif
