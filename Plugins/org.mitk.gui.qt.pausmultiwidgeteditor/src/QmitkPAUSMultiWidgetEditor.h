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

#ifndef QmitkPAUSMultiWidgetEditor_h
#define QmitkPAUSMultiWidgetEditor_h

#include <QmitkAbstractRenderEditor.h>

#include <org_mitk_gui_qt_pausmultiwidgeteditor_Export.h>

class ORG_MITK_GUI_QT_PAUSMULTIWIDGETEDITOR QmitkPAUSMultiWidgetEditor
    : public QmitkAbstractRenderEditor
{
  Q_OBJECT

public:

  berryObjectMacro(QmitkPAUSMultiWidgetEditor)

  static const QString EDITOR_ID;

  QmitkPAUSMultiWidgetEditor();
  ~QmitkPAUSMultiWidgetEditor();

protected:

  void SetFocus() override;

  // -------------------  mitk::IRenderWindowPart  ----------------------

  /**
  * \see mitk::IRenderWindowPart::GetActiveQmitkRenderWindow()
  */
  QmitkRenderWindow* GetActiveQmitkRenderWindow() const override;

  /**
  * \see mitk::IRenderWindowPart::GetQmitkRenderWindows()
  */
  QHash<QString, QmitkRenderWindow*> GetQmitkRenderWindows() const override;

  /**
  * \see mitk::IRenderWindowPart::GetQmitkRenderWindow(QString)
  */
  QmitkRenderWindow* GetQmitkRenderWindow(const QString& id) const override;

  /**
  * \see mitk::IRenderWindowPart::GetSelectionPosition()
  */
  mitk::Point3D GetSelectedPosition(const QString& id = QString()) const override;

  /**
  * \see mitk::IRenderWindowPart::SetSelectedPosition()
  */
  void SetSelectedPosition(const mitk::Point3D& pos, const QString& id = QString()) override;

  /**
  * \see mitk::IRenderWindowPart::EnableDecorations()
  */
  void EnableDecorations(bool enable, const QStringList& decorations = QStringList()) override;

  /**
  * \see mitk::IRenderWindowPart::IsDecorationEnabled()
  */
  bool IsDecorationEnabled(const QString& decoration) const override;

  /**
  * \see mitk::IRenderWindowPart::GetDecorations()
  */
  QStringList GetDecorations() const override;


  QHash<QString, QmitkRenderWindow*> m_RenderWindows;


  void CreateQtPartControl(QWidget* parent) override;

};
#endif /*QmitkPAUSMultiWidgetEditor_h*/
