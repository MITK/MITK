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


#ifndef QMITKFLOWSEGMENTATIONPERSPECTIVE_H_
#define QMITKFLOWSEGMENTATIONPERSPECTIVE_H_

#include <berryIPerspectiveFactory.h>

class QmitkFlowSegmentationPerspective : public QObject, public berry::IPerspectiveFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IPerspectiveFactory)

public:

  QmitkFlowSegmentationPerspective();

  void CreateInitialLayout(berry::IPageLayout::Pointer layout) override;

};

#endif /* QMITKFLOWSEGMENTATIONPERSPECTIVE_H_ */
