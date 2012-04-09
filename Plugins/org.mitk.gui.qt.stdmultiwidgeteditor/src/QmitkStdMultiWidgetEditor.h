/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QMITKSTDMULTIWIDGETEDITOR_H_
#define QMITKSTDMULTIWIDGETEDITOR_H_

#include <QmitkAbstractRenderEditor.h>

#include <mitkILinkedRenderWindowPart.h>

#include <org_mitk_gui_qt_stdmultiwidgeteditor_Export.h>

class QmitkStdMultiWidget;
class QmitkMouseModeSwitcher;
class QmitkStdMultiWidgetEditorPrivate;

/**
 * \ingroup org_mitk_gui_qt_stdmultiwidgeteditor
 */
class ORG_MITK_GUI_QT_STDMULTIWIDGETEDITOR QmitkStdMultiWidgetEditor
    : public QmitkAbstractRenderEditor, public mitk::ILinkedRenderWindowPart
{
  Q_OBJECT

public:

  berryObjectMacro(QmitkStdMultiWidgetEditor)

  static const std::string EDITOR_ID;

  QmitkStdMultiWidgetEditor();
  ~QmitkStdMultiWidgetEditor();

  QmitkStdMultiWidget* GetStdMultiWidget();

  // -------------------  mitk::IRenderWindowPart  ----------------------

  /**
   * \see mitk::IRenderWindowPart::GetActiveRenderWindow()
   */
  QmitkRenderWindow* GetActiveRenderWindow() const;

  /**
   * \see mitk::IRenderWindowPart::GetRenderWindows()
   */
  QHash<QString,QmitkRenderWindow*> GetRenderWindows() const;

  /**
   * \see mitk::IRenderWindowPart::GetRenderWindow(QString)
   */
  QmitkRenderWindow* GetRenderWindow(const QString& id) const;

  /**
   * \see mitk::IRenderWindowPart::GetSelectionPosition()
   */
  mitk::Point3D GetSelectedPosition(const QString& id = QString()) const;

  /**
   * \see mitk::IRenderWindowPart::SetSelectedPosition()
   */
  void SetSelectedPosition(const mitk::Point3D& pos, const QString& id = QString());

  /**
   * \see mitk::IRenderWindowPart::EnableDecorations()
   */
  void EnableDecorations(bool enable, const QStringList& decorations = QStringList());

  /**
   * \see mitk::IRenderWindowPart::IsDecorationEnabled()
   */
  bool IsDecorationEnabled(const QString& decoration) const;

  /**
   * \see mitk::IRenderWindowPart::GetDecorations()
   */
  QStringList GetDecorations() const;

  // -------------------  mitk::ILinkedRenderWindowPart  ----------------------

  mitk::SlicesRotator* GetSlicesRotator() const;
  mitk::SlicesSwiveller* GetSlicesSwiveller() const;

  void EnableSlicingPlanes(bool enable);
  bool IsSlicingPlanesEnabled() const;

  void EnableLinkedNavigation(bool enable);
  bool IsLinkedNavigationEnabled() const;

protected:

  void SetFocus();

  void OnPreferencesChanged(const berry::IBerryPreferences*);

  void CreateQtPartControl(QWidget* parent);

private:

  const QScopedPointer<QmitkStdMultiWidgetEditorPrivate> d;

};

#endif /*QMITKSTDMULTIWIDGETEDITOR_H_*/
