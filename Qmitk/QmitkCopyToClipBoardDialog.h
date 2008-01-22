/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitk.cpp,v $
Language:  C++
Date:      $Date$
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

/**
  \brief Displays read-only QTextEdit.

  For output of any kind of information that might be copied into other applications.
*/
class QmitkCopyToClipBoardDialog : public QDialog
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

