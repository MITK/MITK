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

#ifndef QMITK_SLIDERDIALOGBAR_H
#define QMITK_SLIDERDIALOGBAR_H


#include "QmitkDialogBar.h"
#include "mitkDataTreeFilter.h"



/**
 * \brief DialogBar with sliders for positioning transversal, sagittal,
 * and coronal planes, and for selecting the time step of a time resolved
 * geometry.
 *
 * \ingroup Functionalities
 */
class QMITK_EXPORT QmitkSliderDialogBar : public QmitkDialogBar
{
  Q_OBJECT

public:
  QmitkSliderDialogBar( QObject *parent = 0, const char *name = 0,
    QmitkStdMultiWidget *multiWidget = NULL,
    mitk::DataTreeIteratorBase* dataIt = NULL );

  virtual ~QmitkSliderDialogBar();

  virtual QAction *CreateAction( QObject *parent );

  virtual QString GetFunctionalityName();


protected:
  virtual QWidget *CreateDialogBar( QWidget *parent );

};

#endif

