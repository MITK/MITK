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


#ifndef BERRYMMMENULISTENER_H
#define BERRYMMMENULISTENER_H

#include <QObject>

namespace berry
{

class MenuManager;

class MMMenuListener : public QObject
{
  Q_OBJECT

  berry::MenuManager* mm;

public:

  MMMenuListener(berry::MenuManager* mm);

private:

  Q_SLOT void HandleAboutToShow();
};

}

#endif // BERRYMMMENULISTENER_H
