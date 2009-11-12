/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-03-21 14:46:24 +0100 (Sat, 21 Mar 2009) $
Version:   $Revision: 15412 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#if !defined(QMITK_MEASUREMENT_H__INCLUDED)
#define QMITK_MEASUREMENT_H__INCLUDED

//#include "QmitkFunctionality.h"

#include <QmitkFunctionality.h>
#include <QmitkStandardViews.h>
#include <QmitkStdMultiWidgetEditor.h>

#include <mitkPointSetInteractor.h>
//!mm
#include "ui_QmitkSimpleMeasurementControls.h"
//!

class QmitkStdMultiWidget;
//!mm
//class QmitkSimpleMeasurementControls;
//!

/*!
\brief SimpleMeasurement
Allows to measure distances, angles, etc.

\sa QmitkFunctionality
\ingroup org_mitk_gui_qt_simplemeasurement_internal
*/
class QmitkSimpleMeasurement : public QObject, public QmitkFunctionality
{
  Q_OBJECT


  public:
    virtual void StdMultiWidgetAvailable(QmitkStdMultiWidget& stdMultiWidget);
    virtual void StdMultiWidgetNotAvailable();

    virtual ~QmitkSimpleMeasurement();

  protected:
    // Pseudo Ctor
    void CreateQtPartControl(QWidget* parent);

  public:
  /*!
  \brief default constructor
  */
  //!mm
  //QmitkSimpleMeasurement(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);
  // use this function as constructor
  //!

  /*!
  \brief default destructor
  */
  //virtual ~QmitkSimpleMeasurement();

  /*!
  \brief method for creating the widget containing the application   controls, like sliders, buttons etc.
  */
  //virtual QWidget * CreateControlWidget(QWidget *parent);

  /*!
  \brief method for creating the applications main widget
  */
  //virtual QWidget * CreateMainWidget(QWidget * parent);

  /*!
  \brief method for creating the connections of main and control widget
  */
  virtual void CreateConnections();

  /*!
  \brief method for creating an QAction object, i.e. button & menu entry  @param parent the parent QWidget
  */
  //virtual QAction * CreateAction(QActionGroup *parent);

  virtual void Activated();
  virtual void Deactivated();
protected slots:
  void TreeChanged();

  void AddDistanceSimpleMeasurement();

  void AddAngleSimpleMeasurement();

  void AddPathSimpleMeasurement();

protected:
  // save the parent widget of our UI
  QWidget* m_Parent;

  //mitk::DataTreeIteratorClone m_DataTreeIterator;

  /*!
  * default main widget containing 4 windows showing 3
  * orthogonal slices of the volume and a 3d render window
  */
  //QmitkStdMultiWidget * m_MultiWidget;

  /*!
  * controls containing sliders for scrolling through the slices
  */
  Ui::QmitkSimpleMeasurementControls * m_Controls;

  /*
   * Interactor for performing the simplemeasurements.
   */
  mitk::PointSetInteractor::Pointer m_PointSetInteractor;

  /*
   * Node representing the PointSet last created. It is used to delete empty point sets.
   */
  mitk::DataTreeNode::Pointer m_CurrentPointSetNode;
};

#endif // !defined(QMITK_MEASUREMENT_H__INCLUDED)
