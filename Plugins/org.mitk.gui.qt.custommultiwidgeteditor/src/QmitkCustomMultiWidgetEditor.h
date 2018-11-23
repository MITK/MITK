/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKCUSTOMMULTIWIDGETEDITOR_H
#define QMITKCUSTOMMULTIWIDGETEDITOR_H

#include <QmitkAbstractMultiWidgetEditor.h>

// custom multi widget editor
#include <org_mitk_gui_qt_custommultiwidgeteditor_Export.h>

// mitk core
#include <mitkSliceNavigationController.h>

// c++
#include <memory>

class CUSTOMMULTIWIDGETEDITOR_EXPORT QmitkCustomMultiWidgetEditor final : public QmitkAbstractMultiWidgetEditor
{
  Q_OBJECT

public:

  using ViewDirection = mitk::SliceNavigationController::ViewDirection;

  static const QString EDITOR_ID;

  QmitkCustomMultiWidgetEditor();
  ~QmitkCustomMultiWidgetEditor();

private Q_SLOTS:

  void OnViewDirectionChanged(ViewDirection viewDirection);

private:
  /**
  * @brief Overridden from QmitkAbstractRenderEditor
  */
  virtual void SetFocus() override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor : QtEditorPart
  */
  virtual void CreateQtPartControl(QWidget* parent) override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor
  */
  virtual void OnPreferencesChanged(const berry::IBerryPreferences* preferences) override;

  class Impl;
  const std::unique_ptr<Impl> m_Impl;
};

#endif // QMITKCUSTOMMULTIWIDGETEDITOR_H
