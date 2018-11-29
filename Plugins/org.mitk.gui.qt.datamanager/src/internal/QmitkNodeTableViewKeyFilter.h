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

  virtual bool eventFilter(QObject *obj, QEvent *event) override;
  /**
   * @brief The Preferences Service to retrieve and store preferences.
   */
  berry::IPreferencesService *m_PreferencesService;

  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
};

#endif // QMITKNODETABLEVIEWKEYFILTER_H
