/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDataStorageHistoryModel_h
#define QmitkDataStorageHistoryModel_h

#include <MitkQtWidgetsExports.h>

#include <QmitkDataStorageDefaultListModel.h>

/**
* @brief Internal DataStorage model to represent the history of node selections.
*
* The model will present all nodes in the history under the following conditions
* - the nodes are sorted by selection time (lifo -> last is first)
* - node must be in the storage
* - node must be valid
* - node will only be in the history once.
*
*/

class MITKQTWIDGETS_EXPORT QmitkDataStorageHistoryModel : public QmitkDataStorageDefaultListModel
{
    Q_OBJECT

public:

    QmitkDataStorageHistoryModel(QObject *parent);

    /** Adds the passed node to the history. If the node is already in the history, old instances will be removed.
     If the passed node is nullptr, it will be ignored.*/
    static void AddNodeToHistory(mitk::DataNode* node);
    static void ResetHistory();

protected:
    void UpdateModelData() override;
};

#endif
