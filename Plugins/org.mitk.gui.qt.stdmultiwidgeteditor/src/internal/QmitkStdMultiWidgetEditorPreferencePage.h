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
#include <QScopedPointer>
#include <QString>

#include <array>

class QPushButton;

namespace Ui
{
  class QmitkStdMultiWidgetEditorPreferencePage;
}

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
   * @brief ResetColors set default colors and refresh the GUI.
   */
  void ResetPreferencesAndGUI();

  /**
   * @brief OnWidgetComboBoxChanged slot called when the QComboBox to chose the widget was modified.
   * @param i index of the combobox to select the widget (1-4).
   */
  void OnWidgetComboBoxChanged(int i);

  /**
   * @brief AnnotationTextChanged called when QLineEdit for the annotation was changed.
   * @param text The new text.
   */
  void AnnotationTextChanged(QString text);

protected:
  /**
   * @brief m_WidgetBackgroundColor1 the background colors.
   *
   * If two different colors are chosen, a gradient background appears.
   */
  std::array<QString, 4> m_WidgetBackgroundColor1;
  std::array<QString, 4> m_WidgetBackgroundColor2;

  /**
   * @brief m_WidgetDecorationColor the decoration color.
   *
   * The rectangle prop, the crosshair, the 3D planes and the corner annotation use this.
   */
  std::array<QString, 4> m_WidgetDecorationColor;

  /**
   * @brief m_Widget1Annotation the text of the corner annotation.
   */
  std::array<QString, 4> m_WidgetAnnotation;

  /**
   * @brief SetStyleSheetToColorChooserButton colorize a button.
   * @param backgroundcolor color for the button.
   * @param button the button.
   */
  void SetStyleSheetToColorChooserButton(QColor backgroundcolor, QPushButton* button);

protected slots:

  /**
   * @brief ColorChooserButtonClicked slot called when a button to choose color was clicked.
   */
  void ColorChooserButtonClicked();

private:
  QScopedPointer<Ui::QmitkStdMultiWidgetEditorPreferencePage> m_Ui;
  QWidget* m_Control;
};
#endif
