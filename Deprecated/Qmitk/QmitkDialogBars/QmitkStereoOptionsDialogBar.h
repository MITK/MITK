/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 11316 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkStereoOptionsDialogBarHIncluded
#define QmitkStereoOptionsDialogBarHIncluded

#include "QmitkDialogBar.h"

/**
 * \brief DialogBar for setting stereo visualization options
 *
 * \ingroup Functionalities
 */
class QMITK_EXPORT QmitkStereoOptionsDialogBar : public QmitkDialogBar
{
  Q_OBJECT

public:
  QmitkStereoOptionsDialogBar( QObject *parent = 0, const char *name = 0,
    QmitkStdMultiWidget *multiWidget = NULL,
    mitk::DataTreeIteratorBase* dataIt = NULL );

  virtual ~QmitkStereoOptionsDialogBar();

  virtual QAction *CreateAction( QObject *parent );

  virtual QString GetFunctionalityName();


protected:
  virtual QWidget *CreateDialogBar( QWidget *parent );

protected slots:
  void stereoSelectionChanged( int id );

};

#endif

