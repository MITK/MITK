// QmitkSimpleExampleFunctionality.h: interface for the QmitkSimpleExampleFunctionality class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QMITKSIMPLEEXAMPLEFUNCTIONALITY_H__1DC0BA6E_9B8D_4D63_8A63_5B661CE33712__INCLUDED_)
#define AFX_QMITKSIMPLEEXAMPLEFUNCTIONALITY_H__1DC0BA6E_9B8D_4D63_8A63_5B661CE33712__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "QmitkFunctionality.h"

#include <mitkOperationActor.h>

#include <mitkFloatProperty.h>
#include <mitkLookupTableProperty.h>

class QmitkStdMultiWidget;
class QmitkSimpleExampleControls;

/*!\class
\brief MITK example demonstrating how a new application functionality can be implemented

One needs to reimplement the methods createControlWidget(..), createMainWidget(..) 
and createAction(..) from QmitkFunctionality. A QmitkFctMediator object gets passed a reference of a 
functionality and positions the widgets in the application window controlled by a 
layout template.
*/
class QmitkSimpleExampleFunctionality : public QmitkFunctionality, public mitk::OperationActor
{
    Q_OBJECT
public:

  /*!
  \brief default constructor
  */
	QmitkSimpleExampleFunctionality(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIterator * dataIt = NULL);

  /*!
  \brief default destructor
  */
  virtual ~QmitkSimpleExampleFunctionality();

  /*!
  \brief method for creating the widget containing the application 
					controls, like sliders, buttons etc.
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
	@param parent the parent QWidget
  */
  virtual QAction * createAction(QActionGroup *parent);

	/*! 
		* \brief returns the name of this functionality object
		*/
  virtual QString getFunctionalityName();


	/*!
	 \brief  inits the widgets of a functionality, 
						i.e. initializing slider etc.
	 */
	void initWidgets();

    //##Documentation
    //## \brief Implementation of OperationActor-method. Used for scrolling
    //## through the slices by clicking into the images.
    //## 
    //## mitk::Operation is expected to be an mitk::PointOperation. 
    //## The slices of widgets 1,2,3 are moved so that the seed point is on them.
    virtual void ExecuteOperation(mitk::Operation* operation);

    virtual void activated();
protected slots:

	/*!
  qt slot for event processing from a slider control,
		controls transversal view
  */
  void selectSliceWidgetXY( int z );

  /*!
  qt slot for event processing from a slider control
			controls sagital view
  */
  void selectSliceWidgetXZ( int z );

  /*!
  qt slot for event processing from a slider control
			controls axial view
  */
  void selectSliceWidgetYZ( int z );

  /*!
  qt slot for event processing from a slider control
  */
  void selectSliceWidgetFP( int z );

protected:

	/*!
	* default main widget containing 4 windows showing 3 
	* orthogonal slices of the volume and a 3d render window
	*/
	QmitkStdMultiWidget * multiWidget;

	/*!
	* controls containing sliders for scrolling through the slices
	*/
	QmitkSimpleExampleControls * controls;
  

	/*!

	* ???

	*/

//	int count;

  

	/*!

	* property for opacity for showing image overlays.

	* This property can be adjusted over a slider in 

	* the control widget.

	* 

	*/

	mitk::FloatProperty::Pointer opacityprop;

  mitk::LookupTableProperty::Pointer lookupTableProp;

		
};

#endif // !defined(AFX_QMITKSIMPLEEXAMPLEFUNCTIONALITY_H__1DC0BA6E_9B8D_4D63_8A63_5B661CE33712__INCLUDED_)
