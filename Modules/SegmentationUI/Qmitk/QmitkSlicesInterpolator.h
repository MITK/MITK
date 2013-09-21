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
#include "SegmentationUIExports.h"
#include "mitkDataNode.h"
#include "mitkDataStorage.h"
//#include "mitkWeakPointer.h"
#include "mitkSurfaceInterpolationController.h"
#include "mitkSegmentationInterpolationController.h"
#include "mitkToolManager.h"
#include "mitkDiffSliceOperation.h"
#include "mitkContourModel.h"

#include <map>

#include <QWidget>
#include <QComboBox>
#include <QGroupBox>
#include <QCheckBox>

//For running 3D interpolation in background
#include <QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>
#include <QTimer>


namespace mitk
{
  class PlaneGeometry;
  class SliceNavigationController;
  class LabelSetImage;
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
class SegmentationUI_EXPORT QmitkSlicesInterpolator : public QWidget
{
  Q_OBJECT

  public:

    QmitkSlicesInterpolator(QWidget* parent = 0, const char* name = 0);

    /**
      Initializes the widget. To be called once before real use.
    */
    void Initialize(const QList<mitk::SliceNavigationController*> &controllers);

    /**
      Removal of observers.
    */
    void Uninitialize();

    virtual ~QmitkSlicesInterpolator();

    void SetDataStorage( mitk::DataStorage::Pointer storage );
    mitk::DataStorage* GetDataStorage();

    void OnWorkingImageModified(const itk::EventObject&);

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
    void OnSliceInterpolationInfoChanged(const itk::EventObject&);

    /**
      Just public because it is called by itk::Commands. You should not need to call this.
    */
    void OnSurfaceInterpolationInfoChanged(const itk::EventObject&);

    /**
     * @brief Set the visibility of the 3d interpolation
    */
    void Show3DInterpolationResult(bool);

    void EnableInterpolation(bool);

  signals:

    void SignalRememberContourPositions(bool);
    void SignalShowMarkerNodes(bool);

  protected slots:

    /**
      Reaction to button clicks.
    */
    void OnAcceptInterpolationClicked();

    /**
      Gets the working slice based on the last saved interaction
    */
    mitk::Image::Pointer GetWorkingSlice(const mitk::PlaneGeometry* planeGeometry);

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

    void ActivateInterpolation(bool);

    void OnInterpolationMethodChanged(int index);

    void OnShowMarkers(bool);

    void Run3DInterpolation();

    void OnSurfaceInterpolationFinished();

    void StartUpdateInterpolationTimer();

    void StopUpdateInterpolationTimer();

    void ChangeSurfaceColor();

  protected:

    typedef std::map<QAction*, mitk::SliceNavigationController*> ActionToSliceDimensionMapType;

    const ActionToSliceDimensionMapType CreateActionToSliceDimension();
    ActionToSliceDimensionMapType m_ActionToSliceDimensionMap;

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

    void Activate2DInterpolation(bool);

    void Activate3DInterpolation(bool);

private:

    void HideAllInterpolationControls();
    void Show2DInterpolationControls(bool show);
    void Show3DInterpolationControls(bool show);

    mitk::SegmentationInterpolationController::Pointer m_SliceInterpolatorController;
    mitk::SurfaceInterpolationController::Pointer m_SurfaceInterpolator;

    mitk::ToolManager::Pointer m_ToolManager;
    bool m_Initialized;

    QHash<mitk::SliceNavigationController*, int> m_ControllerToTimeObserverTag;
    QHash<mitk::SliceNavigationController*, int> m_ControllerToSliceObserverTag;
    QHash<mitk::SliceNavigationController*, int> m_ControllerToDeleteObserverTag;

    unsigned int m_InterpolationInfoChangedObserverTag;
    unsigned int m_SurfaceInterpolationInfoChangedObserverTag;

    mitk::DiffSliceOperation* m_doOperation;
    mitk::DiffSliceOperation* m_undoOperation;

    QGroupBox* m_GroupBoxEnableExclusiveInterpolationMode;
    QComboBox* m_CmbInterpolation;
    QPushButton* m_BtnApply2D;
    QPushButton* m_BtnApplyForAllSlices2D;
    QPushButton* m_BtnApply3D;
    QCheckBox* m_ChkShowPositionNodes;

    mitk::DataNode::Pointer m_FeedbackNode;
    mitk::ContourModel::Pointer m_FeedbackContour;
    mitk::DataNode::Pointer m_InterpolatedSurfaceNode;
    mitk::DataNode::Pointer m_3DContourNode;

    mitk::LabelSetImage::Pointer m_WorkingImage;

    mitk::SliceNavigationController* m_LastSNC;

    unsigned int m_LastSliceIndex;

    unsigned long m_WorkingImageObserverID;

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

