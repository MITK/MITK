/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkVisualizationPerspective_h
#define QmitkVisualizationPerspective_h

#include <berryIPerspectiveFactory.h>

class QmitkVisualizationPerspective : public QObject, public berry::IPerspectiveFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IPerspectiveFactory)

public:

  QmitkVisualizationPerspective() {}
  ~QmitkVisualizationPerspective() override {}

  void CreateInitialLayout(berry::IPageLayout::Pointer layout) override;
};

#endif
