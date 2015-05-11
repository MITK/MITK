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

#ifndef SimpleRenderWindowView_H_
#define SimpleRenderWindowView_H_

#include <QmitkAbstractView.h>
#include <berryQtViewPart.h>
#include <mitkIRenderWindowPart.h>

class QmitkRenderWindow;
class AbstractRenderWindowViewPrivate;

/**
 * \brief A view class suited for the ViewerPerspective within the custom viewer plug-in.
 *
 * This view class contributes data node rendering functionality to the ViewerPerspective.
 * Being a subclass of QmitkAbstractView, this class yields access to the data storage and
 * thus is interconnected with the mitk::QmitkDataManagerView present in the same perspective.
 * As a subclass of mitk::IRenderWindowPart, this class provides an instance of QmitkRenderWindow.
 * A SimpleRenderWindowView instance is part of the ViewerPerspective for data visualization.
 */
// //! [SimpleRenderWindowViewDeclaration]
class SimpleRenderWindowView : public QmitkAbstractView, public mitk::IRenderWindowPart
// //! [SimpleRenderWindowViewDeclaration]
{
  Q_OBJECT

public:

  /**
   * Standard constructor.
   */
  SimpleRenderWindowView();

  ~SimpleRenderWindowView();

  /**
   *  String based view identifier.
   */
  static const std::string VIEW_ID;

  berryObjectMacro(SimpleRenderWindowView);

  // -------------------  mitk::IRenderWindowPart  ----------------------

  /**
   * \see mitk::IRenderWindowPart::GetActiveQmitkRenderWindow()
   */
  QmitkRenderWindow* GetActiveQmitkRenderWindow() const override;

  /**
   * \see mitk::IRenderWindowPart::GetRenderWindows()
   */
  QHash<QString,QmitkRenderWindow*> GetRenderWindows() const;

  /**
   * \see mitk::IRenderWindowPart::GetQmitkRenderWindows()
   */
  QHash<QString,QmitkRenderWindow*> GetQmitkRenderWindows() const override;

  /**
   * \see mitk::IRenderWindowPart::GetRenderWindow(QString)
   */
  QmitkRenderWindow* GetRenderWindow(const QString& id) const;

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

  /**
   * \see mitk::QmitkAbstractRenderEditor::GetRenderingManager()
   */
  mitk::IRenderingManager* GetRenderingManager() const override;

  /**
   * \see mitk::QmitkAbstractRenderEditor::RequestUpdate()
   */
  void RequestUpdate(mitk::RenderingManager::RequestType requestType = mitk::RenderingManager::REQUEST_UPDATE_ALL) override;

  /**
   * \see mitk::QmitkAbstractRenderEditor::ForceImmediateUpdate()
   */
  void ForceImmediateUpdate(mitk::RenderingManager::RequestType) override;

  /**
   * \see mitk::QmitkAbstractRenderEditor::GetTimeNavigationController()
   */
  mitk::SliceNavigationController *GetTimeNavigationController() const override;

protected:

  void SetFocus() override;

  /**
   * Creates the QmitkRenderWindow whose renderer is being connected to the view's data storage.
   */
  void CreateQtPartControl(QWidget* parent) override;

private:

  /**
   * The view's render window.
   */
  QmitkRenderWindow* m_RenderWindow;

  QScopedPointer<AbstractRenderWindowViewPrivate> d;

};

#endif /*SimpleRenderWindowView_H_*/
