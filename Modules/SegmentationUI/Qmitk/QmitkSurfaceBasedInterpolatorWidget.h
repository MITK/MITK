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

#ifndef QmitkSurfaceBasedInterpolatorWidgetWidget_h_Included
#define QmitkSurfaceBasedInterpolatorWidgetWidget_h_Included

#include "MitkSegmentationUIExports.h"
#include "mitkDataNode.h"
#include "mitkDataStorage.h"
#include "mitkLabelSetImage.h"
#include "mitkSurfaceBasedInterpolationController.h"

#include <map>

#include <QWidget>

// For running 3D interpolation in background
#include <QFuture>
#include <QFutureWatcher>
#include <QTimer>
#include <QtConcurrentRun>

#include "ui_QmitkSurfaceBasedInterpolatorWidgetGUIControls.h"

namespace mitk
{
  class ToolManager;
}

/**
  \brief GUI for surface-based interpolation.

  \ingroup ToolManagerEtAl
  \ingroup Widgets

  \sa QmitkInteractiveSegmentation
  \sa mitk::SurfaceBasedInterpolationController

  There is a separate page describing the general design of QmitkInteractiveSegmentation: \ref
  QmitkInteractiveSegmentationTechnicalPage

  QmitkSurfaceBasedInterpolatorWidgetController is responsible to watch the GUI, to notice, which slice is currently
  visible. It triggers generation of interpolation suggestions and also triggers acception of suggestions.
*/
class MITKSEGMENTATIONUI_EXPORT QmitkSurfaceBasedInterpolatorWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkSurfaceBasedInterpolatorWidget(QWidget *parent = 0, const char *name = 0);
  virtual ~QmitkSurfaceBasedInterpolatorWidget();

  void SetDataStorage(mitk::DataStorage &storage);

  void OnToolManagerWorkingDataModified();

  /**
     Just public because it is called by itk::Commands. You should not need to call this.
   */
  void OnSurfaceInterpolationInfoChanged(const itk::EventObject &);

  /**
   * @brief Set the visibility of the interpolation
   */
  void ShowInterpolationResult(bool);

  Ui::QmitkSurfaceBasedInterpolatorWidgetGUIControls m_Controls;

public slots:

  /**
    \brief Reaction to "Start/Stop" button click
  */
  void OnToggleWidgetActivation(bool);

protected slots:

  void OnAcceptInterpolationClicked();

  void OnSurfaceInterpolationFinished();

  void OnRunInterpolation();

  void OnShowMarkers(bool);

  void StartUpdateInterpolationTimer();

  void StopUpdateInterpolationTimer();

  void ChangeSurfaceColor();

private:
  mitk::SurfaceBasedInterpolationController::Pointer m_SurfaceBasedInterpolatorController;

  mitk::ToolManager *m_ToolManager;

  bool m_Activated;

  unsigned int m_SurfaceInterpolationInfoChangedObserverTag;

  mitk::DataNode::Pointer m_InterpolatedSurfaceNode;
  mitk::DataNode::Pointer m_3DContourNode;

  mitk::DataStorage::Pointer m_DataStorage;

  mitk::LabelSetImage::Pointer m_WorkingImage;

  QFuture<void> m_Future;
  QFutureWatcher<void> m_Watcher;
  QTimer *m_Timer;
};

#endif
