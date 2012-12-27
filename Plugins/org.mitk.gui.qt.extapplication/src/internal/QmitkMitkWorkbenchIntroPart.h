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


#ifndef QMITKWORKBENCHINTROPART_H_
#define QMITKWORKBENCHINTROPART_H_

#include <QtCore/qconfig.h>

#include <berryQtIntroPart.h>
#include <ui_QmitkWelcomeScreenViewControls.h>


/**
 * \ingroup org_mitk_gui_qt_welcomescreen_internal
 * \brief QmitkMitkWorkbenchIntroPart
 * The WelcomeView Module is an helpful feature to people new to MITK. The main idea is to provide first
 * information about the MITK Workbench.
 * The WelcomeView is realized by making use of the QTWebKit Module. The Qt WebKit module
 * provides an HTML browser engine that makes it easy to embed web content into native applications, and to enhance
 * web content with native controls.
 * For the welcome view of the application the QWebView, QWebPage classes have been used. The shown WelcomeView
 * html start page is styled by an external css stylesheet. The required resources as well as the html pages are integrated
 * into the QtResource system. The QT resource system allows the storage of files like html pages, css pages, jpgs etc.
 * as binaries within the executable.
 * This minimizes the risk of loosing resource files as well as the risk of getting files deleted. In order to use the Qt
 * resource system the resource files have to be added to the associated qrt resource file list.
 *
 * The foundation is set to design more complex html pages. The Q::WebPage gives options to set a
 * LinkDelegationPolicy. The used policy defines how links to external or internal resources are handled. To fit our needs
 * the delegate all links policy is used. This requires all external as well as internal links of the html pages to be handle
 * explicitly. In order to change mitk working modes (perspectives) a mitk url scheme has been designed. The url scheme
 * is set to mitk. The url host provides information about what's next to do. In our case, the case of switching to a
 * particular working mode the host is set to perspectives. The followed path provides information about the perspective id.
 * (e.g. mitk//::mitk.perspectives/org.mitk.qt.defaultperspective) The the generic design of the mitk url scheme allows to
 * execute other task depending on the mitk url host.
 * \sa QmitkWelcomePage Editor
 */

class QWebView ;

class QmitkMitkWorkbenchIntroPart : public berry::QtIntroPart
{

// this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

public:

  QmitkMitkWorkbenchIntroPart();
  QmitkMitkWorkbenchIntroPart(const QmitkMitkWorkbenchIntroPart& other)
  {
    Q_UNUSED(other)
    throw std::runtime_error("Copy constructor not implemented");
  }
 ~QmitkMitkWorkbenchIntroPart();


  virtual void CreateQtPartControl(QWidget *parent);

  void StandbyStateChanged(bool standby);

  void SetFocus();
#ifdef QT_WEBKIT

  virtual void CreateConnections();


protected slots:


  void DelegateMeTo(const QUrl& ShowMeNext);
#endif
protected:

  Ui::QmitkWelcomeScreenViewControls* m_Controls;
  QWebView* m_view;
};

#endif /* QMITKWORKBENCHINTROPART_H_ */
