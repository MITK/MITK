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

#include <QWebPage>

#ifndef QMITK_WEBPAGE_H
#define QMITK_WEBPAGE_H


/**
 * @brief The QmitkWebPage class
 *
 * This class reimplements QWebPage as a fix for bug 16158.
 */
class QmitkJSWebPage : public QWebPage
{
  Q_OBJECT

public:
  /**
  * @brief Constructor
  */
  QmitkJSWebPage(QObject *parent=0);

  /**
  * @brief Destructor
  */
  virtual  ~QmitkJSWebPage();

public slots:
  /**
   * @brief shouldInterruptJavaScript
   *
   *  Overwritten from QWebView, to stop asking Qt to stop the JavaScript.
   * @return false so JavaScript is not stopped
   */
  bool shouldInterruptJavaScript();
};

#endif

