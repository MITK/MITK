// QmitkSimpleExampleFunctionality.h: interface for the QmitkSimpleExampleFunctionality class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QMITKFUNCTIONALITY_H__1DC0BA6E_9B8D_4D63_8A63_5B661CE33712__INCLUDED_)
#define AFX_QMITKSIMPLEEXAMPLEFUNCTIONALITY_H__1DC0BA6E_9B8D_4D63_8A63_5B661CE33712__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "QmitkFunctionality.h"
#include "QmitkStdMultiWidget.h"
#include "QmitkSimpleExampleControls.h"
#include <mitkSurfaceData.h>
#include <mitkColorProperty.h>
#include <mitkFloatProperty.h>

#include <qlist.h>

/*!\class
\brief MITK example demonstrating how a new application functionality can be implemented

One needs to reimplement the methods createControlWidget(..), createMainWidget(..) 
and createAction(..) from QmitkFunctionality. A QmitkFctMediator object gets passed a reference of a 
functionality and positions the widgets in the application window controlled by a 
layout template
*/
class QmitkSimpleExampleFunctionality : public QmitkFunctionality
{
    Q_OBJECT
public:

    /*!
    \brief default constructor
    */
    QmitkSimpleExampleFunctionality(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget=NULL);

    /*!
    \brief default destructor
    */
    virtual ~QmitkSimpleExampleFunctionality();

    /*!
    \brief method for creating the widget containing the application controls, like sliders, buttons etc.
    */
    virtual QWidget * createControlWidget(QWidget *parent);

    /*!
    \brief method for creating the applications main widget
    */
    virtual QWidget * createMainWidget(QWidget * parent);

    /*!
    \brief method for creating the connections of main and control widget
    */
    virtual void createConnections();

    /*!
    \brief method for creating an QAction object, i.e. button & menu entry
    */
    virtual QAction * createAction(QActionGroup *parent);

    virtual QString getFunctionalityName();

protected slots:
    /*!
    qt slot for event processing from a slider control
    */
    void selectSliceWidgetXY( int z );

    /*!
    qt slot for event processing from a slider control
    */
    void selectSliceWidgetXZ( int z );

    /*!
    qt slot for event processing from a slider control
    */
    void selectSliceWidgetYZ( int z );

    /*!
    qt slot for event processing from a slider control
    */
    void selectSliceWidgetFP( int z );

protected:
    QmitkStdMultiWidget * multiWidget;
    QmitkSimpleExampleControls * controls;
    int count;
    mitk::FloatProperty::Pointer opacityprop;


};

#endif // !defined(AFX_QMITKSIMPLEEXAMPLEFUNCTIONALITY_H__1DC0BA6E_9B8D_4D63_8A63_5B661CE33712__INCLUDED_)
