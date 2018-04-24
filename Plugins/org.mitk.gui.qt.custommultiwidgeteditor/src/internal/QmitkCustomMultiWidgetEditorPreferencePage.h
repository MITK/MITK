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

#ifndef QMITKCUSTOMMULTIWIDGETEDITORPREFERENCEPAGE_H
#define QMITKCUSTOMMULTIWIDGETEDITORPREFERENCEPAGE_H

// custom multi widget editor
#include "ui_QmitkCustomMultiWidgetEditorPreferencePage.h"

#include <berryIPreferences.h>
#include <berryIQtPreferencePage.h>
#include <QProcess>
#include <QScopedPointer>
#include <QPushButton>

class QmitkCustomMultiWidgetEditorPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  QmitkCustomMultiWidgetEditorPreferencePage();
  ~QmitkCustomMultiWidgetEditorPreferencePage();

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

  /**
   * @brief ChangeRenderingMode slot to chose the rendering mode via QComboBox.
   * @param i index of the box.
   */
  void ChangeRenderingMode(int i);

  void ChangeColormap(int i);

protected:
  /**
   * @brief m_CurrentRenderingMode String for the rendering mode.
   */
  std::string m_CurrentRenderingMode;

  std::string m_CurrentColormap;

  /**
   * @brief m_Preferences the berry preferences.
   */
  berry::IPreferences::Pointer m_Preferences;

private:

  Ui::QmitkCustomMultiWidgetEditorPreferencePage m_Ui;
  QWidget* m_MainControl;

};

#endif // QMITKCUSTOMMULTIWIDGETEDITORPREFERENCEPAGE_H
