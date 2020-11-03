/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkUiLoader_h
#define QmitkUiLoader_h

#include <ctkCmdLineModuleQtUiLoader.h>
#include <QStringList>
#include "mitkDataStorage.h"

/**
 * \class QmitkUiLoader
 * \brief Derived from ctkCmdLineModuleQtGuiLoader to enable us to instantiate widgets from Qmitk at runtime,
 * and currently we instatiate QmitkDataStorageComboBoxWithSelectNone, used for image input widgets.
 * \author Matt Clarkson (m.clarkson@ucl.ac.uk)
 * \ingroup org_mitk_gui_qt_cmdlinemodules_internal
 * \sa ctkCmdLineModuleQtUiLoader
 */
class QmitkUiLoader : public ctkCmdLineModuleQtUiLoader
{

  Q_OBJECT

public:
  QmitkUiLoader(const mitk::DataStorage* dataStorage, QObject *parent=nullptr);
  ~QmitkUiLoader() override;

  /**
   * \brief Returns the list of available widgets in ctkCmdLineModuleQtGuiLoader and also QmitkDataStorageComboBoxWithSelectNone.
   * \see ctkCmdLineModuleQtGuiLoader::availableWidgets()
   */
  QStringList availableWidgets () const;

  /**
   * \brief If className is QmitkDataStorageComboBox, instantiates QmitkDataStorageComboBoxWithSelectNone and
   * otherwise delegates to base class.
   * \see ctkCmdLineModuleQtGuiLoader::createWidget()
   */
  QWidget* createWidget(const QString & className, QWidget * parent = nullptr, const QString & name = QString() ) override;

private:

  const mitk::DataStorage* m_DataStorage;

}; // end class

#endif // QmitkUiLoader_h
