/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkModulesDialog_h
#define QmitkModulesDialog_h

#include <QDialog>

#include <MitkQtWidgetsExtExports.h>

/**
 * \brief Dialog displaying a sortable table of all loaded CppMicroServices modules.
 *
 * Shows module ID, name, version, and location in a QTableView with
 * sort and extended selection support. Uses QmitkModuleTableModel internally.
 *
 * \sa QmitkModuleTableModel, QmitkAboutDialog
 */
class MITKQTWIDGETSEXT_EXPORT QmitkModulesDialog : public QDialog
{
public:
  /**
   * \brief Construct the modules dialog.
   * \param[in] parent The parent widget.
   * \param[in] f Window flags.
   */
  explicit QmitkModulesDialog(QWidget *parent = nullptr,
                              Qt::WindowFlags f = Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
};

#endif
