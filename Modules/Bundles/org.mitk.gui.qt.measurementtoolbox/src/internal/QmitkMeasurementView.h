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

#ifndef QMITK_MEASUREMENT_H__INCLUDED
#define QMITK_MEASUREMENT_H__INCLUDED

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


///
/// A view for doing measurements in digital images by means of
/// mitk::Planarfigures which can represent drawing primitives (Lines, circles, ...).
/// The view consists of only three main elements:
/// 1. A toolbar for activating PlanarFigure drawing
/// 2. A textbrowser which shows details for the selected PlanarFigures
/// 3. A button for copying all details to the clipboard
///
class QmitkMeasurementView : public QmitkFunctionality
{
  Q_OBJECT

  public:
    ///
    /// Just a shortcut
    ///
    typedef std::vector<mitk::DataNode::Pointer> DataNodes;

    ///
    /// Initialize pointers to 0. The rest is done in CreateQtPartControl()
    ///
    QmitkMeasurementView();

  QmitkMeasurementView(const QmitkMeasurementView& other)
  {
    Q_UNUSED(other)
    throw std::runtime_error("Copy constructor not implemented");
  }

    ///
    /// Remove all event listener from DataStorage, DataStorageSelection, Selection Service
    ///
    virtual ~QmitkMeasurementView();

    static const std::string VIEW_ID;

  public:
    ///
    /// Initializes all variables.
    /// Builds up GUI.
    ///
    void CreateQtPartControl(QWidget* parent);
    ///
    /// Set widget planes visibility to false.
    /// Show only transversal view.
    /// Add an interactor to all PlanarFigures in the DataStorage (if they dont have one yet).
    /// Add their interactor to the global interaction.
    ///
    virtual void Activated();
    ///
    /// Show widget planes and all renderwindows again.
    /// Remove all planar figure interactors from the global interaction.
    ///
    virtual void Deactivated();
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
    virtual void OnSelectionChanged(std::vector<mitk::DataNode*> nodes);

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

  ///
  /// Saves the last renderwindow any info data was inserted
  ///
  QmitkRenderWindow* m_LastRenderWindow;

  private:
 mitk::DataNode::Pointer DetectTopMostVisibleImage();

};

#endif // QMITK_MEASUREMENT_H__INCLUDED
