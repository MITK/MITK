/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef _QmitkFileDialog_H_INCLUDED
#define _QmitkFileDialog_H_INCLUDED

#include "QmitkExports.h"
#include <vector>

//QT headers
#include <QWidget>
#include <qfiledialog.h>
#include <QListWidgetItem>

//Microservices
#include "usServiceReference.h"
#include "usModuleContext.h"
#include "usServiceEvent.h"
#include "usServiceInterface.h"

/**
* \ingroup QmitkModule
*
* \brief
*/
class QMITK_EXPORT QmitkFileDialog :public QFileDialog
{
  //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

private:

  //us::ModuleContext* m_Context;
  ///** \brief a filter to further narrow down the list of results*/
  //std::string m_Filter;
  ///** \brief The name of the ServiceInterface that this class should list */
  //std::string m_Interface;
  ///** \brief The name of the ServiceProperty that will be displayed in the list to represent the service */
  //std::string m_NamingProperty;

public:

  static const std::string VIEW_ID;

  QmitkFileDialog(QWidget* p = 0, Qt::WindowFlags f1 = 0);
  virtual ~QmitkFileDialog();

  /** \brief This method is part of the widget and needs not to be called separately. */
  //virtual void CreateQtPartControl(QWidget *parent);
  /** \brief This method is part of the widget and needs not to be called separately. (Creation of the connections of main and control widget.)*/
  virtual void CreateConnections();

signals:

  public slots:

    protected slots:

protected:

  //Ui::QmitkFileDialogControls* m_Controls; ///< member holding the UI elements of this widget
};

#endif // _QmitkFileDialog_H_INCLUDED
