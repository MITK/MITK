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

//QT headers
#include <QFileDialog>

// MITK
#include <mitkIFileReader.h>

class QmitkFileDialogPrivate;

/**
 * \ingroup QmitkModule
 *
 * \brief This is the abstract base class for QmitkFileOpenDialog and QmitkFileSaveDialog.
 *
 * It contains common functionality and logic, but is not really useful on it's own. Use the subclasses!
 */
class QMITK_EXPORT QmitkFileDialog :public QFileDialog
{
  //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

public:

  QmitkFileDialog(QWidget* p = 0, Qt::WindowFlags f1 = 0);
  virtual ~QmitkFileDialog();

  /**
   * \brief Returns the Options that are currently / have been selected in the widget.
   */
  virtual mitk::IFileReader::OptionList GetSelectedOptions();

protected:

  /**
   * \brief This is kind of a workaround. Getting the Options is different whether using a reader or a writer,
   * so the subclasses of this dialog must implement this logic.
   */
  virtual mitk::IFileReader::OptionList QueryAvailableOptions(const QString& path) = 0;


  /**
   * \brief Called when the file selection has changed and the options need to be adapted.
   */
  Q_SLOT void DisplayOptions(const QString& path);


  /**
   * \brief When the Dialog is closed, the subclass must execute the logic to process the selected files.
   */
  Q_SLOT virtual void ProcessSelectedFile() = 0;


private:

  QScopedPointer<QmitkFileDialogPrivate> d;
};

#endif // _QmitkFileDialog_H_INCLUDED
