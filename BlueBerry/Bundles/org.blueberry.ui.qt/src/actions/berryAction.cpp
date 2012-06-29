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


#include "berryAction.h"

namespace berry {

class ActionPrivate
{
public:

  QString id;

};

Action::Action()
  : QAction(0), d(new ActionPrivate)
{}

Action::Action(const QString& text)
  : QAction(text, 0), d(new ActionPrivate)
{
}

Action::Action(const QString& text, const QIcon& image)
  : QAction(image, text, 0), d(new ActionPrivate)
{
}

}
