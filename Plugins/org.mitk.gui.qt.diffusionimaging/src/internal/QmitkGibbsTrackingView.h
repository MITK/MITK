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

#include <QmitkFunctionality.h>

#include "ui_QmitkGibbsTrackingViewControls.h"

#include <mitkQBallImage.h>
#include <QThread>
#include <mitkFiberBundleX.h>
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
  \brief QmitkGibbsTrackingView

  \warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.

  \sa QmitkFunctionality
  \ingroup Functionalities
*/
typedef itk::Image< float, 3 >            FloatImageType;

namespace itk
{
template<class X>
class GibbsTrackingFilter;
}

class QmitkGibbsTrackingView : public QmitkFunctionality
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  typedef itk::Image<float,3>                               ItkFloatImageType;
  typedef itk::Vector<float, QBALL_ODFSIZE>                 OdfVectorType;
  typedef itk::Image<OdfVectorType, 3>                      ItkQBallImgType;
  typedef itk::Image< itk::DiffusionTensor3D<float>, 3 >    ItkTensorImage;
  typedef itk::GibbsTrackingFilter< ItkQBallImgType >       GibbsTrackingFilterType;

  static const std::string VIEW_ID;

  QmitkGibbsTrackingView();
  virtual ~QmitkGibbsTrackingView();

  virtual void CreateQtPartControl(QWidget *parent);

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

signals:

protected slots:

  void StartGibbsTracking();
  void StopGibbsTracking();
  void AfterThread();
  void BeforeThread();
  void TimerUpdate();
  void SetMask();
  void AdvancedSettings();
  void SaveTrackingParameters();
  void LoadTrackingParameters();
  void SetIterations(int value);
  void SetParticleWidth(int value);
  void SetParticleLength(int value);
  void SetInExBalance(int value);
  void SetFiberLength(int value);
  void SetParticleWeight(int value);
  void SetStartTemp(int value);
  void SetEndTemp(int value);
  void SetCurvatureThreshold(int value);
  void SetOutputFile();

private:

  // Visualization & GUI
  void GenerateFiberBundle(bool smoothFibers);
  void UpdateGUI();
  void UpdateTrackingStatus();

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

  template<class InputImageType>
  void CastToFloat(InputImageType* image, typename mitk::Image::Pointer outImage);

  void UpdateIteraionsGUI(unsigned long iterations);

  Ui::QmitkGibbsTrackingViewControls* m_Controls;
  QmitkStdMultiWidget* m_MultiWidget;

  // data objects
  mitk::FiberBundleX::Pointer m_FiberBundle;
  ItkFloatImageType::Pointer        m_MaskImage;
  mitk::TensorImage::Pointer  m_TensorImage;
  mitk::QBallImage::Pointer   m_QBallImage;
  ItkQBallImgType::Pointer    m_ItkQBallImage;
  ItkTensorImage::Pointer     m_ItkTensorImage;

  // data nodes
  mitk::DataNode::Pointer m_ImageNode;
  mitk::DataNode::Pointer m_MaskImageNode;
  mitk::DataNode::Pointer m_FiberBundleNode;

  // flags etc.
  bool            m_ThreadIsRunning;
  QTimer*         m_TrackingTimer;
  QTime           m_TrackingTime;
  unsigned long   m_ElapsedTime;
  unsigned long   m_Iterations;
  int             m_LastStep;
  QString         m_OutputFileName;

  // global tracker and friends
  itk::SmartPointer<GibbsTrackingFilterType> m_GlobalTracker;
  QmitkTrackingWorker m_TrackingWorker;
  QThread m_TrackingThread;

  friend class QmitkTrackingWorker;
};

#endif // _QMITKGibbsTrackingVIEW_H_INCLUDED

