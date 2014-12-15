/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QmitkStdMultiWidgetEditorPreferencePage_h
#define QmitkStdMultiWidgetEditorPreferencePage_h

#include <berryIPreferences.h>
#include <berryIQtPreferencePage.h>
#include <QProcess>
#include <QScopedPointer>
#include <QPushButton>

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
  ~QmitkStdMultiWidgetEditorPreferencePage();

  void CreateQtControl(QWidget* parent);
  QWidget* GetQtControl() const;
  void Init(berry::IWorkbench::Pointer);
  void PerformCancel();
  bool PerformOk();
  void Update();

public slots:
  /**
   * @brief ResetColors set default gradient background colors.
   */
  void ResetColors();

  /**
   * @brief ChangeRenderingMode slot to chose the rendering mode via QComboBox.
   * @param i index of the box.
   */
  void ChangeRenderingMode(int i);

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
   * @brief m_CurrentRenderingMode String for the rendering mode.
   */
  std::string m_CurrentRenderingMode;

  /**
   * @brief m_WidgetBackgroundColor1 the background colors.
   *
   * If two different colors are chosen, a gradient background appears.
   */
  std::string m_WidgetBackgroundColor1[4];
  std::string m_WidgetBackgroundColor2[4];

  /**
   * @brief m_WidgetDecorationColor the decoration color.
   *
   * The rectangle prop, the crosshair, the 3D planes and the corner annotation use this.
   */
  std::string m_WidgetDecorationColor[4];

  /**
   * @brief m_Widget1Annotation the text of the corner annotation.
   */
  std::string m_WidgetAnnotation[4];

  /**
   * @brief m_Preferences the berry preferences.
   */
  berry::IPreferences::Pointer m_Preferences;

  /**
   * @brief SetStyleSheetToColorChooserButton colorize a button.
   * @param backgroundcolor color for the button.
   * @param button the button.
   */
  void SetStyleSheetToColorChooserButton(QColor backgroundcolor, QPushButton* button);

  /**
   * @brief StringToColor convert a hexadecimal std::string to QColor.
   * @param colorInHex string in the form of "#123456" where each digit is a hex value (0-F).
   * @return color in Qt format.
   */
  QColor HexStringToQtColor(std::string colorInHex);

protected slots:

  /**
   * @brief ColorChooserButtonClicked slot called when a button to choose color was clicked.
   */
  void ColorChooserButtonClicked();

private:
  QScopedPointer<Ui::QmitkStdMultiWidgetEditorPreferencePage> m_Ui;
  QWidget* m_Control;
};
#endif //QmitkStdMultiWidgetEditorPreferencePage_h
