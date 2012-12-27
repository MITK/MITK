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


#ifndef BERRYQTDNDTWEAKLET_H_
#define BERRYQTDNDTWEAKLET_H_

#include <berryDnDTweaklet.h>

namespace berry {

class QtDnDTweaklet : public QObject, public DnDTweaklet
{
  Q_OBJECT
  Q_INTERFACES(berry::DnDTweaklet)

public:

  QtDnDTweaklet();
  QtDnDTweaklet(const QtDnDTweaklet& other);

  ITracker* CreateTracker();
};

}

#endif /* BERRYQTDNDTWEAKLET_H_ */
