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

#ifndef QmitkFreeSurferParcellationWidget_h
#define QmitkFreeSurferParcellationWidget_h

// Qmitk
#include "ui_QmitkFreeSurferParcellationWidgetControls.h"
#include <QmitkFreeSurferParcellationHandler.h>
// Qt
#include <QObject>
#include <QString>
#include <QWidget>

class QmitkFreeSurferParcellationWidget;

class QmitkFreeSurferParcellationWidget : public QWidget
{
    Q_OBJECT

  public:
    typedef Ui::QmitkFreeSurferParcellationWidgetControls  ViewType;
    typedef QmitkFreeSurferParcellationHandler             HandlerType;

    // Constructors
    QmitkFreeSurferParcellationWidget( QWidget* parent = 0 );
    ~QmitkFreeSurferParcellationWidget();


    // Signals
  Q_SIGNALS:
    /**
     * @brief Emitted when anatomical region is clicked
     */
    void Clicked( const QString & );
    /**
     * @brief Emitted when anatomical region is hovered
     */
    void Hovered( const QString & );


    // Slots
  protected slots:
    void OnHandlerClicked( const QString & name );
    void OnHandlerHovered( const QString & name );

  protected:
    // Members:
    ViewType             m_View;
    HandlerType*         m_Handler;
};

#endif  //QmitkFreeSurferParcellationWidget_h
