/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
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

#include <QmitkFunctionality.h>
#include <QmitkStandardViews.h>
#include <QmitkStdMultiWidgetEditor.h>
#include <mitkPointSetInteractor.h>

class QGridLayout;
class QMainWindow;
class QToolBar;
class QTableWidget;

/*!
\brief Measurement
Allows to measure distances, angles, etc.

\sa QmitkFunctionality
\ingroup org_mitk_gui_qt_measurement_internal
*/
class QmitkMeasurement : public QObject, public QmitkFunctionality
{
  Q_OBJECT

  public:
    QmitkMeasurement();
    virtual ~QmitkMeasurement();

  public:
    void CreateQtPartControl(QWidget* parent);
    virtual void Activated();
    virtual void Deactivated();
  
  protected:
    ///
    /// Clears m_PointSetInteractor and m_CurrentPointSetNode
    ///
    void Reset();

    ///# draw actions
  protected slots:
    void ActionDrawLineTriggered( bool checked = false );
    void ActionDrawPathTriggered( bool checked = false );
    void ActionDrawAngleTriggered( bool checked = false );
    void ActionDrawFourPointAngleTriggered( bool checked = false );
    void ActionDrawEllipseTriggered( bool checked = false );
    void ActionDrawRectangleTriggered( bool checked = false );
    void ActionDrawPolygonTriggered( bool checked = false );
    void ActionDrawArrowTriggered( bool checked = false );
    void ActionDrawTextTriggered( bool checked = false ); 

  // fields
  protected:
    ///
    /// Interactor for performing the measurements.
    ///
    mitk::PointSetInteractor::Pointer m_PointSetInteractor;

    ///
    /// Node representing the PointSet last created. It is used to delete unused point sets.
    ///
    mitk::DataTreeNode::Pointer m_CurrentPointSetNode;

  // widgets
  protected:
    QMainWindow* m_MainWindow;
    QGridLayout* m_Layout;
    QToolBar* m_DrawActionsToolBar;
    QToolBar* m_DrawActionsMainWindowToolBar;
    QTableWidget* m_DrawItemsTableWidget;
};

#endif // !defined(QMITK_MEASUREMENT_H__INCLUDED)
