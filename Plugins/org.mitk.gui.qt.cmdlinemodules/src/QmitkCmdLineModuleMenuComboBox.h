/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCmdLineModuleMenuComboBox_h
#define QmitkCmdLineModuleMenuComboBox_h

#include <QObject>
#include <QList>
#include <QHash>
#include <ctkMenuComboBox.h>
#include <ctkCmdLineModuleManager.h>

/**
 * \class QmitkCmdLineModuleMenuComboBox
 * \brief Subclass of ctkMenuComboBox to listen to ctkCmdLineModuleManager
 * moduleRegistered and moduleUnregistered signals, and update the menu accordingly.
 * \ingroup org_mitk_gui_qt_cmdlinemodules_internal
 * \author Matt Clarkson (m.clarkson@ucl.ac.uk)
 * \sa ctkMenuComboBox
 */
class QmitkCmdLineModuleMenuComboBox : public ctkMenuComboBox {

  Q_OBJECT

public:

  QmitkCmdLineModuleMenuComboBox(QWidget* parent = nullptr);
  ~QmitkCmdLineModuleMenuComboBox() override;

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

#endif
