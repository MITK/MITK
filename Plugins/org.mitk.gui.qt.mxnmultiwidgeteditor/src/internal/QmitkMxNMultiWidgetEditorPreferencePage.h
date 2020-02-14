/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKMXNMULTIWIDGETEDITORPREFERENCEPAGE_H
#define QMITKMXNMULTIWIDGETEDITORPREFERENCEPAGE_H

#include "ui_QmitkMxNMultiWidgetEditorPreferencePage.h"

#include <berryIPreferences.h>
#include <berryIQtPreferencePage.h>
#include <QProcess>
#include <QScopedPointer>
#include <QPushButton>

class QmitkMxNMultiWidgetEditorPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:

  QmitkMxNMultiWidgetEditorPreferencePage();
  ~QmitkMxNMultiWidgetEditorPreferencePage() override;

  void Init(berry::IWorkbench::Pointer) override;
  void CreateQtControl(QWidget* parent) override;
  QWidget* GetQtControl() const override;

  bool PerformOk() override;
  void PerformCancel() override;
  void Update() override;

public slots:
  /**
   * @brief ResetColors set default colors and refresh the GUI.
   */
  void ResetPreferencesAndGUI();

  void ChangeColormap(int i);

protected:
  std::string m_CurrentColormap;

  /**
   * @brief m_Preferences the berry preferences.
   */
  berry::IPreferences::Pointer m_Preferences;

private:

  Ui::QmitkMxNMultiWidgetEditorPreferencePage m_Ui;
  QWidget* m_MainControl;

};

#endif // QMITKMXNMULTIWIDGETEDITORPREFERENCEPAGE_H
