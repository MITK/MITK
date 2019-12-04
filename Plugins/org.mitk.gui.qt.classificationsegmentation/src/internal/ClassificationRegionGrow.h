/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef ClassificationRegionGrow_h
#define ClassificationRegionGrow_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_ClassificationRegionGrowControls.h"
#include <mitkImage.h>
#include <itkConstNeighborhoodIterator.h>
#include <itkNeighborhoodFunctorImageFilter.h>
#include <itkFirstOrderStatisticsFeatureFunctor.h>

#include <mitkImageCast.h>
//#include <vigra/tensorutilities.hxx>
//#include <vigra/convolution.hxx>
//#include <vigra/multi_array.hxx>
#include <mitkPointSet.h>
#include <itkHistogram.h>
#include <QFutureWatcher>
#include "QmitkPointListViewWidget.h"
#include <mitkPointSetDataInteractor.h>

#include <mitkVigraRandomForestClassifier.h>
/**
\brief ClassificationRegionGrow

\warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

\sa QmitkAbstractView
\ingroup ${plugin_target}_internal
*/


//class QmitkPointListWidget;
class ctkSliderWidget;
class ClassificationRegionGrow : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  bool m_CalculateFeatures;

  std::vector<mitk::Image::Pointer> m_FeatureImageVector;
  std::vector<mitk::Image::Pointer> m_ResultImageVector;

  bool m_BlockManualSegmentation;
  QFutureWatcher<std::vector<mitk::Image::Pointer>> m_ManualSegmentationFutureWatcher;

  bool m_BlockPostProcessing;
  QFutureWatcher<std::vector<mitk::Image::Pointer>> m_PostProcessingFutureWatcher;

  public slots:

    /// \brief Called when the user clicks the GUI button
    void DoAutomSegmentation();
    void AddInputField();

    void RemoveItemFromLabelList();

    void OnFeatureSettingsChanged();
    void OnInitializeSession(const mitk::DataNode*);

protected:
  std::vector<itk::Index<3> > m_SegmentedLocations;
  std::vector<itk::Index<3> > m_SegmentedOrganLocations;



  typedef float MeasurementType;
  typedef itk::Statistics::Histogram< MeasurementType,
    itk::Statistics::DenseFrequencyContainer2 > HistogramType;


  void CreateQtPartControl(QWidget *parent) override;
  void SetFocus() override;

  mitk::DataNode::Pointer AddAsDataNode(const mitk::BaseData::Pointer & data_, const std::string & name );

  void ProcessFeatureImages(const mitk::Image::Pointer & raw_image);
  void TrainClassifier(const mitk::Image::Pointer & raw_image, const mitk::Image::Pointer & mask_image);
  void PredictSegmentation(const mitk::Image::Pointer & raw_image, const mitk::Image::Pointer & mask_image);

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
    const QList<mitk::DataNode::Pointer>& nodes ) override;

  Ui::ClassificationRegionGrowControls m_Controls;

  mitk::VigraRandomForestClassifier::Pointer m_Classifier;
};

#endif // ClassificationRegionGrow_h
