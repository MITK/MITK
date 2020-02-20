/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYINTROHANDLER_H
#define BERRYINTROHANDLER_H

#include <berryAbstractHandler.h>

namespace berry {

class IntroDescriptor;
class Workbench;

class IntroHandler : public AbstractHandler
{
  Q_OBJECT

private:

  Workbench* workbench;
  SmartPointer<IntroDescriptor> introDescriptor;

public:

  IntroHandler();
  ~IntroHandler() override;

  SmartPointer<Object> Execute(const SmartPointer<const ExecutionEvent>& event) override;

  bool IsEnabled() const override;

};

}

#endif // BERRYINTROHANDLER_H
