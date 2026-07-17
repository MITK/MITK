/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryQHelpEngineWrapper.h"

namespace berry {

QHelpEngineWrapper::QHelpEngineWrapper(const QString &collectionFile)
  : QHelpEngine(collectionFile)
{
}

QHelpEngineWrapper::~QHelpEngineWrapper()
{
}

void QHelpEngineWrapper::initialDocSetupDone()
{
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
