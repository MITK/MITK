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

// STL
#include <vector>

// MITK
#include <QmitkFileDialog.h>
#include <mitkIFileReader.h>
#include <mitkBaseData.h>

class QmitkFileOpenDialogPrivate;

/**
* \ingroup QmitkModule
*
* \brief
*/
class QMITK_EXPORT QmitkFileOpenDialog :public QmitkFileDialog
{
  //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

public:

  QmitkFileOpenDialog(QWidget* p = 0, Qt::WindowFlags f1 = 0);
  virtual ~QmitkFileOpenDialog();

  std::vector <mitk::IFileReader*> GetReaders();

  std::vector< mitk::BaseData::Pointer > GetBaseDatas();

protected:

  virtual mitk::IFileReader::OptionList QueryAvailableOptions(const QString& path);

  virtual void ProcessSelectedFile();

private:

  QScopedPointer<QmitkFileOpenDialogPrivate> d;

};

#endif // _QmitkFileOpenDialog_H_INCLUDED
