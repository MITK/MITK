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


namespace berry {

class QHelpEngineWrapper : public QHelpEngine
{
  Q_OBJECT
  Q_DISABLE_COPY(QHelpEngineWrapper)

public:

  QHelpEngineWrapper(const QString &collectionFile);
  ~QHelpEngineWrapper() override;

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
