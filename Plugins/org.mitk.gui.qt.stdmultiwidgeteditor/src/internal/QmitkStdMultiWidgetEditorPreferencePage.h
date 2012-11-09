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


#ifndef QMITKSTDMULTIWIDGETEDITORPREFERENCEPAGE_H_
#define QMITKSTDMULTIWIDGETEDITORPREFERENCEPAGE_H_

#include "berryIQtPreferencePage.h"
#include <berryIPreferences.h>

class QWidget;
class QCheckBox;
class QPushButton;
class QWidgetAction;

struct QmitkStdMultiWidgetEditorPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  QmitkStdMultiWidgetEditorPreferencePage();

  void Init(berry::IWorkbench::Pointer workbench);

  void CreateQtControl(QWidget* widget);

  QWidget* GetQtControl() const;

  ///
  /// \see IPreferencePage::PerformOk()
  ///
  virtual bool PerformOk();

  ///
  /// \see IPreferencePage::PerformCancel()
  ///
  virtual void PerformCancel();

  ///
  /// \see IPreferencePage::Update()
  ///
  virtual void Update();

public slots:
  void FirstColorChanged();

  void SecondColorChanged();

  void UseGradientBackgroundSelected();

  void ColorActionChanged();

  void ResetColors();

protected:
  QWidget* m_MainControl;
  QCheckBox* m_EnableFlexibleZooming;
  QCheckBox* m_ShowLevelWindowWidget;
  QCheckBox* m_UseGradientBackground;
  QCheckBox* m_ChangeBackgroundColors;
  QCheckBox* m_PACSLikeMouseMode;
  QPushButton* m_ColorButton1;
  QPushButton* m_ColorButton2;
  std::string m_FirstColor;
  std::string m_SecondColor;
  QString m_FirstColorStyleSheet;
  QString m_SecondColorStyleSheet;
  berry::IPreferences::Pointer m_StdMultiWidgetEditorPreferencesNode;
};

#endif /* QMITKDATAMANAGERPREFERENCEPAGE_H_ */
