/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkModuleTableModel_h
#define QmitkModuleTableModel_h

#include <QAbstractTableModel>
#include <QList>

#include <MitkQtWidgetsExtExports.h>

namespace us
{
  class ModuleContext;
  class Module;
}

class QmitkModuleTableModelPrivate;

/**
 * \brief Table model listing all CppMicroServices modules.
 *
 * Provides four columns: ID, Name, Version, and Location. Listens for
 * module events and automatically updates when modules are loaded or unloaded.
 * Unloaded modules are displayed with gray text. Tooltips show detailed module info.
 */
class MITKQTWIDGETSEXT_EXPORT QmitkModuleTableModel : public QAbstractTableModel
{
public:
  /**
   * \brief Construct the model.
   * \param[in] parent The parent QObject.
   * \param[in] mc The module context to query. Defaults to the current module context.
   */
  QmitkModuleTableModel(QObject *parent = nullptr, us::ModuleContext *mc = nullptr);

  /** \brief Destructor. Removes the module event listener. */
  ~QmitkModuleTableModel() override;

protected:
  /**
   * \brief Return the number of modules.
   * \param[in] parent The parent index (unused for table models).
   * \return The number of rows (modules).
   */
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  /**
   * \brief Return the number of columns (always 4: ID, Name, Version, Location).
   * \param[in] parent The parent index (unused for table models).
   * \return 4
   */
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  /**
   * \brief Return data for the given index and role.
   *
   * Supports Qt::DisplayRole, Qt::ForegroundRole (gray for unloaded), and Qt::ToolTipRole.
   *
   * \param[in] index The model index.
   * \param[in] role The data role.
   * \return The requested data as QVariant.
   */
  QVariant data(const QModelIndex &index, int role) const override;

  /**
   * \brief Return column header data.
   * \param[in] section The column index.
   * \param[in] orientation The header orientation (only Qt::Horizontal is supported).
   * \param[in] role The data role (only Qt::DisplayRole is supported).
   * \return The header label as QVariant.
   */
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
  friend class QmitkModuleTableModelPrivate;

  void insertModule(us::Module *module);

  QmitkModuleTableModelPrivate *const d;
};

#endif
