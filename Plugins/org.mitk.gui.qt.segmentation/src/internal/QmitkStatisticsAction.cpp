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
#include "QmitkStatisticsAction.h"

QmitkStatisticsAction::QmitkStatisticsAction(): m_BlueBerryView(NULL)
{
}

QmitkStatisticsAction::~QmitkStatisticsAction()
{
}

void QmitkStatisticsAction::Run(const QList<mitk::DataNode::Pointer>& /*selectedNodes*/)
{
  QSharedPointer<ctkPlugin> imageStatisticsBundle = berry::Platform::GetCTKPlugin("org.mitk.gui.qt.measurementtoolbox");

  if (m_BlueBerryView && !imageStatisticsBundle.isNull())
  {
    m_BlueBerryView->GetSite()->GetWorkbenchWindow()->GetActivePage()->ShowView("org.mitk.views.imagestatistics");
  }
}

void QmitkStatisticsAction::SetFunctionality(berry::QtViewPart* functionality)
{
  this->m_BlueBerryView = functionality;
}

void QmitkStatisticsAction::SetDataStorage(mitk::DataStorage* /*dataStorage*/)
{
  //not needed
}

void QmitkStatisticsAction::SetSmoothed(bool /*smoothed*/)
{
  //not needed
}

void QmitkStatisticsAction::SetDecimated(bool /*decimated*/)
{
  //not needed
}

