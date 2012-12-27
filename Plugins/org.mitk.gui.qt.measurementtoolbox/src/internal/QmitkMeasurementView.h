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

#ifndef QMITK_MEASUREMENT_H__INCLUDED
#define QMITK_MEASUREMENT_H__INCLUDED

/*
#include <berryIPartListener.h>
#include <berryISelection.h>
#include <berryISelectionProvider.h>
#include <berryIPreferencesService.h>
#include <berryIBerryPreferences.h>
#include <berryISelectionListener.h>
#include <berryIStructuredSelection.h>

#include <mitkWeakPointer.h>
#include <mitkPlanarFigure.h>
#include <mitkDataStorageSelection.h>
#include <mitkIZombieViewPart.h>

class QmitkPlanarFiguresTableModel;
class QGridLayout;
class QMainWindow;
class QToolBar;
class QLabel;
class QTableView;
class QTextBrowser;
class QActionGroup;
class QPushButton;

class vtkRenderer;
class vtkCornerAnnotation;
*/

#include <QmitkAbstractView.h>
#include <mitkILifecycleAwarePart.h>

/// forward declarations
struct QmitkMeasurementViewData;
namespace mitk
{
  class PlanarFigure;
}

///
/// A view for doing measurements in digital images by means of
/// mitk::Planarfigures which can represent drawing primitives (Lines, circles, ...).
/// The view consists of only three main elements:
/// 1. A toolbar for activating PlanarFigure drawing
/// 2. A textbrowser which shows details for the selected PlanarFigures
/// 3. A button for copying all details to the clipboard
///
class QmitkMeasurementView : public QmitkAbstractView
{
  Q_OBJECT

  public:

    static const std::string VIEW_ID;
    QmitkMeasurementView();
    virtual ~QmitkMeasurementView();

    void CreateQtPartControl(QWidget* parent);
    void SetFocus();

    virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part,
      const QList<mitk::DataNode::Pointer> &nodes);

    void NodeAdded(const mitk::DataNode* node);
    void NodeChanged(const mitk::DataNode* node);
    void NodeRemoved(const mitk::DataNode* node);

    void PlanarFigureSelected( itk::Object* object, const itk::EventObject& );
  protected slots:
    ///# draw actions
    void ActionDrawLineTriggered( bool checked = false );
    void ActionDrawPathTriggered( bool checked = false );
    void ActionDrawAngleTriggered( bool checked = false );
    void ActionDrawFourPointAngleTriggered( bool checked = false );
    void ActionDrawEllipseTriggered( bool checked = false );
    void ActionDrawRectangleTriggered( bool checked = false );
    void ActionDrawPolygonTriggered( bool checked = false );
    void CopyToClipboard( bool checked = false );

  private:
    void CreateConnections();
    mitk::DataNode::Pointer AddFigureToDataStorage(mitk::PlanarFigure* figure, const QString& name);
    void UpdateMeasurementText();
    void AddAllInteractors();
    void RemoveAllInteractors();
    mitk::DataNode::Pointer DetectTopMostVisibleImage();
    void EnableCrosshairNavigation();
    void DisableCrosshairNavigation();
    void PlanarFigureInitialized();
    void CheckForTopMostVisibleImage(mitk::DataNode* _NodeToNeglect=0);

    QmitkMeasurementViewData* d;
};

/*
  public:
    ///
    /// Just a shortcut
    ///
    typedef std::vector<mitk::DataNode::Pointer> DataNodes;

    ///
    /// Initialize pointers to 0. The rest is done in CreateQtPartControl()
    ///
    QmitkMeasurementView();

    ///
    /// Remove all event listener from DataStorage, DataStorageSelection, Selection Service
    ///
    virtual ~QmitkMeasurementView();


  public:
    ///
    /// Initializes all variables.
    /// Builds up GUI.
    ///
    void CreateQtPartControl(QWidget* parent);
    ///
    /// Set widget planes visibility to false.
    /// Show only axial view.
    /// Add an interactor to all PlanarFigures in the DataStorage (if they dont have one yet).
    /// Add their interactor to the global interaction.
    ///
    virtual void Activated();

    virtual void Deactivated();

    virtual void Visible();
    virtual void Hidden();

    ///
    /// Show widget planes and all renderwindows again.
    /// Remove all planar figure interactors from the global interaction.
    ///
    virtual void ActivatedZombieView(berry::IWorkbenchPartReference::Pointer zombieView);
    ///
    /// Invoked from a DataStorage selection
    ///
    virtual void NodeChanged(const mitk::DataNode* node);
    virtual void PropertyChanged(const mitk::DataNode* node, const mitk::BaseProperty* prop);
    virtual void NodeRemoved(const mitk::DataNode* node);
    virtual void NodeAddedInDataStorage(const mitk::DataNode* node);
    virtual void PlanarFigureInitialized();
    virtual void PlanarFigureSelected( itk::Object* object, const itk::EventObject& event );
    virtual void AddFigureToDataStorage(mitk::PlanarFigure* figure, const QString& name,
      const char *propertyKey = NULL, mitk::BaseProperty *property = NULL );

    ///
    /// Invoked when the DataManager selection changed.
    /// If an image is in the selection it will be set as the selected one for measurement,
    /// If a planarfigure is in the selection its parent image will be set as the selected one for measurement.
    /// All selected planarfigures will be added to m_SelectedPlanarFigures.
    /// Then PlanarFigureSelectionChanged is called
    ///
    virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer> &nodes);

  public slots:
    ///
    /// Called when the renderwindow gets deleted
    ///
    void OnRenderWindowDelete(QObject * obj = 0);

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

    bool AssertDrawingIsPossible(bool checked);

    void EnableCrosshairNavigation();
    void DisableCrosshairNavigation();

    void SetFocus();

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

    void ReproducePotentialBug(bool);
    // fields
  // widgets
protected:
  QAction* m_DrawLine;
  QAction* m_DrawPath;
  QAction* m_DrawAngle;
  QAction* m_DrawFourPointAngle;
  QAction* m_DrawEllipse;
  QAction* m_DrawRectangle;
  QAction* m_DrawPolygon;
  QToolBar* m_DrawActionsToolBar;
  QActionGroup* m_DrawActionsGroup;
  QTextBrowser* m_SelectedPlanarFiguresText;
  QPushButton* m_CopyToClipboard;
  vtkRenderer * m_MeasurementInfoRenderer;
  vtkCornerAnnotation *m_MeasurementInfoAnnotation;

  // Selection service
  /// berry::SelectionChangedAdapter<QmitkPropertyListView> must be a friend to call
  friend struct berry::SelectionChangedAdapter<QmitkMeasurementView>;
  berry::ISelectionListener::Pointer m_SelectionListener;

  mitk::DataStorageSelection::Pointer m_SelectedPlanarFigures;
  /// Selected image on which measurements will be performed
  ///
  mitk::DataStorageSelection::Pointer m_SelectedImageNode;
  mitk::DataNode::Pointer m_CurrentFigureNode;

  /// Counter variables to give a newly created Figure a unique name.
  ///
  unsigned int m_LineCounter;
  unsigned int m_PathCounter;
  unsigned int m_AngleCounter;
  unsigned int m_FourPointAngleCounter;
  unsigned int m_EllipseCounter;
  unsigned int m_RectangleCounter;
  unsigned int m_PolygonCounter;
  unsigned int m_EndPlacementObserverTag;
  unsigned int m_SelectObserverTag;
  unsigned int m_StartInteractionObserverTag;
  unsigned int m_EndInteractionObserverTag;
  bool m_Visible;
  bool m_CurrentFigureNodeInitialized;
  bool m_Activated;

  ///
  /// Saves the last renderwindow any info data was inserted
  ///
  QmitkRenderWindow* m_LastRenderWindow;

  private:
    void RemoveEndPlacementObserverTag();
 mitk::DataNode::Pointer DetectTopMostVisibleImage();
 */

#endif // QMITK_MEASUREMENT_H__INCLUDED
