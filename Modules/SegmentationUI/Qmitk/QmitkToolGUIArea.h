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

#ifndef QmitkToolGUIArea_h_Included
#define QmitkToolGUIArea_h_Included

#include "mitkCommon.h"
#include <MitkSegmentationUIExports.h>

#include <qwidget.h>

/**
  \brief Dummy class for putting into a GUI (mainly using Qt Designer).

  This class is nothing more than a QWidget. It is good for use with QmitkToolSelectionBox as a place to display GUIs for active tools.

  Last contributor: $Author$
*/
class MitkSegmentationUI_EXPORT QmitkToolGUIArea : public QWidget
{
  Q_OBJECT

  public:
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  QmitkToolGUIArea( QWidget* parent = 0, Qt::WFlags f = 0 );
#else
  QmitkToolGUIArea( QWidget* parent = 0, Qt::WindowFlags f = 0 );
#endif
  virtual ~QmitkToolGUIArea();

  signals:

  public slots:

  protected slots:

  protected:
};

#endif

