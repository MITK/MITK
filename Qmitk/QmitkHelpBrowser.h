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
#ifndef HELPWINDOW_H
#define HELPWINDOW_H

#include <qmainwindow.h>
#include <qtextbrowser.h>

class QComboBox;
class QPopupMenu;

class QmitkHelpBrowser : public QMainWindow
{
    Q_OBJECT
public:
    QmitkHelpBrowser( const QString& home_,  const QString& path, QWidget* parent = 0, const char *name=0 );
    ~QmitkHelpBrowser();

private slots:
    void setBackwardAvailable( bool );
    void setForwardAvailable( bool );

    void sourceChanged( const QString& );
    void print();

    void pathSelected( const QString & );

private:

    QTextBrowser* browser;
    QComboBox *pathCombo;
    int backwardId, forwardId;

};

#endif

