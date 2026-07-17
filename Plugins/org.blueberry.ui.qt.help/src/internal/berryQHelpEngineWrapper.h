/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYQHELPENGINEWRAPPER_H
#define BERRYQHELPENGINEWRAPPER_H

#include <QHelpEngine>
#include <QUrl>


namespace berry {

class QHelpEngineWrapper : public QHelpEngine
{
  Q_OBJECT
  Q_DISABLE_COPY(QHelpEngineWrapper)

public:

  QHelpEngineWrapper(const QString &collectionFile);
  ~QHelpEngineWrapper() override;

  /*
   * To be called after the initial search for qch files finished, so
   * the index and contents are set up for the registered documentation.
   */
  void initialDocSetupDone();

  const QString homePage() const;
  void setHomePage(const QString &page);

Q_SIGNALS:

  void homePageChanged(const QString& page);

  /*
   * Emitted whenever the help editor navigates to a page (link, history
   * or context help), so views can sync to the currently shown page.
   */
  void currentPageChanged(const QUrl& url);

private:

  QString m_HomePage;

};

} // end namespace berry

#endif // BERRYQHELPENGINEWRAPPER_H
