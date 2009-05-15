/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitk.cpp,v $
Language:  C++
Date:      $Date: 2008-04-24 18:31:42 +0200 (Do, 24 Apr 2008) $
Version:   $Revision: 1.0 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef QmitkToolGUIArea_h_Included
#define QmitkToolGUIArea_h_Included

#include "mitkCommon.h"

#include <qwidget.h>

/**
  \brief Dummy class for putting into a GUI (mainly using Qt Designer).

  This class is nothing more than a QWidget. It is good for use with QmitkToolSelectionBox as a place to display GUIs for active tools.

  Last contributor: $Author$
*/
class QMITKEXT_EXPORT QmitkToolGUIArea : public QWidget
{
  Q_OBJECT

  public:
    
	QmitkToolGUIArea( QWidget* parent = 0, Qt::WFlags f = 0 );
  virtual ~QmitkToolGUIArea();

  signals:

  public slots:
  
  protected slots:

  protected:
};

#endif

