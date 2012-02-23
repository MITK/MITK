/*=========================================================================
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
Version:   $Revision: 21975 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
template<class X, class Y>
class GibbsTrackingFilter;
}

class QmitkGibbsTrackingView : public QmitkFunctionality
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  typedef itk::Image<float,3> MaskImgType;

  typedef itk::Vector<float, QBALL_ODFSIZE> OdfVectorType;
  typedef itk::Image<OdfVectorType, 3> ItkQBallImgType;

  typedef itk::GibbsTrackingFilter<ItkQBallImgType, MaskImgType> GibbsTrackingFilterType;

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
  MaskImgType::Pointer        m_MaskImage;
  mitk::QBallImage::Pointer   m_QBallImage;
  ItkQBallImgType::Pointer    m_ItkQBallImage;

  // data nodes
  mitk::DataNode::Pointer m_QBallImageNode;
  mitk::DataNode::Pointer m_MaskImageNode;
  mitk::DataNode::Pointer m_FiberBundleNode;

  // flags etc.
  bool            m_ThreadIsRunning;
  QTimer*         m_TrackingTimer;
  QTime           m_TrackingTime;
  unsigned long   m_ElapsedTime;
  bool            m_QBallSelected;
  bool            m_FibSelected;
  unsigned long   m_Iterations;
  int             m_LastStep;
  QString         m_OutputFileName;
  int m_SaveCounter;

  // global tracker and friends
  itk::SmartPointer<GibbsTrackingFilterType> m_GlobalTracker;
  QmitkTrackingWorker m_TrackingWorker;
  QThread m_TrackingThread;

  friend class QmitkTrackingWorker;
};

#endif // _QMITKGibbsTrackingVIEW_H_INCLUDED

