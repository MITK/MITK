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
 * \brief DataStorage model that maintains a history of node selections.
 *
 * This model extends QmitkDataStorageDefaultListModel to present nodes
 * ordered by selection time (LIFO -- most recently selected first).
 * The history is maintained as a static list shared across all instances.
 *
 * Nodes appear in the history only if they satisfy all of the following:
 * - The node is still present in the data storage.
 * - The node pointer is valid (not expired).
 * - Each node appears at most once (duplicates are removed).
 *
 * \sa QmitkDataStorageDefaultListModel
 * \sa QmitkDataStorageSelectionHistoryInspector
 */
class MITKQTWIDGETS_EXPORT QmitkDataStorageHistoryModel : public QmitkDataStorageDefaultListModel
{
    Q_OBJECT

public:

    /**
     * \brief Constructs the history model.
     * \param[in] parent The parent QObject.
     */
    QmitkDataStorageHistoryModel(QObject *parent);

    /**
     * \brief Adds a node to the global selection history.
     *
     * If the node is already present in the history, the old entry is removed
     * before inserting the node at the front. A nullptr node is silently ignored.
     *
     * \param[in] node The data node to add. May be nullptr.
     */
    static void AddNodeToHistory(mitk::DataNode* node);

    /**
     * \brief Clears the entire selection history.
     */
    static void ResetHistory();

protected:
    void UpdateModelData() override;
};

#endif
