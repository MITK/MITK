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

#include <cherryIPartListener.h>
#include <cherryISelection.h>
#include <cherryISelectionProvider.h>
#include <cherryIPreferencesService.h>
#include <cherryICherryPreferences.h>
#include <cherryISelectionListener.h>
#include <cherryIStructuredSelection.h>

#include <QmitkFunctionality.h>
#include <QmitkStandardViews.h>
#include <QmitkStdMultiWidgetEditor.h>
#include <mitkPointSetInteractor.h>

class QmitkPlanarFiguresTableModel;
class QGridLayout;
class QMainWindow;
class QToolBar;
class QLabel;
class QTableView;


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
    virtual void Visible();
    virtual void Hidden();

    ///
    /// Invoked when the DataManager selection changed
    ///
    virtual void SelectionChanged(cherry::IWorkbenchPart::Pointer part
      , cherry::ISelection::ConstPointer selection);
  
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
    void CopyToClipboard( bool checked = false ); 
    void PlanarFigureTableModelRowsInserted( const QModelIndex & parent, int start, int end );
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
  QGridLayout* m_Layout;
  QLabel* m_SelectedImage;
  QToolBar* m_DrawActionsToolBar;
  QTableView* m_PlanarFiguresTable;
  QmitkPlanarFiguresTableModel* m_PlanarFiguresModel;
  QPushButton* m_CopyToClipboard;

  /// cherry::SelectionChangedAdapter<QmitkPropertyListView> must be a friend to call
  friend struct cherry::SelectionChangedAdapter<QmitkMeasurement>;

  // Selection service
  cherry::IStructuredSelection::ConstPointer m_CurrentSelection;
  cherry::ISelectionListener::Pointer m_SelectionListener;

  // Selected image on which measurements will be performed
  mitk::DataTreeNode::Pointer m_SelectedImageNode;

  unsigned int m_LineCounter;
  unsigned int m_PathCounter;
  unsigned int m_AngleCounter;
  unsigned int m_FourPointAngleCounter;
  unsigned int m_EllipseCounter;
  unsigned int m_RectangleCounter;
  unsigned int m_PolygonCounter;

};

#endif // !defined(QMITK_MEASUREMENT_H__INCLUDED)
