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


#ifndef ClassificationSegmentation_h
#define ClassificationSegmentation_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_ClassificationSegmentationControls.h"
#include <mitkImage.h>
#include <itkConstNeighborhoodIterator.h>
#include <itkNeighborhoodFunctorImageFilter.h>
#include <itkFirstOrderStatisticsFeatureFunctor.h>

#include <mitkImageCast.h>
#include <vigra/tensorutilities.hxx>
#include <vigra/convolution.hxx>
#include <vigra/multi_array.hxx>
#include <mitkPointSet.h>
#include <itkHistogram.h>
#include <initializer_list>
#include <QFutureWatcher>
#include "QmitkPointListViewWidget.h"
#include <mitkPointSetDataInteractor.h>
/**
  \brief ClassificationSegmentation

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/

namespace mitk{
class VigraRandomForestClassifier;
}

//class QmitkPointListWidget;
class ctkSliderWidget;
class ClassificationSegmentation : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkPointListViewWidget * m_PointListWidget;
  std::vector<mitk::DataNode::Pointer> m_PointSetList;
  bool m_CalculateFeatures = {true};
  std::vector<mitk::Image::Pointer> m_FeatureImageVector;


  bool m_BlockManualSegmentation = {false};
  QFutureWatcher<std::vector<mitk::Image::Pointer>> m_ManualSegmentationFutureWatcher;
  QFuture<std::vector<mitk::Image::Pointer>> m_ManualsegmentationFuture;
  std::vector<mitk::Image::Pointer> DoManualSegmentation();

protected slots:

  /// \brief Called when the user clicks the GUI button
  void DoAutomSegmentation();
  void DoSavePointsAsMask();
  void OnButtonCSFToggle(bool);
  void OnButtonLESToggle(bool);
  void OnButtonBRAToggle(bool);
  void OnButtonNoInteractionToggle(bool);
  void OnSetupModiefied();
  void OnManualSegmentationFinished();
  void OnSliderPositionChanged();
  void OnInitializeSession(const mitk::DataNode*);


protected:

  typedef float MeasurementType;
  typedef itk::Statistics::Histogram< MeasurementType,
  itk::Statistics::DenseFrequencyContainer2 > HistogramType;


//  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart);
//  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart);

  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void SetFocus() override;

  mitk::DataNode::Pointer AddImageAsDataNode(const mitk::Image::Pointer & data_image, const std::string & name );

  void PostProcessing(mitk::Image::Pointer &prob1, mitk::Image::Pointer &prob2, mitk::Image::Pointer &prob3, mitk::Image::Pointer &mask);

  void SampleClassMaskByPointSet(const mitk::Image::Pointer & ref_img, mitk::PointSet::Pointer & pointset, mitk::Image::Pointer & outimage);

  double GetEntropyForLabel(const mitk::Image::Pointer & raw_image, const mitk::Image::Pointer & mask_image);
  double GetVarianceForLabel(const mitk::Image::Pointer & raw_image, const mitk::Image::Pointer & mask_image);

  void ProcessFeatureImages(const mitk::Image::Pointer & raw_image, const mitk::Image::Pointer & mask_image);

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
                                   const QList<mitk::DataNode::Pointer>& nodes ) override;

  mitk::Image::Pointer CreateClassMaskByPointsetList(std::map<unsigned int, mitk::PointSet *> a_args);

  Ui::ClassificationSegmentationControls m_Controls;

  ctkSliderWidget * m_GaussSlider;
  ctkSliderWidget * m_HessianSlider;
  ctkSliderWidget * m_STInnerSlider;
  ctkSliderWidget * m_STOuterSlider;

  mitk::PointSetDataInteractor::Pointer m_PointSetDataInteractor;


};

#endif // ClassificationSegmentation_h
