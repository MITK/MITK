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

#ifndef QMITK_DIALOGBAR_H
#define QMITK_DIALOGBAR_H


#include "QmitkBaseFunctionalityComponent.h"
#include "mitkPropertyList.h"

#include <qstring.h>

class QmitkStdMultiWidget;

class QObject;
class QAction;
class QGroupBox;


/**
 * \brief Class providing a standard interface for MITK application
 * dialog bars.
 *
 * A QmitkDialogBar is similar to a QmitkFunctionality (in fact, they share
 * the same superclass) in the sense that both provide application-specific
 * control over MITK and add functionality to the base application. However,
 * in contrast to QmitkFunctionality which generally are full-fledged
 * application modules, QmitkDialogBar classes are small handy widgets which
 * help to execute a small and very specific task. Different from
 * functionalities, dialog bars can be permanently active, and multiple
 * dialog bars can be active at the same time. They are enabled/disabled via
 * toggle buttons in the applications main tool bar.
 *
 * An example class is QmitkSliderDialogBar, which is a small GUI tool for
 * controlling spatial and temporal slicing through the dataset/geometry.
 *
 * While subclasses are responsible for creating and maintaining a suitable
 * (and suitably small) control widget, the superclass Qmitk automatically
 * creates a QGroupBox around the subclasses' widgets.
 *
 * \ingroup Functionalities
 */
class QmitkDialogBar : public QmitkBaseFunctionalityComponent
{
  Q_OBJECT

public:
  QmitkDialogBar( const char *caption, 
    QObject *parent = 0, const char *name = 0,
    QmitkStdMultiWidget *multiWidget = NULL, 
    mitk::DataTreeIteratorBase* dataIt = NULL);

  virtual ~QmitkDialogBar();

  virtual QWidget *CreateControlWidget( QWidget *parent );

  virtual QAction *CreateAction( QObject *parent ) = 0;

  virtual const QString &GetCaption() const;

  QmitkStdMultiWidget *GetMultiWidget() const;

  void SetGlobalOptions( mitk::PropertyList::Pointer options );

public slots:
  virtual void ToggleVisible( bool on );

protected:
  virtual QWidget *CreateDialogBar( QWidget *parent ) = 0;

  QGroupBox *m_GroupBox;

  QmitkStdMultiWidget *m_MultiWidget;

  QString m_Caption;

private:
  mitk::PropertyList::Pointer m_GlobalOptions;

};

#endif

