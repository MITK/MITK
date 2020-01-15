/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKMXNMULTIWIDGETEDITOR_H
#define QMITKMXNMULTIWIDGETEDITOR_H

#include <org_mitk_gui_qt_mxnmultiwidgeteditor_Export.h>

#include <QmitkAbstractMultiWidgetEditor.h>

// c++
#include <memory>

class QmitkMxNMultiWidget;

class MXNMULTIWIDGETEDITOR_EXPORT QmitkMxNMultiWidgetEditor final : public QmitkAbstractMultiWidgetEditor
{
  Q_OBJECT

public:

  static const QString EDITOR_ID;

  QmitkMxNMultiWidgetEditor();
  ~QmitkMxNMultiWidgetEditor() override;

  virtual QmitkLevelWindowWidget* GetLevelWindowWidget() const override { return nullptr; }

  void OnLayoutSet(int row, int column);
  void OnInteractionSchemeChanged(mitk::InteractionSchemeSwitcher::InteractionScheme scheme);

private:
  /**
  * @brief Overridden from QmitkAbstractRenderEditor
  */
  void SetFocus() override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor
  */
  void CreateQtPartControl(QWidget* parent) override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor
  */
  void OnPreferencesChanged(const berry::IBerryPreferences* preferences) override;

  struct Impl;
  std::unique_ptr<Impl> m_Impl;

};

#endif // QMITKMXNMULTIWIDGETEDITOR_H
