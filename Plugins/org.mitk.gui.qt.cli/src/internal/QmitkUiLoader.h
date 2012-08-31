/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
 
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
 * \ingroup org_mitk_gui_qt_cli_internal
 */
class QmitkUiLoader : public ctkCmdLineModuleQtUiLoader
{

  Q_OBJECT

public:
  QmitkUiLoader(const mitk::DataStorage* dataStorage, QObject *parent=0);
  virtual ~QmitkUiLoader();

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
  virtual QWidget* createWidget(const QString & className, QWidget * parent = 0, const QString & name = QString() );

private:

  const mitk::DataStorage* m_DataStorage;

}; // end class

#endif // QmitkUiLoader_h
