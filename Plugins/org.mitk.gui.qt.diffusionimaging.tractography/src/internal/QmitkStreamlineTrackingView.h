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

#ifndef QmitkStreamlineTrackingView_h
#define QmitkStreamlineTrackingView_h

#include <QmitkAbstractView.h>

#include "ui_QmitkStreamlineTrackingViewControls.h"

#include <mitkTensorImage.h>
#include <mitkDataStorage.h>
#include <mitkDataNode.h>
#include <mitkImage.h>
#include <itkImage.h>
#include <itkStreamlineTrackingFilter.h>
#include <Algorithms/TrackingHandlers/mitkTrackingHandlerTensor.h>
#include <Algorithms/TrackingHandlers/mitkTrackingHandlerPeaks.h>
#include <Algorithms/TrackingHandlers/mitkTrackingHandlerOdf.h>
#include <Algorithms/TrackingHandlers/mitkTrackingHandlerRandomForest.h>
#include <random>
#include <mitkPointSet.h>
#include <mitkPointSetShapeProperty.h>
#include <mitkTractographyForest.h>
#include <QThread>
#include <QTimer>
#include <QmitkStdMultiWidget.h>
#include <QmitkSliceNavigationListener.h>
#include <mitkILifecycleAwarePart.h>
#include <mitkStreamlineTractographyParameters.h>

class QmitkStreamlineTrackingView;

class QmitkStreamlineTrackingWorker : public QObject
{
  Q_OBJECT

public:

  QmitkStreamlineTrackingWorker(QmitkStreamlineTrackingView* view);

public slots:

  void run();

private:

  QmitkStreamlineTrackingView* m_View;
};

/*!
\brief View for tensor based deterministic streamline fiber tracking.
*/
class QmitkStreamlineTrackingView : public QmitkAbstractView, public mitk::ILifecycleAwarePart
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  typedef itk::Image< unsigned int, 3 >   ItkUintImgType;
  typedef itk::Image< unsigned char, 3 >  ItkUCharImageType;
  typedef itk::Image< float, 3 >          ItkFloatImageType;
  typedef itk::StreamlineTrackingFilter   TrackerType;

  QmitkStreamlineTrackingView();
  virtual ~QmitkStreamlineTrackingView();

  virtual void CreateQtPartControl(QWidget *parent) override;

  ///
  /// Sets the focus to an internal widget.
  ///
  virtual void SetFocus() override;

  TrackerType::Pointer              m_Tracker;
  QmitkStreamlineTrackingWorker     m_TrackingWorker;
  QThread                           m_TrackingThread;

  virtual void Activated() override;
  virtual void Deactivated() override;
  virtual void Visible() override;
  virtual void Hidden() override;

protected slots:

  void DoFiberTracking();   ///< start fiber tracking
  void UpdateGui();
  void ToggleInteractive();
  void DeleteTrackingHandler();
  void OnParameterChanged();
  void InteractiveSeedChanged(bool posChanged=false);
  void ForestSwitched();
  void OutputStyleSwitched();
  void AfterThread();                       ///< update gui etc. after tracking has finished
  void BeforeThread();                      ///< start timer etc.
  void TimerUpdate();
  void StopTractography();
  void OnSliceChanged();
  void SaveParameters();
  void LoadParameters();

protected:

  /// \brief called by QmitkAbstractView when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  Ui::QmitkStreamlineTrackingViewControls* m_Controls;

protected slots:

private:

  bool CheckAndStoreLastParams(QObject* obj);
  void StartStopTrackingGui(bool start);
  std::shared_ptr< mitk::StreamlineTractographyParameters > GetParametersFromGui();
  void ParametersToGui(mitk::StreamlineTractographyParameters& params);

  std::vector< itk::Point<float> >        m_SeedPoints;
  mitk::DataNode::Pointer                 m_ParentNode;
  mitk::DataNode::Pointer                 m_InteractiveNode;
  mitk::DataNode::Pointer                 m_InteractivePointSetNode;

  std::vector< mitk::DataNode::Pointer >  m_InputImageNodes; ///< input image nodes
  std::vector< mitk::Image::ConstPointer >     m_AdditionalInputImages;
  bool                                    m_FirstTensorProbRun;
  bool                                    m_FirstInteractiveRun;

  mitk::TrackingDataHandler*              m_TrackingHandler;
  bool                                    m_ThreadIsRunning;
  QTimer*                                 m_TrackingTimer;
  bool                                    m_DeleteTrackingHandler;
  QmitkSliceNavigationListener            m_SliceChangeListener;
  bool                                    m_Visible;
  mitk::DataNode::Pointer                 m_LastPrior;
  mitk::TrackingDataHandler*              m_TrackingPriorHandler;
  std::map< QString, std::string >        m_LastTractoParams;
  QString                                 m_ParameterFile;
};



#endif // _QMITKFIBERTRACKINGVIEW_H_INCLUDED

