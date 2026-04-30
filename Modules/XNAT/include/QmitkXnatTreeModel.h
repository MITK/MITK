/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkXnatTreeModel_h
#define QmitkXnatTreeModel_h

// CTK includes
#include <ctkXnatTreeModel.h>

// MITK includes
#include <MitkXNATExports.h>

namespace mitk
{
  class DataNode;
}

/**
 * \brief Tree model for browsing the XNAT server hierarchy with MITK-specific enhancements.
 *
 * QmitkXnatTreeModel extends ctkXnatTreeModel to provide MITK-specific icon decoration for
 * different XNAT object types (server, project, subject, experiment, resource, scan, file),
 * drag-and-drop support for uploading MITK data nodes to XNAT resources, and error handling
 * that displays user-facing error dialogs through QmitkHttpStatusCodeHandler.
 *
 * The model supports inverse matching in its match() override (returns non-matching indices),
 * and allows dropping data nodes onto subjects, experiments, resources, and resource folders.
 *
 * \sa QmitkHttpStatusCodeHandler, QmitkSelectXnatUploadDestinationDialog, ctkXnatTreeModel
 */
class MITKXNAT_EXPORT QmitkXnatTreeModel : public ctkXnatTreeModel
{
  Q_OBJECT

public:
  /**
   * \brief Default constructor.
   */
  QmitkXnatTreeModel();

  /**
   * \brief Return data for the given index and role, with MITK-specific icon decoration.
   *
   * For Qt::DecorationRole, returns icons specific to the XNAT object type at the given
   * index (server, project, subject, experiment, resource folder, resource, scan folder,
   * scan, or file). For all other roles, delegates to ctkXnatTreeModel::data().
   *
   * \param[in] index The model index to retrieve data for.
   * \param[in] role The Qt item data role.
   * \return The data for the given role, or an invalid QVariant if the index is not valid.
   */
  QVariant data(const QModelIndex &index, int role) const override;

  /**
   * \brief Handle dropped MIME data containing MITK data node pointers.
   *
   * When data nodes are dropped onto an XNAT tree node, emits the ResourceDropped signal
   * with the list of dropped nodes, the target XNAT object, and the parent index.
   *
   * \param[in] data The MIME data containing serialized data node pointers.
   * \param[in] action The drop action being performed.
   * \param[in] row The target row (unused).
   * \param[in] column The target column (unused).
   * \param[in] parent The model index of the drop target.
   * \return \c true if the data was in the expected MITK data node format, \c false otherwise.
   */
  bool dropMimeData(
    const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

  using QAbstractItemModel::supportedDropActions;

  /**
   * \brief Return supported drop actions (Qt::CopyAction).
   *
   * \return Qt::CopyAction.
   */
  virtual Qt::DropActions supportedDropActions();

  /**
   * \brief Return item flags for the given index.
   *
   * Adds Qt::ItemIsDropEnabled for indices representing subjects, experiments, resources,
   * or resource folders. Other XNAT object types do not accept drops.
   *
   * \param[in] index The model index to query flags for.
   * \return The item flags for the given index.
   */
  Qt::ItemFlags flags(const QModelIndex &index) const override;

  /**
   * \brief Look up an XNAT object by its REST URL.
   *
   * Traverses the XNAT hierarchy (project, subject, experiment, scan) by matching URL
   * segments, fetching children as needed. Returns the deepest matching object, or the
   * "Resources" folder of the deepest matched parent.
   *
   * \param[in] url The XNAT REST URL to resolve (e.g., ".../projects/P/subjects/S/experiments/E").
   * \return Pointer to the matching ctkXnatObject, or \c nullptr if the root is not valid.
   */
  ctkXnatObject *GetXnatObjectFromUrl(const QString &url);

  /**
   * \brief Fetch child data for the given index, with error handling.
   *
   * Delegates to ctkXnatTreeModel::fetchMore() and catches ctkRuntimeException to
   * display an error dialog via QmitkHttpStatusCodeHandler. Emits the Error signal
   * on failure.
   *
   * \param[in] index The model index to fetch children for.
   */
  void fetchMore(const QModelIndex &index) override;

  /**
   * \brief Find model indices that do NOT match the given value (inverse matching).
   *
   * Unlike the standard QAbstractItemModel::match(), this implementation returns indices
   * whose data does \e not match the specified value and match flags. This is useful for
   * filtering out matching items.
   *
   * \param[in] start The starting index for the search.
   * \param[in] role The item data role to match against.
   * \param[in] value The value to compare against.
   * \param[in] hits Maximum number of results to return (-1 for unlimited).
   * \param[in] flags Match flags controlling the comparison type and recursion.
   * \return List of model indices whose data does not match the given value.
   */
  QModelIndexList match(
    const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const override;

signals:
  /**
   * \brief Emitted when fetching child data for an index fails due to an HTTP error.
   *
   * \param[in] idx The model index for which the fetch operation failed.
   */
  void Error(const QModelIndex &idx);

  /**
   * \brief Emitted when MITK data nodes are dropped onto an XNAT tree node.
   *
   * \param[in] nodes The list of dropped MITK data nodes.
   * \param[in] target The XNAT object that received the drop.
   * \param[in] index The model index of the drop target.
   */
  void ResourceDropped(const QList<mitk::DataNode *> &nodes, ctkXnatObject *target, const QModelIndex &index);

private:
  ctkXnatObject *InternalGetXnatObjectFromUrl(const QString &xnatObjectType, const QString &url, ctkXnatObject *parent);
};

#endif
