/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKNODETABLEVIEWKEYFILTER_H
#define QMITKNODETABLEVIEWKEYFILTER_H

// mitk core
#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>

#include <QObject>

namespace berry
{
  struct IPreferencesService;
}

/**
* @brief A small class which receives key-pressed events on the node table.
*/
class QmitkNodeTableViewKeyFilter : public QObject
{
  Q_OBJECT

public:

  QmitkNodeTableViewKeyFilter(QObject *dataManagerView, mitk::DataStorage *dataStorage);

protected:

  bool eventFilter(QObject *obj, QEvent *event) override;
  /**
   * @brief The Preferences Service to retrieve and store preferences.
   */
  berry::IPreferencesService *m_PreferencesService;

  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
};

#endif // QMITKNODETABLEVIEWKEYFILTER_H
