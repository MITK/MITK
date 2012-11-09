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

#ifndef QMITKNODETABLEVIEWKEYFILTER_H_
#define QMITKNODETABLEVIEWKEYFILTER_H_

#include "berryIPreferencesService.h"

#include <QObject>

///
/// A small class which "eats" all Del-Key-pressed events on the node table.
/// When the Del Key is pressed selected nodes should be removed.
///
class QmitkNodeTableViewKeyFilter : public QObject
{
  Q_OBJECT
public:
  QmitkNodeTableViewKeyFilter(QObject* _DataManagerView = 0);
protected:
  bool eventFilter(QObject *obj, QEvent *event);

  ///
  /// The Preferences Service to retrieve and store preferences.
  ///
  berry::IPreferencesService::WeakPtr m_PreferencesService;
};

#endif // QMITKNODETABLEVIEWKEYFILTER_H_
