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


#ifndef BERRYQTGLOBALEVENTFILTER_H_
#define BERRYQTGLOBALEVENTFILTER_H_

#include <QObject>

class QEvent;

namespace berry {

class QtGlobalEventFilter : public QObject
{
  Q_OBJECT

public:

  QtGlobalEventFilter(QObject* parent = 0);

  bool eventFilter(QObject* obj, QEvent* event);
};

}

#endif /* BERRYQTGLOBALEVENTFILTER_H_ */
