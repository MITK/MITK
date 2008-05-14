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

#ifndef QmitkSplashScreen_h_Included
#define QmitkSplashScreen_h_Included

#include "mitkCommon.h"
#include <qwidget.h>

/**
  \brief MITK splash screen.

  Should inform the user that this is not a commerical product and not for patient treatment.
*/
class QMITK_EXPORT QmitkSplashScreen : public QWidget
{
  Q_OBJECT

  public:
    
    QmitkSplashScreen( const QPixmap& pixmap );
    virtual ~QmitkSplashScreen();

    virtual void repaint();

  signals:

  public slots:

    virtual void close();
  
  protected slots:

  protected:

};

#endif

