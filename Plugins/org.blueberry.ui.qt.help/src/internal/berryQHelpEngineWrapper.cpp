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

#include "berryQHelpEngineWrapper.h"

#include <QHelpSearchEngine>


namespace berry {

QHelpEngineWrapper::QHelpEngineWrapper(const QString &collectionFile)
  : QHelpEngine(collectionFile)
{
  /*
   * Otherwise we will waste time if several new docs are found,
   * because we will start to index them, only to be interrupted
   * by the next request. Also, there is a nasty SQLITE bug that will
   * cause the application to hang for minutes in that case.
   * This call is reverted by initalDocSetupDone(), which must be
   * called after the new docs have been installed.
   */
  disconnect(this, SIGNAL(setupFinished()),
             searchEngine(), SLOT(indexDocumentation()));
}

QHelpEngineWrapper::~QHelpEngineWrapper()
{
}

void QHelpEngineWrapper::initialDocSetupDone()
{
  connect(this, SIGNAL(setupFinished()),
          searchEngine(), SLOT(indexDocumentation()));
  setupData();
}

const QString QHelpEngineWrapper::homePage() const
{
  return m_HomePage;
}

void QHelpEngineWrapper::setHomePage(const QString &page)
{
  if (m_HomePage != page)
  {
    m_HomePage = page;
    emit homePageChanged(page);
  }
}

} // end namespace berry
