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

#include "mitkDataTree.h"
#include "QmitkSelectableGLWidget.h"
#include "vecmath.h"

/*!
 \brief class providing a standard interface for MITK application functionality

 A functionality contains the application main widget, a controls widget and the 
 application logic. The functionality has to set up the main widget according to 
 its needs when it is activated, especially if it uses a shared main wiget. 
 It does @em not need to reverse the settings when it is deactivated, since it cannot
 know if the subsequently activated functionality will use that settings or some other
 setting. Furthermore, most functionalities will connect interaction objects in activated()
 and disconnect them in deactivated().
 */
class QmitkFunctionality : public QObject
{
	Q_OBJECT
	bool available;

public:

	/*!
		\brief default constructor for a Functionality object
		@param parent the parent widget
		@param name the name of the functionality widget
		*/
	QmitkFunctionality(QObject *parent=0, const char *name=0, mitk::DataTreeIterator * dataIt=NULL);
		
	/*!
		default destructor
		*/
	virtual ~QmitkFunctionality();

	/*!
	\brief method for creating the widget containing the functionalities controls, like sliders, buttons etc.
	*/
	virtual QWidget * createControlWidget(QWidget *parent) = 0;

	/*!
	\brief method for creating the functionality's main widget
	*/
	virtual QWidget * createMainWidget(QWidget * parent) = 0;

	/*!
	\brief method for creating the connections of main and control widget
	*/
	virtual void createConnections() {};

	/*!
  \brief method for creating a QAction object, i.e., toolbar button and menu entry
  */
	virtual QAction * createAction(QActionGroup * parent) { return NULL; };

  /*!
  \brief method for defining the name of the functionality
  */
	virtual QString getFunctionalityName() = 0;

  /*!
  \brief called when a functionality becomes active/visible. Often, event-handlers are connected (e.g., 
  GlobalStateMachine::AddStateMachine()) in activated() and the connection is removed in deactivated()
  (e.g., GlobalStateMachine::RemoveStateMachine()).
  */
	virtual void activated();

  /*!
  \brief called when a functionality is deactivated, i.e., is no longer active/visible. Often, event-handlers are connected (e.g., 
  GlobalStateMachine::AddStateMachine()) in activated() and the connection is removed in deactivated()
  (e.g., GlobalStateMachine::RemoveStateMachine()).
  */
	virtual void deactivated();

	virtual bool isAvailable();

	/*!
	 \brief setter method for data tree attribute
	 @param it the reference to a data tree ieterator object
	 */
	void setDataTree(mitk::DataTreeIterator * it);

	/*!
	 \brief getter for dataTree attribute. It returns the 
					reference to a data tree iterator object
	 */
	mitk::DataTreeIterator * getDataTree();


  /*!
	 \brief initialize the widgets
	 */
  void initWidget(mitk::DataTreeIterator* it,
				   QmitkSelectableGLWidget* widget,
				   const Vector3f& origin,
				   const Vector3f& right,
				   const Vector3f& bottom);
signals:
	void signal_dummy();
	void availabilityChanged(QmitkFunctionality*);
	void availabilityChanged();


protected:
	virtual void setAvailability(bool available);


protected:
	/*!
			a reference to a data tree iterator object
	 */
	mitk::DataTreeIterator*	m_DataTreeIterator;


};

#endif // !defined(AFX_QUSFUNCTIONALITY_H__1DC0BA6E_9B8D_4D63_8A63_5B661CE33712__INCLUDED_)
