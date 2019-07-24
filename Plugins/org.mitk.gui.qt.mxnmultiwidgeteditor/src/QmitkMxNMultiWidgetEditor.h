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

#ifndef QMITKMXNMULTIWIDGETEDITOR_H
#define QMITKMXNMULTIWIDGETEDITOR_H

#include <QmitkAbstractMultiWidgetEditor.h>

#include <org_mitk_gui_qt_mxnmultiwidgeteditor_Export.h>

// berry
#include <berryIPartListener.h>

// c++
#include <memory>

class QmitkMxNMultiWidget;

class MXNMULTIWIDGETEDITOR_EXPORT QmitkMxNMultiWidgetEditor final : public QmitkAbstractMultiWidgetEditor
{
  Q_OBJECT

public:

  static const QString EDITOR_ID;

  QmitkMxNMultiWidgetEditor();
  virtual ~QmitkMxNMultiWidgetEditor() override;

  virtual void OnLayoutSet(int row, int column) override;

private:
  /**
  * @brief Overridden from QmitkAbstractRenderEditor
  */
  virtual void SetFocus() override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor
  */
  virtual void CreateQtPartControl(QWidget* parent) override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor
  */
  virtual void OnPreferencesChanged(const berry::IBerryPreferences* preferences) override;

  struct Impl;
  std::unique_ptr<Impl> m_Impl;

};

#endif // QMITKMXNMULTIWIDGETEDITOR_H
