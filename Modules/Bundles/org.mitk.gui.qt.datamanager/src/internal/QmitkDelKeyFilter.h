/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QMITKDELKEYFILTER_H_
#define QMITKDELKEYFILTER_H_

#include <QObject>

///
/// A small class which "eats" all Del-Key-pressed events on the node table.
/// When the Del Key is pressed selected nodes should be removed.
///
class QmitkDelKeyFilter : public QObject
{
  Q_OBJECT
public:
  QmitkDelKeyFilter(QObject* _DataManagerView = 0);
protected:
  bool eventFilter(QObject *obj, QEvent *event);
};

#endif // QMITKDELKEYFILTER_H_