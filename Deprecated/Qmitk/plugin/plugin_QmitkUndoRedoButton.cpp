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

#include "plugin_QmitkUndoRedoButton.h"

#include "QmitkUndoRedoButton.h"
#include "qundoredobutton.h"
#include <iostream>

static const char *undoredobutton_xpm[] = {
"18 11 5 1",
"   c None",
".  c #00008B",
"+  c #000083",
"@  c #8B858B",
"#  c #838183",
"         .+.+.+@  ",
".      +.+.+.+.+. ",
"+.   .+.+      .+@",
".+. .+.         .+",
"+.+.+.          +.",
".+.+.           .+",
"+.+.+.          +.",
".+.+.+.         .+",
"+.+.+.+.       .+@",
"             #.+. ",
"             .+.  "
};
  
QmitkUndoRedoButtonPlugin::QmitkUndoRedoButtonPlugin()
{
}

QmitkUndoRedoButtonPlugin::~QmitkUndoRedoButtonPlugin()
{
}

QStringList QmitkUndoRedoButtonPlugin::keys() const
{
    QStringList list;
    list << "QUndoRedoButton" << "QmitkUndoRedoButton";
    return list;
}

QWidget* QmitkUndoRedoButtonPlugin::create( const QString &key, QWidget* parent, const char* name )
{
    if ( key == "QmitkUndoRedoButton" )
        return new QmitkUndoRedoButton( parent, name );
    if ( key == "QUndoRedoButton" )
        return new QUndoRedoButton( parent, name );
    return 0;
}

QString QmitkUndoRedoButtonPlugin::group( const QString& feature) const
{
    if ( feature == "QmitkUndoRedoButton" ) return "MITK";
    if ( feature == "QUndoRedoButton" ) return "Input";
    return QString::null;
}

QIconSet QmitkUndoRedoButtonPlugin::iconSet( const QString& feature) const
{
    
  if ( feature == "QmitkUndoRedoButton" ) return QIconSet( QPixmap( undoredobutton_xpm ) );
  if ( feature == "QUndoRedoButton" )     return QIconSet( QPixmap( undoredobutton_xpm ) );
  return QIconSet();
}

QString QmitkUndoRedoButtonPlugin::includeFile( const QString& feature ) const
{
  if ( feature == "QmitkUndoRedoButton" )    return QString("QmitkUndoRedoButton.h");
  if ( feature == "QUndoRedoButton" )    return QString("qundoredobutton.h");
  return QString::null;
}

QString QmitkUndoRedoButtonPlugin::toolTip( const QString& feature ) const
{
    if ( feature == "QmitkUndoRedoButton" )
      return "A Undo/Redo Button like in Word (for use with MITK)";
    if ( feature == "QUndoRedoButton" )
      return "A Undo/Redo Button like in Word";
    return QString::null;
}

QString QmitkUndoRedoButtonPlugin::whatsThis( const QString& feature ) const
{
    if ( feature == "QmitkUndoRedoButton" )
      return "A Undo/Redo Button like in Word (for use with MITK)";
    if ( feature == "QUndoRedoButton" )
      return "A Undo/Redo Button like in Word";
    return QString::null;
}

bool QmitkUndoRedoButtonPlugin::isContainer( const QString& ) const
{
    return FALSE;
}

Q_EXPORT_PLUGIN( QmitkUndoRedoButtonPlugin )
