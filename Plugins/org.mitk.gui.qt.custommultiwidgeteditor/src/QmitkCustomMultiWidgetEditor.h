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

#include <QmitkAbstractRenderEditor.h>
#include <mitkILinkedRenderWindowPart.h>

// custom multi widget editor
#include <org_mitk_gui_qt_custommultiwidgeteditor_Export.h>

#include <memory>

class QmitkCustomMultiWidget;

class CUSTOMMULTIWIDGETEDITOR_EXPORT QmitkCustomMultiWidgetEditor final : public QmitkAbstractRenderEditor
{
  Q_OBJECT

public:

  berryObjectMacro(QmitkCustomMultiWidgetEditor)

  static const QString EDITOR_ID;

  QmitkCustomMultiWidgetEditor();
  ~QmitkCustomMultiWidgetEditor();

  /**
  * @brief Overridden from QmitkAbstractRenderEditor  : IRenderWindowPart
  */
  virtual QmitkRenderWindow* GetActiveQmitkRenderWindow() const override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor  : IRenderWindowPart
  */
  virtual QHash<QString, QmitkRenderWindow*> GetQmitkRenderWindows() const override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor  : IRenderWindowPart
  */
  virtual QmitkRenderWindow* GetQmitkRenderWindow(const QString& id) const override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor  : IRenderWindowPart
  */
  virtual mitk::Point3D GetSelectedPosition(const QString& id = QString()) const override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor  : IRenderWindowPart
  */
  virtual void SetSelectedPosition(const mitk::Point3D& pos, const QString& id = QString()) override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor  : IRenderWindowPart
  */
  virtual void EnableDecorations(bool enable, const QStringList& decorations = QStringList()) override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor  : IRenderWindowPart
  */
  virtual bool IsDecorationEnabled(const QString& decoration) const override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor  : IRenderWindowPart
  */
  virtual QStringList GetDecorations() const override;
  /**
  * @brief Return the current custom multi widget of this editor.
  */
  QmitkCustomMultiWidget* GetCustomMultiWidget();

private Q_SLOTS:

  void OnLayoutSet(int row, int column);
  void OnSynchronize(bool synchronized);

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

  class Impl;
  const std::unique_ptr<Impl> m_Impl;
};

#endif // QMITKCUSTOMMULTIWIDGETEDITOR_H
