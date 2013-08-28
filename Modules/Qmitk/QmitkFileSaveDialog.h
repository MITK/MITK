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

// MITK
#include <QmitkFileDialog.h>
#include <mitkIFileWriter.h>
#include <mitkFileWriterRegistry.h>
#include <mitkBaseData.h>

/**
* \ingroup QmitkModule
*
* \brief
*/
class QMITK_EXPORT QmitkFileSaveDialog :public QmitkFileDialog
{
  //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

private:

public:

  static const std::string VIEW_ID;

  QmitkFileSaveDialog(mitk::BaseData::Pointer baseData, QWidget* p = 0, Qt::WindowFlags f1 = 0);
  virtual ~QmitkFileSaveDialog();

  virtual mitk::IFileWriter* GetWriter();

  /**
  * \brief Writes the Basedata that the WIdget was constructed with into the file selected by the user.
  *
  */
  virtual void WriteBaseData();

signals:

  protected slots:

    virtual void ProcessSelectedFile();

protected:

  mitk::IFileWriter* m_FileWriter;
  std::vector <mitk::IFileWriter::FileServiceOption> m_Options;
  mitk::FileWriterRegistry m_FileWriterRegistry;
  mitk::BaseData::Pointer m_BaseData;

  std::vector<mitk::IFileWriter::FileServiceOption> QueryAvailableOptions(std::string path);

  //Ui::QmitkFileSaveDialogControls* m_Controls; ///< member holding the UI elements of this widget
};

#endif // _QmitkFileSaveDialog_H_INCLUDED
