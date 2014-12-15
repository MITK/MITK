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

protected:
  std::string m_CurrentRenderingMode;

  std::string m_FirstColor;
  std::string m_SecondColor;
  std::string m_Widget1Color;
  QString m_FirstColorStyleSheet;
  QString m_SecondColorStyleSheet;
  QString m_Widget1ColorStyleSheet;
  berry::IPreferences::Pointer m_Preferences;

private:
  QScopedPointer<Ui::QmitkStdMultiWidgetEditorPreferencePage> m_Ui;
  QWidget* m_Control;
};

#endif
