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

#ifndef SimpleRenderWindowView_H_
#define SimpleRenderWindowView_H_

#include <QmitkAbstractView.h>
#include <berryQtViewPart.h>
#include <mitkIRenderWindowPart.h>

class QmitkRenderWindow;
class AbstractRenderWindowViewPrivate;

class SimpleRenderWindowView : public QmitkAbstractView, public mitk::IRenderWindowPart
{
  Q_OBJECT

public:

  berryObjectMacro(SimpleRenderWindowView)

  static const std::string VIEW_ID;

  SimpleRenderWindowView();


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

  mitk::IRenderingManager* GetRenderingManager() const;

  void RequestUpdate(mitk::RenderingManager::RequestType requestType = mitk::RenderingManager::REQUEST_UPDATE_ALL);

  void ForceImmediateUpdate(mitk::RenderingManager::RequestType);

  mitk::SliceNavigationController *GetTimeNavigationController() const;

protected:

  void SetFocus();

  void CreateQtPartControl(QWidget* parent);

private:

  QmitkRenderWindow* m_RenderWindow;

  QScopedPointer<AbstractRenderWindowViewPrivate> d;

};

#endif /*SimpleRenderWindowView_H_*/
