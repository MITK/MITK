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

#include "MitkQtWidgetsExports.h"

// STL
#include <vector>

// MITK
#include <QmitkFileDialog.h>
#include <mitkIFileWriter.h>
#include <mitkBaseData.h>

class QmitkFileSaveDialogPrivate;

/**
* \ingroup QmitkModule
*
* \brief
*/
class QMITK_EXPORT QmitkFileSaveDialog :public QmitkFileDialog
{
  //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

public:

  QmitkFileSaveDialog(mitk::BaseData::Pointer baseData, QWidget* p = 0, Qt::WindowFlags f1 = 0);
  virtual ~QmitkFileSaveDialog();

  mitk::IFileWriter* GetWriter();

  /**
  * \brief Writes the Basedata that the WIdget was constructed with into the file selected by the user.
  *
  */
  void WriteBaseData();

protected:

  virtual mitk::IFileWriter::OptionList QueryAvailableOptions(const QString& path);

  virtual void ProcessSelectedFile();

private:

  QScopedPointer<QmitkFileSaveDialogPrivate> d;

};

#endif // _QmitkFileSaveDialog_H_INCLUDED
