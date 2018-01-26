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

#ifndef QmitkGibbsTrackingView_h
#define QmitkGibbsTrackingView_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_QmitkGibbsTrackingViewControls.h"

#include <mitkOdfImage.h>
#include <QThread>
#include <mitkFiberBundle.h>
#include <QTime>
#include <itkImage.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <itkDiffusionTensor3D.h>
#include <mitkTensorImage.h>

class QmitkGibbsTrackingView;

class QmitkTrackingWorker : public QObject
{
  Q_OBJECT

public:

  QmitkTrackingWorker(QmitkGibbsTrackingView* view);

public slots:

  void run();

private:

  QmitkGibbsTrackingView* m_View;
};

/*!
  \brief View for global fiber tracking (Gibbs tracking)
*/
typedef itk::Image< float, 3 >            FloatImageType;

namespace itk
{
template<class X>
class GibbsTrackingFilter;
}

class QmitkGibbsTrackingView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  typedef itk::Image<float,3>                             ItkFloatImageType;
  typedef itk::Vector<float, ODF_SAMPLING_SIZE>           OdfVectorType;
  typedef mitk::OdfImage::ItkOdfImageType                 ItkOdfImgType;
  typedef mitk::TensorImage::ItkTensorImageType           ItkTensorImage;
  typedef itk::GibbsTrackingFilter< ItkOdfImgType >       GibbsTrackingFilterType;

  static const std::string VIEW_ID;

  QmitkGibbsTrackingView();
  virtual ~QmitkGibbsTrackingView();

  virtual void CreateQtPartControl(QWidget *parent) override;

  ///
  /// Sets the focus to an internal widget.
  ///
  virtual void SetFocus() override;

signals:

protected slots:

  void StartGibbsTracking();
  void StopGibbsTracking();
  void AfterThread();                       ///< update gui etc. after tracking has finished
  void BeforeThread();                      ///< start timer etc.
  void TimerUpdate();
  void AdvancedSettings();                  ///< show/hide advanced tracking options
  void SaveTrackingParameters();            ///< save tracking parameters to xml file
  void LoadTrackingParameters();            ///< load tracking parameters from xml file

  /** update labels if parameters have changed */
  void SetParticleWidth(int value);
  void SetParticleLength(int value);
  void SetInExBalance(int value);
  void SetFiberLength(int value);
  void SetParticleWeight(int value);
  void SetStartTemp(int value);
  void SetEndTemp(int value);
  void SetCurvatureThreshold(int value);
  void SetRandomSeed(int value);
  void SetOutputFile();
  void UpdateGUI();             ///< update button activity etc. dpending on current datamanager selection

private:

  // Visualization & GUI
  void GenerateFiberBundle();   ///< generate fiber bundle from tracking output and add to datanode
  void UpdateTrackingStatus();  ///< update textual status display of the tracking process

  /// \brief called by QmitkAbstractView when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  /// \brief called when DataNode is removed to stop gibbs tracking after node is removed
  virtual void NodeRemoved(const mitk::DataNode * node) override;

  void UpdateIteraionsGUI(unsigned long iterations);    ///< update iterations label text

  Ui::QmitkGibbsTrackingViewControls* m_Controls;

  /** data objects */
  mitk::DataNode::Pointer       m_TrackingNode;     ///< actual node that is tracked
  mitk::FiberBundle::Pointer    m_FiberBundle;      ///< tracking output
  ItkFloatImageType::Pointer    m_MaskImage;        ///< used to reduce the algorithms search space. tracking only inside of the mask.
  mitk::TensorImage::Pointer    m_TensorImage;      ///< actual image that is tracked
  mitk::OdfImage::Pointer     m_OdfImage;       ///< actual image that is tracked
  ItkOdfImgType::Pointer      m_ItkOdfImage;    ///< actual image that is tracked
  ItkTensorImage::Pointer       m_ItkTensorImage;   ///< actual image that is tracked

  /** data nodes */
  mitk::DataNode::Pointer m_ImageNode;
  mitk::DataNode::Pointer m_MaskImageNode;
  mitk::DataNode::Pointer m_FiberBundleNode;

  /** flags etc. */
  bool            m_ThreadIsRunning;
  QTimer*         m_TrackingTimer;
  QTime           m_TrackingTime;
  unsigned long   m_ElapsedTime;
  QString         m_OutputFileName;

  /** global tracker and friends */
  itk::SmartPointer<GibbsTrackingFilterType> m_GlobalTracker;
  QmitkTrackingWorker m_TrackingWorker;
  QThread m_TrackingThread;
  friend class QmitkTrackingWorker;
};

#endif // _QMITKGibbsTrackingVIEW_H_INCLUDED
