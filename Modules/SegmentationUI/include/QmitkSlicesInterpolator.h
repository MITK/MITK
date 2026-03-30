/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSlicesInterpolator_h
#define QmitkSlicesInterpolator_h

#include <mitkDataNode.h>
#include <mitkDataStorage.h>
#include <mitkSegmentationInterpolationController.h>
#include <mitkSurfaceInterpolationController.h>
#include <mitkToolManager.h>
#include <MitkSegmentationUIExports.h>

#include <QWidget>
#include <map>

#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QGroupBox>
#include <QRadioButton>

#include <mitkVtkRepresentationProperty.h>
#include <vtkProperty.h>

// For running 3D interpolation in background
#include <QFuture>
#include <QFutureWatcher>
#include <QTimer>
#include <QtConcurrentRun>

namespace mitk
{
  class PlaneGeometry;
  class SliceNavigationController;
  class TimeNavigationController;
}

class QPushButton;
class QmitkRenderWindow;

/**
  \brief GUI for slices interpolation.

  \ingroup ToolManagerEtAl
  \ingroup Widgets

  \sa QmitkInteractiveSegmentation
  \sa mitk::SegmentationInterpolation

  While mitk::SegmentationInterpolation does the bookkeeping of interpolation
  (keeping track of which slices contain how much segmentation) and the algorithmic work,
  QmitkSlicesInterpolator is responsible to watch the GUI, to notice, which slice is currently
  visible. It triggers generation of interpolation suggestions and also triggers acception of
  suggestions.
*/
class MITKSEGMENTATIONUI_EXPORT QmitkSlicesInterpolator : public QWidget
{
  Q_OBJECT

public:
  QmitkSlicesInterpolator(QWidget *parent = nullptr, const char *name = nullptr);

  /**
    To be called once before real use.
    */
  void Initialize(mitk::ToolManager *toolManager, const QList<QmitkRenderWindow*>& windows);

  /** \brief Releases resources and disconnects from the tool manager and render windows. */
  void Uninitialize();

  ~QmitkSlicesInterpolator() override;

  /** \brief Sets the data storage used for interpolation feedback nodes. */
  void SetDataStorage(mitk::DataStorage::Pointer storage);

  /** \brief Returns the current data storage. */
  mitk::DataStorage *GetDataStorage();

  /** \brief Sets the active label value for which interpolation is performed. */
  void SetActiveLabelValue(mitk::MultiLabelSegmentation::LabelValueType labelValue);

  /**
    Just public because it is called by itk::Commands. You should not need to call this.
  */
  void OnToolManagerWorkingDataModified();

  /**
    Just public because it is called by itk::Commands. You should not need to call this.
  */
  void OnToolManagerReferenceDataModified();

  /** \brief Reacts to the time changed event. Public for itk::Command callbacks. */
  void OnTimeChanged(itk::Object *sender, const itk::EventObject &);

  /** \brief Reacts to the slice changed event. Public for itk::Command callbacks. */
  void OnSliceChanged(itk::Object *sender, const itk::EventObject &);


  /** \brief Handles deletion of a SliceNavigationController. Public for itk::Command callbacks. */
  void OnSliceNavigationControllerDeleted(const itk::Object *sender, const itk::EventObject &);

  /**
    Just public because it is called by itk::Commands. You should not need to call this.
  */
  void OnInterpolationInfoChanged(const itk::EventObject &);

  /**
    Just public because it is called by itk::Commands. You should not need to call this.
  */
  void OnInterpolationAborted(const itk::EventObject &);

  /**
    Just public because it is called by itk::Commands. You should not need to call this.
  */
  void OnSurfaceInterpolationInfoChanged(const itk::EventObject &);


private:
  /** \brief Sets the visibility of the 3D interpolation result in the render windows. */
  void Show3DInterpolationResult(bool);

  /** \brief Reacts to a change in the active label of the working segmentation image. */
  void OnActiveLabelChanged(mitk::Label::PixelType);

signals:

  void SignalRememberContourPositions(bool);
  void SignalShowMarkerNodes(bool);

public slots:

  virtual void setEnabled(bool);
  /**
    Call this from the outside to enable/disable interpolation
  */
  void EnableInterpolation(bool);

  /** \brief Enables or disables 3D surface interpolation. */
  void Enable3DInterpolation(bool);

  /**
    Call this from the outside to accept all interpolations
  */
  void FinishInterpolation(mitk::SliceNavigationController *slicer = nullptr);

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

  /** \brief Reinitializes 3D interpolation by re-reading the plane geometries of the segmentation image. */
  void OnReinit3DInterpolation();

  /*
   * Will trigger interpolation for all slices in given orientation (called from popup menu of
   * OnAcceptAllInterpolationsClicked)
   */
  void OnAcceptAllPopupActivated(QAction *action);

  /**
    Called on activation/deactivation
  */
  void OnInterpolationActivated(bool);

  /** \brief Called when 3D interpolation is activated or deactivated. */
  void On3DInterpolationActivated(bool);

  /** \brief Called when the interpolation method combo box selection changes. */
  void OnInterpolationMethodChanged(int index);

  /** \brief Called when the 2D interpolation radio button is toggled. */
  void On2DInterpolationEnabled(bool);
  /** \brief Called when the 3D interpolation radio button is toggled. */
  void On3DInterpolationEnabled(bool);
  /** \brief Called when the "no interpolation" radio button is toggled. */
  void OnInterpolationDisabled(bool);
  /** \brief Called when the "show position markers" checkbox is toggled. */
  void OnShowMarkers(bool);

  /** \brief Triggers 3D surface interpolation in a background thread. */
  void Run3DInterpolation();

  /**
   * \brief Called when the surface interpolation thread completes.
   *
   * Retrieves the interpolated surface data, renders it in the active color label,
   * and stores the surface information in the feedback node.
   */
  void OnSurfaceInterpolationFinished();

  /** \brief Starts the timer that periodically triggers interpolation updates. */
  void StartUpdateInterpolationTimer();

  /** \brief Stops the interpolation update timer. */
  void StopUpdateInterpolationTimer();

  /** \brief Updates the surface color to match the active label color. */
  void ChangeSurfaceColor();

protected:

  typedef std::map<QAction*, mitk::SliceNavigationController*> ActionToSliceDimensionMapType;
  const ActionToSliceDimensionMapType CreateActionToSlicer(const QList<QmitkRenderWindow*>& windows);
  ActionToSliceDimensionMapType m_ActionToSlicerMap;

  void AcceptAllInterpolations(mitk::SliceNavigationController *slicer);

  /**
    Retrieves the currently selected PlaneGeometry from a SlicedGeometry3D that is generated by a
    SliceNavigationController
    and calls Interpolate to further process this PlaneGeometry into an interpolation.

    \param e is a actually a mitk::SliceNavigationController::GeometrySliceEvent, sent by a SliceNavigationController
    \param sliceNavigationController the SliceNavigationController
        */
  bool TranslateAndInterpolateChangedSlice(const itk::EventObject &e,
                                           mitk::SliceNavigationController *sliceNavigationController);

  bool TranslateAndInterpolateChangedSlice(const mitk::TimeGeometry* timeGeometry);
  /**
    Given a PlaneGeometry, this method figures out which slice of the first working image (of the associated
    ToolManager)
    should be interpolated. The actual work is then done by our SegmentationInterpolation object.
   */
  void Interpolate(mitk::PlaneGeometry *plane);

  /**
    Called internally to update the interpolation suggestion. Finds out about the focused render window and requests an
    interpolation.
   */
  void UpdateVisibleSuggestion();

  void SetCurrentContourListID();

private:
  void InitializeWindow(QmitkRenderWindow* window);
  void HideAllInterpolationControls();
  void Show2DInterpolationControls(bool show);
  void Show3DInterpolationControls(bool show);
  void CheckSupportedImageDimension();
  void WaitForFutures();
  void NodeRemoved(const mitk::DataNode* node);

  mitk::SegmentationInterpolationController::Pointer m_Interpolator;
  mitk::SurfaceInterpolationController::Pointer m_SurfaceInterpolator;

  mitk::ToolManager::Pointer m_ToolManager;
  bool m_Initialized;

  unsigned int m_ControllerToTimeObserverTag;
  QHash<mitk::SliceNavigationController *, int> m_ControllerToSliceObserverTag;
  QHash<mitk::SliceNavigationController *, int> m_ControllerToDeleteObserverTag;

  unsigned int InterpolationInfoChangedObserverTag;
  unsigned int SurfaceInterpolationInfoChangedObserverTag;
  unsigned int InterpolationAbortedObserverTag;

  QGroupBox *m_GroupBoxEnableExclusiveInterpolationMode;
  QComboBox *m_CmbInterpolation;
  QPushButton *m_BtnApply2D;
  QPushButton *m_BtnApplyForAllSlices2D;
  QPushButton *m_BtnApply3D;

  QCheckBox *m_ChkShowPositionNodes;
  QPushButton *m_BtnReinit3DInterpolation;

  mitk::DataNode::Pointer m_FeedbackNode;
  mitk::DataNode::Pointer m_InterpolatedSurfaceNode;

  mitk::MultiLabelSegmentation *m_Segmentation;

  mitk::SliceNavigationController *m_LastSNC;
  unsigned int m_LastSliceIndex;

  mitk::TimePointType m_TimePoint;

  bool m_2DInterpolationEnabled;
  bool m_3DInterpolationEnabled;

  unsigned int m_numTimesLabelSetConnectionAdded;

  mitk::DataStorage::Pointer m_DataStorage;

  QFuture<void> m_Future;
  QFutureWatcher<void> m_Watcher;

  QFuture<void> m_ModifyFuture;
  QFutureWatcher<void> m_ModifyWatcher;

  QTimer *m_Timer;

  QFuture<void> m_PlaneFuture;
  QFutureWatcher<void> m_PlaneWatcher;

  mitk::Label::PixelType m_CurrentActiveLabelValue;

  bool m_FirstRun;
};

#endif
