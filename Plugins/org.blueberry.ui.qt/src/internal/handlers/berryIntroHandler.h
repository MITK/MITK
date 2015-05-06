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
  ~IntroHandler();

  SmartPointer<Object> Execute(const SmartPointer<const ExecutionEvent>& event);

  bool IsEnabled() const;

};

}

#endif // BERRYINTROHANDLER_H
