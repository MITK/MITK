// QmitkFunctionality.h: interface for the QmitkFunctionality class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QUSFUNCTIONALITY_H__1DC0BA6E_9B8D_4D63_8A63_5B661CE33712__INCLUDED_)
#define AFX_QUSFUNCTIONALITY_H__1DC0BA6E_9B8D_4D63_8A63_5B661CE33712__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <qobject.h>
#include <qstring.h>
#include <qtoolbutton.h>
#include <qaction.h>

class QActionGroup;

/*!
 \brief class providing a standard interface for MITK application functionality

 A functionality contains the application main widget, a controls widget and the 
 application logic
 */
class QmitkFunctionality : public QObject
{
	Q_OBJECT
	bool available;
protected:
	virtual void setAvailability(bool available);
public:
    /*!
    \brief method for creating the widget containing the functionality's controls, like sliders, buttons etc.
    */
	virtual QWidget * createControlWidget(QWidget *parent) = 0;
    /*!
    \brief method for creating the functionality's main widget
    */
	virtual QWidget * createMainWidget(QWidget * parent) = 0;
    /*!
    \brief method for creating the connections of main and control widget
    */
	virtual void createConnections()
    {
    };
    /*!
    \brief method for creating a QAction object, i.e., toolbar button and menu entry
    */
	virtual QAction * createAction(QActionGroup * parent) { return NULL; };

    /*!
    \brief method for defining the name of the functionality
    */
	virtual QString getFunctionalityName() = 0;

	virtual void activated();
	virtual void deactivated();

	virtual bool isAvailable();

	QmitkFunctionality(QObject *parent=0, const char *name=0);
	virtual ~QmitkFunctionality();
signals:
	void signal_dummy();
	void availabilityChanged(QmitkFunctionality*);
	void availabilityChanged();
};

#endif // !defined(AFX_QUSFUNCTIONALITY_H__1DC0BA6E_9B8D_4D63_8A63_5B661CE33712__INCLUDED_)
