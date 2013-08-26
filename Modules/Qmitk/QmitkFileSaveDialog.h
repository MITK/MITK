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

#ifndef _QmitkFileSaveDialog_H_INCLUDED
#define _QmitkFileSaveDialog_H_INCLUDED

#include "QmitkExports.h"
#include <vector>

//QT headers
#include <QWidget>
#include <qfiledialog.h>
#include <qgridlayout.h>

//Microservices
#include "usServiceReference.h"
#include "usModuleContext.h"
#include "usServiceEvent.h"
#include "usServiceInterface.h"

// MITK
#include <mitkIFileWriter.h>
#include <mitkFileReaderManager.h>

/**
* \ingroup QmitkModule
*
* \brief
*/
class QMITK_EXPORT QmitkFileSaveDialog :public QFileDialog
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

  QmitkFileSaveDialog(QWidget* p = 0, Qt::WindowFlags f1 = 0);
  virtual ~QmitkFileSaveDialog();

  /** \brief This method is part of the widget and needs not to be called separately. */
  virtual void CreateQtPartControl(QWidget *parent);
  /** \brief This method is part of the widget and needs not to be called separately. (Creation of the connections of main and control widget.)*/
  virtual void CreateConnections();

  virtual std::list< mitk::IFileWriter::FileServiceOption > GetSelectedOptions();

  virtual mitk::IFileReader* GetReader();

  virtual std::list< mitk::BaseData::Pointer > GetBaseData();

signals:

  public slots:

    virtual void DisplayOptions(QString path);

    protected slots:

      virtual void ProcessSelectedFile();

protected:

  QGridLayout* m_BoxLayout;
  mitk::IFileReader* m_FileReader;
  std::list <mitk::IFileWriter::FileServiceOption> m_Options;
  mitk::FileReaderManager m_FileReaderManager;

  virtual void ClearOptionsBox();

  //Ui::QmitkFileSaveDialogControls* m_Controls; ///< member holding the UI elements of this widget
};

#endif // _QmitkFileSaveDialog_H_INCLUDED
