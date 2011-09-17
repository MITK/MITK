/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


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
