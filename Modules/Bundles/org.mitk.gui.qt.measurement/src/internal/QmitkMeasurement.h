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

#include <mitkWeakPointer.h>
#include <mitkPlanarFigure.h>
#include <mitkDataStorageSelection.h>
#include "internal/mitkMeasurementSelectionProvider.h"

#include <QmitkFunctionality.h>
#include <QmitkStandardViews.h>
#include <QmitkStdMultiWidgetEditor.h>

class QmitkPlanarFiguresTableModel;
class QGridLayout;
class QMainWindow;
class QToolBar;
class QLabel;
class QTableView;
class QTextBrowser;

class vtkRenderer;
class vtkCornerAnnotation;


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
    typedef std::vector<mitk::DataTreeNode::Pointer> DataTreeNodes;

    QmitkMeasurement();
    virtual ~QmitkMeasurement();

  public:

    void CreateQtPartControl(QWidget* parent);
    virtual void Visible();
    virtual void Hidden();
    virtual void NodeChanged(const mitk::DataTreeNode* node);
    virtual void NodeRemoved(const mitk::DataTreeNode* node);
    virtual void NodeAddedInDataStorage(const mitk::DataTreeNode* node);
    virtual void PlanarFigureInitialized();
    virtual void AddFigureToDataStorage(mitk::PlanarFigure* figure, const QString& name);

    ///
    /// Invoked when the DataManager selection changed.
    /// If an image is in the selection this will be set as the selected one for measurement,
    /// If a planarfigure is in the selection its parent image will be set as the selected one for measurement.
    /// All selected planarfigures will be added to m_SelectedPlanarFigures.
    /// Then PlanarFigureSelectionChanged is called
    ///
    virtual void SelectionChanged(cherry::IWorkbenchPart::Pointer part
      , cherry::ISelection::ConstPointer selection);


  protected:
    ///
    /// Prints all features of the selected PlanarFigures into the TextBrowser.
    /// For the last figure in the selection list:
    ///  - Go to the corresponding slice and show figure
    ///  - Draw info text on the bottom right side of the corresponding renderwindow
    ///
    void PlanarFigureSelectionChanged();
    /// Draws a string on the bottom left side of the render window
    ///
    void SetMeasurementInfoToRenderWindow(const QString& text, QmitkRenderWindow* _RenderWindow);

    bool AssertDrawingIsPossible(bool checked) const;

  protected slots:
    ///# draw actions
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
    // fields
  // widgets
protected:
  QGridLayout* m_Layout;
  QLabel* m_SelectedImage;
  QAction* m_DrawLine;
  QAction* m_DrawPath;
  QAction* m_DrawAngle;
  QAction* m_DrawFourPointAngle;
  QAction* m_DrawEllipse;
  QAction* m_DrawRectangle;
  QAction* m_DrawPolygon;
  QToolBar* m_DrawActionsToolBar;
  QTextBrowser* m_SelectedPlanarFiguresText;
  QPushButton* m_CopyToClipboard;
  vtkRenderer * m_MeasurementInfoRenderer;
  vtkCornerAnnotation *m_MeasurementInfoAnnotation;

  // Selection service
  /// cherry::SelectionChangedAdapter<QmitkPropertyListView> must be a friend to call
  friend struct cherry::SelectionChangedAdapter<QmitkMeasurement>;
  cherry::ISelectionListener::Pointer m_SelectionListener;
  mitk::MeasurementSelectionProvider::Pointer m_SelectionProvider;

  mitk::DataStorageSelection::Pointer m_SelectedPlanarFigures;
  /// Selected image on which measurements will be performed
  ///
  mitk::DataStorageSelection::Pointer m_SelectedImageNode;
  mitk::WeakPointer<mitk::DataTreeNode> m_CurrentFigureNode;

  /// Counter variables to give a newly created Figure a unique name.
  ///
  unsigned int m_LineCounter;
  unsigned int m_PathCounter;
  unsigned int m_AngleCounter;
  unsigned int m_FourPointAngleCounter;
  unsigned int m_EllipseCounter;
  unsigned int m_RectangleCounter;
  unsigned int m_PolygonCounter;
  unsigned int m_InitializedObserverTag;
  bool m_Visible;

};

#endif // !defined(QMITK_MEASUREMENT_H__INCLUDED)
