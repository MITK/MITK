/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-13 14:07:17 +0100 (Mi, 13 Feb 2008) $
Version:   $Revision: 1.0 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef QmitkCopyToClipBoardDialog_h_Included
#define QmitkCopyToClipBoardDialog_h_Included

#include <qdialog.h>
#include "QmitkExtExports.h"
#include <mitkCommon.h>
/**
  \brief Displays read-only QTextEdit.

  For output of any kind of information that might be copied into other applications.
*/
class QmitkExt_EXPORT QmitkCopyToClipBoardDialog : public QDialog
{
  Q_OBJECT

  public:
    
    QmitkCopyToClipBoardDialog(const QString& text, QWidget* parent = 0, const char* name = 0);
    virtual ~QmitkCopyToClipBoardDialog();

  signals:

  public slots:
  
  protected slots:

  protected:

};

#endif

