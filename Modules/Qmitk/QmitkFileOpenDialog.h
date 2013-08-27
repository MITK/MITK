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

#ifndef _QmitkFileOpenDialog_H_INCLUDED
#define _QmitkFileOpenDialog_H_INCLUDED

#include "QmitkExports.h"
#include <vector>

//QT headers
#include <QWidget>
#include <qgridlayout.h>

//Microservices
#include "usServiceReference.h"
#include "usModuleContext.h"
#include "usServiceEvent.h"
#include "usServiceInterface.h"

// MITK
#include <QmitkFileDialog.h>
#include <mitkIFileReader.h>
#include <mitkFileReaderRegistry.h>

/**
* \ingroup QmitkModule
*
* \brief
*/
class QMITK_EXPORT QmitkFileOpenDialog :public QmitkFileDialog
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

  QmitkFileOpenDialog(QWidget* p = 0, Qt::WindowFlags f1 = 0);
  virtual ~QmitkFileOpenDialog();

  virtual std::list <mitk::IFileReader*> GetReaders();

  virtual std::list< mitk::BaseData::Pointer > GetBaseDatas();

signals:

  public slots:

    protected slots:

      virtual void ProcessSelectedFile();

protected:

  std::list<mitk::IFileReader*> m_FileReaders;
  std::list <mitk::IFileReader::FileServiceOption> m_Options;
  mitk::FileReaderRegistry m_FileReaderRegistry;

  virtual std::list<mitk::IFileReader::FileServiceOption> QueryAvailableOptions(std::string path) ;

  //Ui::QmitkFileOpenDialogControls* m_Controls; ///< member holding the UI elements of this widget
};

#endif // _QmitkFileOpenDialog_H_INCLUDED
