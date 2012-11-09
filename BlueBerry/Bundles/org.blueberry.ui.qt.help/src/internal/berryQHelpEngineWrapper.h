/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYQHELPENGINEWRAPPER_H
#define BERRYQHELPENGINEWRAPPER_H

#include <QHelpEngine>


namespace berry {

class QHelpEngineWrapper : public QHelpEngine
{
  Q_OBJECT
  Q_DISABLE_COPY(QHelpEngineWrapper)

public:

  QHelpEngineWrapper(const QString &collectionFile);
  ~QHelpEngineWrapper();

  /*
   * To be called after the initial search for qch files finished.
   * This will mainly cause the search index to be updated, if necessary.
   */
  void initialDocSetupDone();

  const QString homePage() const;
  void setHomePage(const QString &page);

Q_SIGNALS:

  void homePageChanged(const QString& page);

private:

  QString m_HomePage;

};

} // end namespace berry

#endif // BERRYQHELPENGINEWRAPPER_H
