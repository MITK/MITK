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
	virtual QWidget * createControlWidget(QWidget *parent) = 0;
	virtual QWidget * createMainWidget(QWidget * parent) = 0;
	virtual QAction * createAction(QActionGroup * parent) { return NULL; };

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
