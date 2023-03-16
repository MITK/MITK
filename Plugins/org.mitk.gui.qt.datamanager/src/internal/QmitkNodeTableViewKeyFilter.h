/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkNodeTableViewKeyFilter_h
#define QmitkNodeTableViewKeyFilter_h

// mitk core
#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>

#include <QObject>

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

  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
};

#endif
