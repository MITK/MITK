/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef QMITKLEVELWINDOWRANGECHANGEDIALOG_H_
#define QMITKLEVELWINDOWRANGECHANGEDIALOG_H_

#include "ui_QmitkLevelWindowRangeChange.h"

#include <QDialog>

#include <mitkCommon.h>

class QMITK_EXPORT QmitkLevelWindowRangeChangeDialog : public QDialog, public Ui::QmitkLevelWindowRangeChange
{
  Q_OBJECT
  
public:
  
  QmitkLevelWindowRangeChangeDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);

  int getLowerLimit();

  int getUpperLimit();

  void setLowerLimit( int rangeMin );

  void setUpperLimit( int rangeMax );

protected slots:
  
  void inputValidator();
  
};

#endif /*QMITKLEVELWINDOWRANGECHANGEDIALOG_H_*/
