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

#ifndef QmitkStandardViewsDialogBarHIncluded
#define QmitkStandardViewsDialogBarHIncluded

#include "QmitkDialogBar.h"
#include "mitkDataTreeFilter.h"


/**
 * \brief DialogBar for selection an standard view in the 3D widget.
 *
 * \ingroup Functionalities
 */
class QMITK_EXPORT QmitkStandardViewsDialogBar : public QmitkDialogBar
{
  Q_OBJECT

public:
  QmitkStandardViewsDialogBar( QObject *parent = 0, const char *name = 0,
    QmitkStdMultiWidget *multiWidget = NULL,
    mitk::DataTreeIteratorBase* dataIt = NULL );

  virtual ~QmitkStandardViewsDialogBar();

  virtual QAction *CreateAction( QObject *parent );

  virtual QString GetFunctionalityName();


protected:
  virtual QWidget *CreateDialogBar( QWidget *parent );

};

#endif

