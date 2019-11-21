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


#ifndef QMITKFLOWBENCHAPPLICATIONPERSPECTIVE_H_
#define QMITKFLOWBENCHAPPLICATIONPERSPECTIVE_H_

#include <berryIPerspectiveFactory.h>

class QmitkFlowBenchApplicationPerspective : public QObject, public berry::IPerspectiveFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IPerspectiveFactory)

public:

  QmitkFlowBenchApplicationPerspective();

  void CreateInitialLayout(berry::IPageLayout::Pointer layout) override;

};

#endif /* QMITKFLOWBENCHAPPLICATIONPERSPECTIVE_H_ */
