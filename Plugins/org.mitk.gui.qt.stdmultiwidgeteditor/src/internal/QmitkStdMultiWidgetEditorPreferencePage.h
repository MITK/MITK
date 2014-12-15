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
  void FirstColorChanged();

  void SecondColorChanged();

  void ResetColors();

  void ChangeRenderingMode(int i);

  void WidgetColorChanged();

  void OnWidgetComboBoxChanged(int i);

  void AnnotationTextChanged(QString text);

protected:
  std::string m_CurrentRenderingMode;

  std::string m_FirstColor;
  std::string m_SecondColor;
  std::string m_Widget1Color;
  std::string m_Widget2Color;
  std::string m_Widget3Color;
  std::string m_Widget4Color;
  std::string m_Widget1Annotation;
  std::string m_Widget2Annotation;
  std::string m_Widget3Annotation;
  std::string m_Widget4Annotation;

  berry::IPreferences::Pointer m_Preferences;

  void SetStyleSheetToColorChooserButton(QColor backgroundcolor, QPushButton* button);
  QColor StringToColor(std::string colorInHex);

private:
  QScopedPointer<Ui::QmitkStdMultiWidgetEditorPreferencePage> m_Ui;
  QWidget* m_Control;
};

#endif
