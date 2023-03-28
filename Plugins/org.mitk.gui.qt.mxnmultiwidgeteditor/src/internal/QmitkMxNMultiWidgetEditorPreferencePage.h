/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxNMultiWidgetEditorPreferencePage_h
#define QmitkMxNMultiWidgetEditorPreferencePage_h

#include "ui_QmitkMxNMultiWidgetEditorPreferencePage.h"

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

private:

  Ui::QmitkMxNMultiWidgetEditorPreferencePage m_Ui;
  QWidget* m_MainControl;

};

#endif
