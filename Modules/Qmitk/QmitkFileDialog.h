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
#include <qgridlayout.h>

//Microservices
#include "usServiceReference.h"

// MITK
#include <mitkIFileReader.h>

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

  static const std::string VIEW_ID;

  QmitkFileDialog(QWidget* p = 0, Qt::WindowFlags f1 = 0);
  virtual ~QmitkFileDialog();

  /** \brief This method is part of the widget and needs not to be called separately. */
  virtual void CreateQtPartControl(QWidget *parent);
  /** \brief This method is part of the widget and needs not to be called separately. (Creation of the connections of main and control widget.)*/
  virtual void CreateConnections();

  /**
  * \brief Returns the Options that are currently / have been selected in the widget.
  */
  virtual std::vector< mitk::IFileReader::FileServiceOption > GetSelectedOptions();

signals:

  public slots:

    /**
    * \brief Called when the file selection has changed and the options need to be adapted.
    */
    virtual void DisplayOptions(QString path);

    protected slots:

      /**
      * \brief When the Dialog is closed, the subclass must execute the logic to process the selected files.
      */
      virtual void ProcessSelectedFile() = 0;

protected:

  /** \brief Contains the checkboxes for the options*/
  QGridLayout* m_BoxLayout;

  /** \brief The Options the user has set for the reader / writer*/
  std::vector <mitk::IFileReader::FileServiceOption> m_Options;

  /**
  * \brief This is kind of a workaround. Getting the Options is differen whether using a reader or a writer,
  * so the subclasses of this dialog must implement this logic.
  */
  virtual std::vector<mitk::IFileReader::FileServiceOption> QueryAvailableOptions(std::string path) = 0;

  /** \brief Remove all checkboxes from the options box.*/
  virtual void ClearOptionsBox();
};

#endif // _QmitkFileDialog_H_INCLUDED
