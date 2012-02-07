/*=========================================================================

 Program:   BlueBerry Platform
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
