/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKCMDLINEMODULEMENUCOMBOBOX_H
#define QMITKCMDLINEMODULEMENUCOMBOBOX_H

#include <QObject>
#include <QList>
#include <QHash>
#include <ctkMenuComboBox.h>
#include <ctkCmdLineModuleManager.h>

/**
 * \class QmitkCmdLineModuleMenuComboBox
 * \brief Subclass of ctkMenuComboBox to listen to ctkCmdLineModuleManager
 * moduleRegistered and moduleUnregistered signals, and update the menu accordingly.
 * \ingroup org_mitk_gui_qt_cli_internal
 * \sa ctkMenuComboBox
 */
class QmitkCmdLineModuleMenuComboBox : public ctkMenuComboBox {

  Q_OBJECT

public:

  QmitkCmdLineModuleMenuComboBox(QWidget* parent = 0);
  virtual ~QmitkCmdLineModuleMenuComboBox();

  /**
   * \brief Inject the module manager, so that this widget can
   * still easily be used via widget promotion in Qt Designer,
   * as it will maintain the default constructor.
   */
  void SetManager(ctkCmdLineModuleManager* manager);

  /**
   * \brief Returns the ctkCmdLineModuleManager.
   */
  ctkCmdLineModuleManager* GetManager() const;

private slots:

  void OnModuleRegistered(const ctkCmdLineModuleReference&);
  void OnModuleUnRegistered(const ctkCmdLineModuleReference&);

private:

  void RebuildMenu();
  void AddName(QList< QHash<QString, QMenu*>* >& listOfHashMaps, const int& depth, const QString& name, QMenu* menu);

  ctkCmdLineModuleManager* m_ModuleManager;
};

#endif QMITKCMDLINEMODULEMENUCOMBOBOX_H

