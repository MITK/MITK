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

#ifndef QmitkSliceBasedInterpolatorWidget_h_Included
#define QmitkSliceBasedInterpolatorWidget_h_Included

#include "MitkSegmentationUIExports.h"
#include "mitkDataStorage.h"
#include "mitkSliceBasedInterpolationController.h"

#include <map>

#include <QWidget>

#include "ui_QmitkSliceBasedInterpolatorWidgetGUIControls.h"

namespace mitk
{
  class PlaneGeometry;
  class SliceNavigationController;
  class LabelSetImage;
  class ToolManager;
  class DiffSliceOperation;
}

/**
  \brief GUI for slices interpolation.

  \ingroup ToolManagerEtAl
  \ingroup Widgets

  \sa QmitkInteractiveSegmentation
  \sa mitk::SegmentationInterpolation

  There is a separate page describing the general design of QmitkInteractiveSegmentation: \ref
  QmitkInteractiveSegmentationTechnicalPage

  While mitk::SegmentationInterpolation does the bookkeeping of interpolation
  (keeping track of which slices contain how much segmentation) and the algorithmic work,
  QmitkSliceBasedInterpolatorWidget is responsible to watch the GUI, to notice, which slice is currently
  visible. It triggers generation of interpolation suggestions and also triggers acception of
  suggestions.

  \todo show/hide feedback on demand

  Last contributor: $Author: maleike $
*/

class MITKSEGMENTATIONUI_EXPORT QmitkSliceBasedInterpolatorWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkSliceBasedInterpolatorWidget(QWidget *parent = 0, const char *name = 0);
  ~QmitkSliceBasedInterpolatorWidget() override;

  void SetDataStorage(mitk::DataStorage &storage);

  /**
    Sets the slice navigation controllers for getting slice changed events from the views.
  */
  void SetSliceNavigationControllers(const QList<mitk::SliceNavigationController *> &controllers);

  void OnToolManagerWorkingDataModified();

  void OnTimeChanged(itk::Object *sender, const itk::EventObject &);

  void OnSliceChanged(itk::Object *sender, const itk::EventObject &);

  void OnSliceNavigationControllerDeleted(const itk::Object *sender, const itk::EventObject &);

  /**
    Just public because it is called by itk::Commands. You should not need to call this.
  */
  void OnSliceInterpolationInfoChanged(const itk::EventObject &);

  Ui::QmitkSliceBasedInterpolatorWidgetGUIControls m_Controls;

signals:

  void signalSliceBasedInterpolationEnabled(bool);

public slots:

  /**
    \brief Reaction to "Start/Stop" button click
  */
  void OnToggleWidgetActivation(bool);

protected slots:

  /**
    \brief Reaction to "Accept Current Slice" button click.
  */
  void OnAcceptInterpolationClicked();

  /*
    \brief Reaction to "Accept All Slices" button click.
    Opens popup to ask about which orientation should be interpolated
  */
  void OnAcceptAllInterpolationsClicked();

  /*
    \brief Called from popup menu of OnAcceptAllInterpolationsClicked()
     Will trigger interpolation for all slices in given orientation
  */
  void OnAcceptAllPopupActivated(QAction *action);

protected:
  typedef std::map<QAction *, mitk::SliceNavigationController *> ActionToSliceDimensionMapType;

  const ActionToSliceDimensionMapType CreateActionToSliceDimension();

  ActionToSliceDimensionMapType m_ActionToSliceDimensionMap;

  void AcceptAllInterpolations(mitk::SliceNavigationController *slicer);

  void WaitCursorOn();

  void WaitCursorOff();

  void RestoreOverrideCursor();

  /**
    Gets the working slice based on the given plane geometry and last saved interaction

    \param planeGeometry a plane geometry
  */
  mitk::Image::Pointer GetWorkingSlice(const mitk::PlaneGeometry *planeGeometry);

  /**
    Retrieves the currently selected PlaneGeometry from a SlicedGeometry3D that is generated by a
    SliceNavigationController
    and calls Interpolate to further process this PlaneGeometry into an interpolation.

    \param e is a actually a mitk::SliceNavigationController::GeometrySliceEvent, sent by a SliceNavigationController
    \param slice the SliceNavigationController
  */
  void TranslateAndInterpolateChangedSlice(const itk::EventObject &e, mitk::SliceNavigationController *slicer);

  /**
    Given a PlaneGeometry, this method figures out which slice of the first working image (of the associated
    ToolManager)
    should be interpolated. The actual work is then done by our SegmentationInterpolation object.
  */
  void Interpolate(mitk::PlaneGeometry *plane, unsigned int timeStep, mitk::SliceNavigationController *slicer);

  /**
    Called internally to update the interpolation suggestion. Finds out about the focused render window and requests an
    interpolation.
   */
  void UpdateVisibleSuggestion();

private:
  mitk::SliceBasedInterpolationController::Pointer m_SliceInterpolatorController;

  mitk::ToolManager *m_ToolManager;

  bool m_Activated;

  template <typename TPixel, unsigned int VImageDimension>
  void WritePreviewOnWorkingImage(itk::Image<TPixel, VImageDimension> *target,
                                  const mitk::Image *source,
                                  int overwritevalue);

  QHash<mitk::SliceNavigationController *, int> m_ControllerToTimeObserverTag;
  QHash<mitk::SliceNavigationController *, int> m_ControllerToSliceObserverTag;
  QHash<mitk::SliceNavigationController *, int> m_ControllerToDeleteObserverTag;

  unsigned int m_InterpolationInfoChangedObserverTag;

  mitk::DiffSliceOperation *m_doOperation;
  mitk::DiffSliceOperation *m_undoOperation;

  mitk::DataNode::Pointer m_PreviewNode;
  mitk::Image::Pointer m_PreviewImage;

  mitk::LabelSetImage::Pointer m_WorkingImage;

  QHash<mitk::SliceNavigationController *, unsigned int> m_TimeStep;

  mitk::DataStorage::Pointer m_DataStorage;

  mitk::SliceNavigationController *m_LastSNC;

  unsigned int m_LastSliceIndex;
};

#endif