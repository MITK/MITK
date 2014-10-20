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

#ifndef QmitkSlicesInterpolator_h_Included
#define QmitkSlicesInterpolator_h_Included

#include "mitkSliceNavigationController.h"
#include <MitkSegmentationUIExports.h>
#include "mitkSegmentationInterpolationController.h"
#include "mitkDataNode.h"
#include "mitkDataStorage.h"
#include "mitkWeakPointer.h"
#include "mitkSurfaceInterpolationController.h"
#include "mitkToolManager.h"

#include <QWidget>
#include <map>

#include <QComboBox>
#include <QFrame>
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
  class PlaneGeometry;
  class SliceNavigationController;
}

class QPushButton;


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
class MitkSegmentationUI_EXPORT QmitkSlicesInterpolator : public QWidget
{
  Q_OBJECT

  public:

    QmitkSlicesInterpolator(QWidget* parent = 0, const char* name = 0);

    /**
      To be called once before real use.
      */
    void Initialize(mitk::ToolManager* toolManager, const QList<mitk::SliceNavigationController*> &controllers);

    void Uninitialize();

    virtual ~QmitkSlicesInterpolator();

    void SetDataStorage( mitk::DataStorage::Pointer storage );
    mitk::DataStorage* GetDataStorage();

    /**
      Just public because it is called by itk::Commands. You should not need to call this.
    */
    void OnToolManagerWorkingDataModified();

    /**
      Just public because it is called by itk::Commands. You should not need to call this.
    */
    void OnToolManagerReferenceDataModified();

    void OnTimeChanged(itk::Object* sender, const itk::EventObject&);

    void OnSliceChanged(itk::Object* sender, const itk::EventObject&);

    void OnSliceNavigationControllerDeleted(const itk::Object *sender, const itk::EventObject& );

    /**
      Just public because it is called by itk::Commands. You should not need to call this.
    */
    void OnInterpolationInfoChanged(const itk::EventObject&);

    /**
      Just public because it is called by itk::Commands. You should not need to call this.
    */
    void OnSurfaceInterpolationInfoChanged(const itk::EventObject&);

    /**
     * @brief Set the visibility of the 3d interpolation
     */
    void Show3DInterpolationResult(bool);

  signals:

    void SignalRememberContourPositions(bool);
    void SignalShowMarkerNodes(bool);

  public slots:

    virtual void setEnabled( bool );
    /**
      Call this from the outside to enable/disable interpolation
    */
   void EnableInterpolation(bool);

   void Enable3DInterpolation(bool);



    /**
      Call this from the outside to accept all interpolations
    */
   void FinishInterpolation(mitk::SliceNavigationController* slicer = NULL);

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

    void OnReinit3DInterpolation();

    /*
     * Will trigger interpolation for all slices in given orientation (called from popup menu of OnAcceptAllInterpolationsClicked)
     */
    void OnAcceptAllPopupActivated(QAction* action);

    /**
      Called on activation/deactivation
    */
    void OnInterpolationActivated(bool);

    void On3DInterpolationActivated(bool);

    void OnInterpolationMethodChanged(int index);

    //Enhancement for 3D interpolation
    void On2DInterpolationEnabled(bool);
    void On3DInterpolationEnabled(bool);
    void OnInterpolationDisabled(bool);
    void OnShowMarkers(bool);

    void Run3DInterpolation();

    void OnSurfaceInterpolationFinished();

    void StartUpdateInterpolationTimer();

    void StopUpdateInterpolationTimer();

    void ChangeSurfaceColor();

  protected:

    const std::map<QAction*, mitk::SliceNavigationController*> createActionToSliceDimension();
    std::map<QAction*, mitk::SliceNavigationController*> ACTION_TO_SLICEDIMENSION;

    void AcceptAllInterpolations(mitk::SliceNavigationController* slicer);

    /**
      Retrieves the currently selected PlaneGeometry from a SlicedGeometry3D that is generated by a SliceNavigationController
      and calls Interpolate to further process this PlaneGeometry into an interpolation.

      \param e is a actually a mitk::SliceNavigationController::GeometrySliceEvent, sent by a SliceNavigationController
      \param slice the SliceNavigationController
          */
    bool TranslateAndInterpolateChangedSlice(const itk::EventObject& e, mitk::SliceNavigationController* slicer);

    /**
      Given a PlaneGeometry, this method figures out which slice of the first working image (of the associated ToolManager)
      should be interpolated. The actual work is then done by our SegmentationInterpolation object.
     */
    void Interpolate( mitk::PlaneGeometry* plane, unsigned int timeStep, mitk::SliceNavigationController *slicer );

    //void InterpolateSurface();

    /**
      Called internally to update the interpolation suggestion. Finds out about the focused render window and requests an interpolation.
     */
    void UpdateVisibleSuggestion();

    void SetCurrentContourListID();

private:

    void HideAllInterpolationControls();
    void Show2DInterpolationControls(bool show);
    void Show3DInterpolationControls(bool show);
    void CheckSupportedImageDimension();

    mitk::SegmentationInterpolationController::Pointer m_Interpolator;
    mitk::SurfaceInterpolationController::Pointer m_SurfaceInterpolator;

    mitk::ToolManager::Pointer m_ToolManager;
    bool m_Initialized;

    QHash<mitk::SliceNavigationController*, int> m_ControllerToTimeObserverTag;
    QHash<mitk::SliceNavigationController*, int> m_ControllerToSliceObserverTag;
    QHash<mitk::SliceNavigationController*, int> m_ControllerToDeleteObserverTag;

    unsigned int InterpolationInfoChangedObserverTag;
    unsigned int SurfaceInterpolationInfoChangedObserverTag;

    QGroupBox* m_GroupBoxEnableExclusiveInterpolationMode;
    QComboBox* m_CmbInterpolation;
    QPushButton* m_BtnApply2D;
    QPushButton* m_BtnApplyForAllSlices2D;
    QPushButton* m_BtnApply3D;
    QCheckBox* m_ChkShowPositionNodes;
    QPushButton* m_BtnReinit3DInterpolation;

    mitk::DataNode::Pointer m_FeedbackNode;
    mitk::DataNode::Pointer m_InterpolatedSurfaceNode;
    mitk::DataNode::Pointer m_3DContourNode;

    mitk::Image* m_Segmentation;

    mitk::SliceNavigationController* m_LastSNC;
    unsigned int m_LastSliceIndex;

    QHash<mitk::SliceNavigationController*, unsigned int> m_TimeStep;

    bool m_2DInterpolationEnabled;
    bool m_3DInterpolationEnabled;
    //unsigned int m_CurrentListID;

    mitk::DataStorage::Pointer m_DataStorage;

    QFuture<void> m_Future;
    QFutureWatcher<void> m_Watcher;
    QTimer* m_Timer;
};

#endif

