/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-09-12 15:46:48 +0200 (Fr, 12 Sep 2008) $
Version:   $Revision: 15236 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef QmitkSlicesInterpolator_h_Included
#define QmitkSlicesInterpolator_h_Included

#include "mitkSliceNavigationController.h"
#include "QmitkExtExports.h"
#include "mitkSegmentationInterpolationController.h"
#include "mitkDataNode.h"
#include "mitkDataStorage.h"
#include "mitkWeakPointer.h"
#include "mitkSurfaceInterpolationController.h"

#include <QWidget>
#include <map>

#include <QRadioButton>
#include <QGroupBox>
#include <QCheckBox>

#include "mitkVtkRepresentationProperty.h"
#include "vtkProperty.h"

//For running 3D interpolation in background
#include <QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>
#include <QTimer>


namespace mitk
{
  class ToolManager;
  class PlaneGeometry;
}

class QmitkStdMultiWidget;
class QPushButton;
//Enhancement for 3D Interpolation
//class QRadioButton;
//class QGroupBox; 
//class QCheckBox;


/**
  \brief GUI for slices interpolation.

  \ingroup ToolManagerEtAl
  \ingroup Widgets

  \sa QmitkInteractiveSegmentation
  \sa mitk::SegmentationInterpolation
  
  There is a separate page describing the general design of QmitkInteractiveSegmentation: \ref QmitkInteractiveSegmentationTechnicalPage

  While mitk::SegmentationInterpolation does the bookkeeping of interpolation 
  (keeping track of which slices contain how much segmentation) and the algorithmic work, 
  QmitkSlicesInterpolator is responsible to watch the GUI, to notice, which slice is currently
  visible. It triggers generation of interpolation suggestions and also triggers acception of
  suggestions.

  \todo show/hide feedback on demand

  Last contributor: $Author: maleike $
*/
class QmitkExt_EXPORT QmitkSlicesInterpolator : public QWidget
{
  Q_OBJECT

  public:
    
    QmitkSlicesInterpolator(QWidget* parent = 0, const char* name = 0);

    /**
      To be called once before real use.
      */
    void Initialize(mitk::ToolManager* toolManager, QmitkStdMultiWidget* multiWidget);

    virtual ~QmitkSlicesInterpolator();

    void SetDataStorage( mitk::DataStorage& storage );
    mitk::DataStorage* GetDataStorage();

    /**
      Just public because it is called by itk::Commands. You should not need to call this.
    */
    void OnToolManagerWorkingDataModified();

    /**
      Just public because it is called by itk::Commands. You should not need to call this.
    */
    void OnToolManagerReferenceDataModified();

    /**
      Just public because it is called by itk::Commands. You should not need to call this.
    */
    void OnTransversalTimeChanged(itk::Object* sender, const itk::EventObject&);

    /**
      Just public because it is called by itk::Commands. You should not need to call this.
    */
    void OnSagittalTimeChanged(itk::Object* sender, const itk::EventObject&);

    /**
      Just public because it is called by itk::Commands. You should not need to call this.
    */
    void OnFrontalTimeChanged(itk::Object* sender, const itk::EventObject&);
 
    /**
      Just public because it is called by itk::Commands. You should not need to call this.
    */
    void OnTransversalSliceChanged(const itk::EventObject&);

    /**
      Just public because it is called by itk::Commands. You should not need to call this.
    */
    void OnSagittalSliceChanged(const itk::EventObject&);

    /**
      Just public because it is called by itk::Commands. You should not need to call this.
    */
    void OnFrontalSliceChanged(const itk::EventObject&);
    
    /**
      Just public because it is called by itk::Commands. You should not need to call this.
    */
    void OnInterpolationInfoChanged(const itk::EventObject&);

    /**
      Just public because it is called by itk::Commands. You should not need to call this.
    */
    void OnSurfaceInterpolationInfoChanged(const itk::EventObject&);

  signals:

    void SignalRememberContourPositions(bool);
    void SignalShowMarkerNodes(bool);
    void Signal3DInterpolationEnabled(bool);

  public slots:

    /**
      Call this from the outside to enable/disable interpolation
    */
   void EnableInterpolation(bool);

   void Enable3DInterpolation(bool);

    /**
      Call this from the outside to accept all interpolations
    */
   void FinishInterpolation(int windowID = -1);

  protected slots:

    /**
      Reaction to button clicks.
    */
    void OnAcceptInterpolationClicked();

    /*
      Opens popup to ask about which orientation should be interpolated
    */
    void OnAcceptAllInterpolationsClicked();

    /*
     Reaction to button clicks
    */
    void OnAccept3DInterpolationClicked();

    /*
     * Will trigger interpolation for all slices in given orientation (called from popup menu of OnAcceptAllInterpolationsClicked)
     */
    void OnAcceptAllPopupActivated(QAction* action);
    
    /**
      Called on activation/deactivation
    */
    void OnInterpolationActivated(bool);

    void On3DInterpolationActivated(bool);

    void OnMultiWidgetDeleted(QObject*);

    //Enhancement for 3D interpolation
    void On2DInterpolationEnabled(bool);
    void On3DInterpolationEnabled(bool);
    void OnInterpolationDisabled(bool);
    void OnShowMarkers(bool);

    void Run3DInterpolation();

    void SurfaceInterpolationFinished();

    void StartUpdateInterpolationTimer();

    void StopUpdateInterpolationTimer();

    void ChangeSurfaceColor();

  protected:

    const std::map<QAction*, unsigned int> createActionToSliceDimension();
    const std::map<QAction*, unsigned int> ACTION_TO_SLICEDIMENSION;

    void AcceptAllInterpolations(unsigned int windowID);
        
    /**
      Retrieves the currently selected PlaneGeometry from a SlicedGeometry3D that is generated by a SliceNavigationController
      and calls Interpolate to further process this PlaneGeometry into an interpolation.

      \param e is a actually a mitk::SliceNavigationController::GeometrySliceEvent, sent by a SliceNavigationController
      \param windowID is 2 for transversal, 1 for frontal, 0 for sagittal (similar to sliceDimension in other methods)
          */
    bool TranslateAndInterpolateChangedSlice(const itk::EventObject& e, unsigned int windowID);
    
    /**
      Given a PlaneGeometry, this method figures out which slice of the first working image (of the associated ToolManager)
      should be interpolated. The actual work is then done by our SegmentationInterpolation object.
     */
    void Interpolate( mitk::PlaneGeometry* plane, unsigned int timeStep );

    //void InterpolateSurface();
    
    /**
      Called internally to update the interpolation suggestion. Finds out about the focused render window and requests an interpolation.
     */
    void UpdateVisibleSuggestion();
    
    /**
     * Tries to figure out the slice position and orientation for a given render window.
     * \param windowID is 2 for transversal, 1 for frontal, 0 for sagittal (similar to sliceDimension in other methods)
     * \return false if orientation could not be determined
     */
    bool GetSliceForWindowsID(unsigned windowID, int& sliceDimension, int& sliceIndex);

    mitk::SegmentationInterpolationController::Pointer m_Interpolator;
    mitk::SurfaceInterpolationController::Pointer m_SurfaceInterpolator;

    QmitkStdMultiWidget* m_MultiWidget;
    mitk::ToolManager* m_ToolManager;
    bool m_Initialized;

    unsigned int TSliceObserverTag;
    unsigned int SSliceObserverTag;
    unsigned int FSliceObserverTag;
    unsigned int TTimeObserverTag;
    unsigned int STimeObserverTag;
    unsigned int FTimeObserverTag;
    unsigned int InterpolationInfoChangedObserverTag;
    unsigned int SurfaceInterpolationInfoChangedObserverTag;

    QPushButton* m_BtnAcceptInterpolation;
    QPushButton* m_BtnAcceptAllInterpolations;

    //Enhancement for 3D Surface Interpolation
    QRadioButton* m_RBtnEnable2DInterpolation;
    QRadioButton* m_RBtnEnable3DInterpolation;
    QRadioButton* m_RBtnDisableInterpolation;
    QGroupBox* m_GroupBoxEnableExclusiveInterpolationMode;
    QPushButton* m_BtnAccept3DInterpolation;
    QCheckBox* m_CbShowMarkers;

    mitk::DataNode::Pointer m_FeedbackNode;
    mitk::DataNode::Pointer m_InterpolatedSurfaceNode;
    mitk::DataNode::Pointer m_3DContourNode;

    mitk::Image* m_Segmentation;
    unsigned int m_LastSliceDimension;
    unsigned int m_LastSliceIndex;
    
    std::vector<unsigned int> m_TimeStep; // current time step of the render windows

    bool m_2DInterpolationEnabled;
    bool m_3DInterpolationEnabled;
    //unsigned int m_CurrentListID;

    mitk::WeakPointer<mitk::DataStorage> m_DataStorage;

    QFuture<void> m_Future;
    QFutureWatcher<void> m_Watcher;
    QTimer* m_Timer;
};

#endif

