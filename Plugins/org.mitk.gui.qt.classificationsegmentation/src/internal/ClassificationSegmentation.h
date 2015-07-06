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
  std::vector<mitk::Image::Pointer> m_ResultImageVector;
  std::vector<mitk::Image::Pointer> m_PostProcessingImageVector;

  bool m_BlockManualSegmentation = {false};
  QFutureWatcher<std::vector<mitk::Image::Pointer>> m_ManualSegmentationFutureWatcher;

  bool m_BlockPostProcessing = {false};
  QFutureWatcher<std::vector<mitk::Image::Pointer>> m_PostProcessingFutureWatcher;



protected slots:

  /// \brief Called when the user clicks the GUI button
  void DoAutomSegmentation();
  void DoSavePointsAsMask();
  void OnButtonCSFToggle(bool);
  void OnButtonLESToggle(bool);
  void OnButtonBRAToggle(bool);
  void OnButtonNoInteractionToggle(bool);

  void ManualSegmentationTrigger();
  std::vector<mitk::Image::Pointer> ManualSegmentationCallback();
  void ManualSegmentationFinished();

  void PostProcessingTrigger();
  std::vector<mitk::Image::Pointer> PostProcessingCallback();
  void PostProcessingFinished();

  void OnFeatureSettingsChanged();
  void OnPostProcessingSettingsChanged();
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

  void SampleClassMaskByPointSet(const mitk::Image::Pointer & ref_img, mitk::PointSet::Pointer & pointset, mitk::Image::Pointer & outimage);

  void ProcessFeatureImages(const mitk::Image::Pointer & raw_image, const mitk::Image::Pointer & mask_image);

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
                                   const QList<mitk::DataNode::Pointer>& nodes ) override;

  mitk::Image::Pointer CreateClassMaskByPointsetList(std::map<unsigned int, mitk::PointSet *> a_args);

  Ui::ClassificationSegmentationControls m_Controls;

  // Feature settings
  ctkSliderWidget * m_GaussSlider;
  ctkSliderWidget * m_HessianSlider;
  ctkSliderWidget * m_STInnerSlider;
  ctkSliderWidget * m_STOuterSlider;

  ctkSliderWidget * m_GaussCSFSlider;
  ctkSliderWidget * m_GaussLESSlider;
  ctkSliderWidget * m_GaussBRASlider;

  ctkSliderWidget * m_WeightCSFSlider;
  ctkSliderWidget * m_WeightLESSlider;
  ctkSliderWidget * m_WeightBRASlider;

  mitk::PointSetDataInteractor::Pointer m_PointSetDataInteractor;


  double GetEntropyForLabel(const mitk::Image::Pointer & raw_image, const mitk::Image::Pointer & mask_image)
  {

    //  mitk::ImageStatisticsCalculator::Pointer calculator = mitk::ImageStatisticsCalculator::New();
    //  calculator->SetImage(raw_image);
    //  calculator->SetImageMask(mask_image);
    //  calculator->SetMaskingMode(mitk::ImageStatisticsCalculator::MASKING_MODE_IMAGE);
    //  calculator->ComputeStatistics();
    //  calculator->GetHistogram()->Print(std::cout);

    itk::Image<short, 3>::Pointer itk_raw_image;
    mitk::CastToItkImage(raw_image, itk_raw_image);

    itk::Image<short, 3>::Pointer itk_mask_image;
    mitk::CastToItkImage(mask_image, itk_mask_image);

    itk::ImageRegionConstIterator<itk::Image<short, 3>> iit(itk_raw_image,itk_raw_image->GetLargestPossibleRegion());
    itk::ImageRegionConstIterator<itk::Image<short, 3>> mit(itk_mask_image,itk_mask_image->GetLargestPossibleRegion());

    std::map<short, unsigned int> map;

    while(!iit.IsAtEnd())
    {
      if(mit.Value()!= 0)
      {
        if(map.find(iit.Value()) == map.end())
          map[iit.Value()] = 0;
        ++map[iit.Value()];
      }
      ++iit;
      ++mit;
    }

    double entropy = 0;
    double sum = 0;

    for(auto const & pair : map)
      sum += pair.second;

    for(auto const & pair : map)
      entropy += pair.second/sum * std::log2(pair.second/sum);

    //  MITK_INFO << "Entropy " << - entropy;
    //  for(auto const & pair : map)
    //  {
    //    MITK_INFO << pair.first << ": " << pair.second;
    //  }
    return entropy;
  }

  double GetVarianceForLabel(const mitk::Image::Pointer & raw_image, const mitk::Image::Pointer & mask_image)
  {
    itk::Image<short, 3>::Pointer itk_raw_image;
    mitk::CastToItkImage(raw_image, itk_raw_image);

    itk::Image<short, 3>::Pointer itk_mask_image;
    mitk::CastToItkImage(mask_image, itk_mask_image);

    itk::ImageRegionConstIterator<itk::Image<short, 3>> iit(itk_raw_image,itk_raw_image->GetLargestPossibleRegion());
    itk::ImageRegionConstIterator<itk::Image<short, 3>> mit(itk_mask_image,itk_mask_image->GetLargestPossibleRegion());


    unsigned int num = 0 ;
    double sum = 0, sqsum = 0;

    while(!iit.IsAtEnd())
    {
      if(mit.Value()!= 0)
      {
        sum += iit.Value();
        sqsum += iit.Value() * iit.Value();
        num++;
      }
      ++iit;
      ++mit;
    }

    double varaince = (sqsum / num) - (sum/ num) * (sum/num);

    return varaince;
  }



};

#endif // ClassificationSegmentation_h
