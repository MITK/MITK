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
// STD
#include <stack>


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "ClassificationRegionGrow.h"

// Qt
#include <QMessageBox>
#include <qgroupbox.h>
#include <qsignalmapper.h>

//mitk image
#include <mitkImage.h>
#include <QmitkDataStorageComboBox.h>
//#include <ctkSliderWidget.h>
//#include <mitkPointSet.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateOr.h>
#include "mitkVigraRandomForestClassifier.h"
#include "mitkCLUtil.h"
#include "qboxlayout.h"
#include <mitkIOUtil.h>

#include "Eigen/Dense"

#include <mitkImageAccessByItk.h>
#include <mitkSliceNavigationController.h>
#include <vnl/vnl_random.h>

#include <itkNeighborhoodFunctorImageFilter.h>
#include <itkFirstOrderStatisticsFeatureFunctor.h>
#include <itkNeighborhood.h>
#include <itkHessianMatrixEigenvalueImageFilter.h>
#include <itkStructureTensorEigenvalueImageFilter.h>
#include <itkLineHistogramBasedMassImageFilter.h>
#include <mitkHistogramGenerator.h>

#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionConstIterator.h>

//#include <mitkRandomForestAccurecyWeighting.h>
#include "mitkLabelSetImage.h"
//#include <vigra\multi_array.hxx>

#include <mitkLookupTable.h>
#include <mitkLookupTableProperty.h>

#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrentRun>
//#include <mitkPointSetDataInteractor.h>
#include <QmitkIOUtil.h>
const std::string ClassificationRegionGrow::VIEW_ID = "org.mitk.views.ClassificationRegionGrow";

void ClassificationRegionGrow::SetFocus()
{
  //  m_Controls.buttonPerformImageProcessing->setFocus();
}

void ClassificationRegionGrow::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  m_CalculateFeatures = true;
  m_BlockManualSegmentation = false;
  m_BlockPostProcessing = false;

  m_Controls.groupLearningParameter->setVisible(false);
  m_Controls.groupFeatureSelection->setVisible(false);

  QmitkDataStorageComboBox * cb_inputimage = new QmitkDataStorageComboBox(this->GetDataStorage(), mitk::TNodePredicateDataType<mitk::Image>::New());
  QmitkDataStorageComboBox * cb_maskimage= new QmitkDataStorageComboBox(this->GetDataStorage(),mitk::TNodePredicateDataType<mitk::LabelSetImage>::New());
  QmitkDataStorageComboBox * cb_baseimage = new QmitkDataStorageComboBox(this->GetDataStorage(), mitk::TNodePredicateDataType<mitk::LabelSetImage>::New());

  m_Controls.m_InputImageLayout->addWidget(cb_inputimage);
  m_Controls.m_MaskImageLayout->addWidget(cb_maskimage);
  m_Controls.StartingPointLayout->addWidget(cb_baseimage);
  m_Controls.addInputButton->setIcon(QIcon::fromTheme("list-add"));
  m_Controls.removeInputButton->setIcon(QIcon::fromTheme("edit-delete"));

  connect( cb_inputimage, SIGNAL(OnSelectionChanged(const mitk::DataNode*)), this, SLOT(OnInitializeSession(const mitk::DataNode*)));
  connect( cb_maskimage, SIGNAL(OnSelectionChanged(const mitk::DataNode*)), this, SLOT(OnInitializeSession(const mitk::DataNode*)));
  connect(m_Controls.SelectAdvancedParameter, SIGNAL(toggled(bool)), m_Controls.groupLearningParameter, SLOT(setVisible(bool)));
  connect(m_Controls.SelectAdvancedParameter, SIGNAL(toggled(bool)), m_Controls.groupFeatureSelection, SLOT(setVisible(bool)));
  connect(m_Controls.SelectSimpleParameters, SIGNAL(toggled(bool)), m_Controls.parameterWidget, SLOT(setVisible(bool)));
  connect(m_Controls.m_DoAutomaticSecmentation, SIGNAL( clicked()), this, SLOT(DoAutomSegmentation()));
  connect(m_Controls.removeInputButton, SIGNAL(clicked()), this, SLOT(RemoveItemFromLabelList()));
  connect(m_Controls.addInputButton, SIGNAL(clicked()), this, SLOT(AddInputField()));

  connect(m_Controls.UseIntensity, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
  connect(m_Controls.Gauss1, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
  connect(m_Controls.Gauss2, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
  connect(m_Controls.Gauss3, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
  connect(m_Controls.Gauss4, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
  connect(m_Controls.Gauss5, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
  connect(m_Controls.DoG1, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
  connect(m_Controls.DoG2, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
  connect(m_Controls.DoG3, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
  connect(m_Controls.DoG4, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
  connect(m_Controls.DoG5, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
  connect(m_Controls.LoG1, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
  connect(m_Controls.LoG2, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
  connect(m_Controls.LoG3, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
  connect(m_Controls.LoG4, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
  connect(m_Controls.LoG5, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
  connect(m_Controls.HoG1, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
  connect(m_Controls.HoG2, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
  connect(m_Controls.HoG3, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
  connect(m_Controls.HoG4, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
  connect(m_Controls.HoG5, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
  connect(m_Controls.LH1, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
  connect(m_Controls.LH2, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
  connect(m_Controls.LH3, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
  connect(m_Controls.LH4, SIGNAL(toggled(bool)), this, SLOT(OnFeatureSettingsChanged()));
}

void ClassificationRegionGrow::AddInputField()
{
  QmitkDataStorageComboBox * cb_inputimage = new QmitkDataStorageComboBox(this->GetDataStorage(), mitk::TNodePredicateDataType<mitk::Image>::New());
  //QPushButton * lockButton = new QPushButton();
  //lockButton->setText("");
  //lockButton->setMinimumWidth(40);
  //lockButton->setCheckable(true);
  //lockButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaStop));

  QHBoxLayout *layout = new QHBoxLayout;
  layout->addWidget(cb_inputimage,100);
  //layout->addWidget(lockButton,1);
  m_Controls.m_InputImageLayout->addLayout(layout);
  connect(cb_inputimage, SIGNAL(OnSelectionChanged(const mitk::DataNode*)), this, SLOT(OnInitializeSession(const mitk::DataNode*)));
}

void ClassificationRegionGrow::RemoveItemFromLabelList()
{
  auto numberOfElements = m_Controls.m_InputImageLayout->count();
  auto lastItem = m_Controls.m_InputImageLayout->itemAt(numberOfElements-1);
  QHBoxLayout *layout = dynamic_cast<QHBoxLayout *>(lastItem);
  while (QLayoutItem* item = layout->takeAt(0))
  {
    if (QWidget* widget = item->widget())
        widget->deleteLater();
    delete item;
  }
  m_Controls.m_InputImageLayout->removeItem(lastItem);
  delete lastItem;

}

void ClassificationRegionGrow::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                                     const QList<mitk::DataNode::Pointer>& nodes )
{
  // iterate all selected objects, adjust warning visibility
  foreach( mitk::DataNode::Pointer node, nodes )
  {
    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      return;
    }
  }
}

void ClassificationRegionGrow::OnInitializeSession(const mitk::DataNode *)
{
  OnFeatureSettingsChanged();
}

void ClassificationRegionGrow::ProcessFeatureImages(const mitk::Image::Pointer & raw_image)
{
  // RAW
  if (m_Controls.UseIntensity->isChecked()) {
    m_FeatureImageVector.push_back(raw_image);
  }

  // GAUSS
  if (m_Controls.Gauss1->isChecked())
  {
    mitk::Image::Pointer smoothed;
    mitk::CLUtil::GaussianFilter(raw_image, smoothed, 1);
    m_FeatureImageVector.push_back(smoothed);
  }
  if (m_Controls.Gauss2->isChecked())
  {
    mitk::Image::Pointer smoothed;
    mitk::CLUtil::GaussianFilter(raw_image, smoothed, 2);
    m_FeatureImageVector.push_back(smoothed);
  }
  if (m_Controls.Gauss3->isChecked())
  {
    mitk::Image::Pointer smoothed;
    mitk::CLUtil::GaussianFilter(raw_image, smoothed, 3);
    m_FeatureImageVector.push_back(smoothed);
  }
  if (m_Controls.Gauss4->isChecked())
  {
    mitk::Image::Pointer smoothed;
    mitk::CLUtil::GaussianFilter(raw_image, smoothed, 4);
    m_FeatureImageVector.push_back(smoothed);
  }
  if (m_Controls.Gauss5->isChecked())
  {
    mitk::Image::Pointer smoothed;
    mitk::CLUtil::GaussianFilter(raw_image, smoothed, 5);
    m_FeatureImageVector.push_back(smoothed);
  }

  // Difference of Gaussian
  if (m_Controls.DoG1->isChecked())
  {
    mitk::Image::Pointer smoothed;
    mitk::CLUtil::DifferenceOfGaussianFilter(raw_image, smoothed, 1,0.8);
    m_FeatureImageVector.push_back(smoothed);
  }
  if (m_Controls.DoG2->isChecked())
  {
    mitk::Image::Pointer smoothed;
    mitk::CLUtil::DifferenceOfGaussianFilter(raw_image, smoothed, 2, 1.8);
    m_FeatureImageVector.push_back(smoothed);
  }
  if (m_Controls.DoG3->isChecked())
  {
    mitk::Image::Pointer smoothed;
    mitk::CLUtil::DifferenceOfGaussianFilter(raw_image, smoothed, 3, 2.6);
    m_FeatureImageVector.push_back(smoothed);
  }
  if (m_Controls.DoG4->isChecked())
  {
    mitk::Image::Pointer smoothed;
    mitk::CLUtil::DifferenceOfGaussianFilter(raw_image, smoothed, 4, 3.4);
    m_FeatureImageVector.push_back(smoothed);
  }
  if (m_Controls.DoG5->isChecked())
  {
    mitk::Image::Pointer smoothed;
    mitk::CLUtil::DifferenceOfGaussianFilter(raw_image, smoothed, 5, 4.3);
    m_FeatureImageVector.push_back(smoothed);
  }

  // Laplacian of Gaussian
  if (m_Controls.LoG1->isChecked())
  {
    mitk::Image::Pointer smoothed;
    mitk::CLUtil::LaplacianOfGaussianFilter(raw_image, smoothed, 1);
    m_FeatureImageVector.push_back(smoothed);
  }
  if (m_Controls.LoG2->isChecked())
  {
    mitk::Image::Pointer smoothed;
    mitk::CLUtil::LaplacianOfGaussianFilter(raw_image, smoothed, 2);
    m_FeatureImageVector.push_back(smoothed);
  }
  if (m_Controls.LoG3->isChecked())
  {
    mitk::Image::Pointer smoothed;
    mitk::CLUtil::LaplacianOfGaussianFilter(raw_image, smoothed, 3);
    m_FeatureImageVector.push_back(smoothed);
  }
  if (m_Controls.LoG4->isChecked())
  {
    mitk::Image::Pointer smoothed;
    mitk::CLUtil::LaplacianOfGaussianFilter(raw_image, smoothed, 4);
    m_FeatureImageVector.push_back(smoothed);
  }
  if (m_Controls.LoG5->isChecked())
  {
    mitk::Image::Pointer smoothed;
    mitk::CLUtil::LaplacianOfGaussianFilter(raw_image, smoothed, 5);
    m_FeatureImageVector.push_back(smoothed);
  }

  // Hessian of Gaussian
  if (m_Controls.HoG1->isChecked())
  {
    mitk::CLUtil::HessianOfGaussianFilter(raw_image, m_FeatureImageVector, 1);
  }
  if (m_Controls.HoG2->isChecked())
  {
    mitk::CLUtil::HessianOfGaussianFilter(raw_image, m_FeatureImageVector, 2);
  }
  if (m_Controls.HoG3->isChecked())
  {
    mitk::CLUtil::HessianOfGaussianFilter(raw_image, m_FeatureImageVector, 3);
  }
  if (m_Controls.HoG4->isChecked())
  {
    mitk::CLUtil::HessianOfGaussianFilter(raw_image, m_FeatureImageVector, 4);
  }
  if (m_Controls.HoG5->isChecked())
  {
    mitk::CLUtil::HessianOfGaussianFilter(raw_image, m_FeatureImageVector, 5);
  }

  // LocalHistogram
  if (m_Controls.LH1->isChecked())
  {
    mitk::CLUtil::LocalHistogram(raw_image, m_FeatureImageVector, 5,3);
  }
  if (m_Controls.LH2->isChecked())
  {
    mitk::CLUtil::LocalHistogram(raw_image, m_FeatureImageVector, 5, 5);
  }
  if (m_Controls.LH3->isChecked())
  {
    mitk::CLUtil::LocalHistogram(raw_image, m_FeatureImageVector, 10, 3);
  }
  if (m_Controls.LH4->isChecked())
  {
    mitk::CLUtil::LocalHistogram(raw_image, m_FeatureImageVector, 10, 5);
  }
}

void ClassificationRegionGrow::OnFeatureSettingsChanged()
{
  MITK_INFO << "FeatureSettingsChanged";
  m_CalculateFeatures = true;
}

void ClassificationRegionGrow::DoAutomSegmentation()
{
  MITK_INFO << "Start Automatic Segmentation ...";
  // Load Images from registration process
  QmitkDataStorageComboBox * cb_image = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls.m_InputImageLayout->itemAt(1)->widget());
  QmitkDataStorageComboBox * cb_maskimage = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls.m_MaskImageLayout->itemAt(1)->widget());
  mitk::Image::Pointer raw_image;
  mitk::Image::Pointer mask_image;
  if ((cb_image != NULL) || (cb_maskimage != NULL))
  {
    raw_image = dynamic_cast<mitk::Image *>(cb_image->GetSelectedNode()->GetData());
    mask_image = dynamic_cast<mitk::Image *>(cb_maskimage->GetSelectedNode()->GetData());
  }
  else {
    QMessageBox msgBox;
    msgBox.setText("Please specify the images that shlould be used.");
    msgBox.exec();
    return;
  }

  if (raw_image.IsNull() || mask_image.IsNull())
  {
    QMessageBox msgBox;
    msgBox.setText("Error during processing the specified images.");
    msgBox.exec();
    return;
  }

  std::vector<mitk::Image::Pointer> imageList;
  imageList.push_back(raw_image);
  for (int i = 2; i < m_Controls.m_InputImageLayout->count(); ++i)
  {
    QLayout* layout = dynamic_cast<QLayout*>(m_Controls.m_InputImageLayout->itemAt(i));
    MITK_INFO << layout;
    QmitkDataStorageComboBox * tmp_cb_image = dynamic_cast<QmitkDataStorageComboBox *>(layout->itemAt(0)->widget());
    MITK_INFO << tmp_cb_image;
    if (tmp_cb_image)
    {
      mitk::Image::Pointer tmp_image = dynamic_cast<mitk::Image *>(tmp_cb_image);
        if (tmp_image.IsNotNull())
      {
        MITK_INFO << "Adding Image...";
        imageList.push_back(tmp_image);
      }
    }
  }

  MITK_INFO << "Start Feature Calculation ...";
  if(m_CalculateFeatures)
  {
    m_FeatureImageVector.clear();
    for (auto img : imageList)
    {
      ProcessFeatureImages(img);
    }
    m_CalculateFeatures = false;
    if (m_Controls.checkAddFeaturesToDataManager->isChecked())
    {
      for (std::size_t i = 0; i < m_FeatureImageVector.size(); ++i)
      {
        auto newName = "Feature_" + std::to_string(i);
        AddAsDataNode(m_FeatureImageVector[i].GetPointer(), newName);
      }
    }
  }

  MITK_INFO << "Start Classifier Training ...";
  TrainClassifier(raw_image, mask_image);

  MITK_INFO << "Predict extended Segmentation ...";
  PredictSegmentation(raw_image, mask_image);
}


void ClassificationRegionGrow::TrainClassifier(const mitk::Image::Pointer & raw_image, const mitk::Image::Pointer & mask_image)
{
  typedef itk::Image<double, 3> DoubleImageType;
  typedef itk::Image<short, 3> ShortImageType;

  DoubleImageType::Pointer input;
  ShortImageType::Pointer mask;
  mitk::CastToItkImage(raw_image, input);
  mitk::CastToItkImage(mask_image, mask);

  int numberOfSegmentedVoxel = 0;
  int numberOfFeatures = m_FeatureImageVector.size();

  auto maskIter = itk::ImageRegionConstIteratorWithIndex<ShortImageType>(mask, mask->GetLargestPossibleRegion());
  m_SegmentedLocations.clear();
  m_SegmentedOrganLocations.clear();

  MITK_INFO << "Count Segmentation Size ...";
  while ( ! maskIter.IsAtEnd())
  {
    if (maskIter.Value() > 0)
    {
      m_SegmentedLocations.push_back(maskIter.GetIndex());
      numberOfSegmentedVoxel++;
      if (maskIter.Value() > 1)
      {
        m_SegmentedOrganLocations.push_back(maskIter.GetIndex());
      }
    }
    ++maskIter;
  }
  MITK_INFO << "Sizes: " << numberOfSegmentedVoxel << " " << m_SegmentedOrganLocations.size();

  Eigen::MatrixXi Y_train = mitk::CLUtil::Transform<int>(mask_image, mask_image);
  Eigen::MatrixXd X_train = Eigen::MatrixXd(numberOfSegmentedVoxel, numberOfFeatures);
  unsigned int index = 0;

  MITK_INFO << "Convert Training Data to Eigen Matrix ...";
  for (const auto & image : m_FeatureImageVector)
  {
    X_train.col(index) = mitk::CLUtil::Transform<double>(image, mask_image);
    ++index;
  }

  MITK_INFO << "Classifier Training ...";
  m_Classifier = mitk::VigraRandomForestClassifier::New();
  //this->m_Controls.Maximum
  m_Classifier->SetTreeCount(m_Controls.NumberOfTrees->value());
  m_Classifier->SetSamplesPerTree(m_Controls.SamplesPerTree->value());
  m_Classifier->SetMinimumSplitNodeSize(m_Controls.MinimumSamplesPerNode->value());
  m_Classifier->SetMaximumTreeDepth(m_Controls.MaximumTreeDepth->value());
  m_Classifier->Train(X_train, Y_train);
}

static void addNeighbours(std::stack<itk::Index<3> > &stack, itk::Index<3> idx)
{
  idx[0] -= 1;
  stack.push(idx);
  idx[0] += 2;
  stack.push(idx);
  idx[0] -= 1;
  idx[1] -= 1;
  stack.push(idx);
  idx[1] += 2;
  stack.push(idx);
  idx[1] -= 1;
  idx[2] -= 1;
  stack.push(idx);
  idx[2] += 2;
  stack.push(idx);
}

void ClassificationRegionGrow::PredictSegmentation(const mitk::Image::Pointer & raw_image, const mitk::Image::Pointer & mask_image)
{
  typedef itk::Image<double, 3> DoubleImageType;
  typedef itk::Image<unsigned short, 3> ShortImageType;

  DoubleImageType::Pointer input;
  ShortImageType::Pointer mask;
  mitk::CastToItkImage(raw_image, input);
  mitk::CastToItkImage(mask_image, mask);

  std::vector<DoubleImageType::Pointer > featureImages;
  for (auto fimage : m_FeatureImageVector)
  {
    DoubleImageType::Pointer feature;
    mitk::CastToItkImage(fimage, feature);
    featureImages.push_back(feature);
  }

  ShortImageType::Pointer usedLocation = ShortImageType::New();
  usedLocation->SetRegions(mask->GetLargestPossibleRegion());
  usedLocation->Allocate();
  usedLocation->FillBuffer(0);

  ShortImageType::Pointer resultSegmentation = ShortImageType::New();
  if (m_Controls.UpdateImage->isChecked()) {
    QmitkDataStorageComboBox * cb_maskimage = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls.StartingPointLayout->itemAt(2)->widget());
    mitk::Image::Pointer base_image = dynamic_cast<mitk::Image *>(cb_maskimage->GetSelectedNode()->GetData());
    mitk::CastToItkImage(base_image, resultSegmentation);
  }
  else {

    resultSegmentation->SetRegions(mask->GetLargestPossibleRegion());
    resultSegmentation->Allocate();
    if (m_Controls.SegmentBackground->isChecked())
    {
      resultSegmentation->FillBuffer(1);
    }
    else {
      resultSegmentation->FillBuffer(0);
    }
  }

  // Fill list of Stacks
  std::vector<std::stack<itk::Index<3> > > listOfStacks;
  while (m_SegmentedOrganLocations.size() > 0)
  {
    auto currentLocation = m_SegmentedOrganLocations.back();
    m_SegmentedOrganLocations.pop_back();
    std::size_t cValue = std::abs(mask->GetPixel(currentLocation));
    resultSegmentation->SetPixel(currentLocation, cValue);
    usedLocation->SetPixel(currentLocation, 1000);
    while (listOfStacks.size() < cValue+1)
    {
      listOfStacks.push_back(std::stack<itk::Index<3> >());
    }
    addNeighbours(listOfStacks[cValue],currentLocation);
  }


  int countPredicted = 0;
  bool connectAllLabels = m_Controls.localGrowing->isChecked();
  //m_SegmentedOrganLocations.reserve(10000);

  Eigen::MatrixXd currentX = Eigen::MatrixXd(1, featureImages.size());
  vigra::MultiArrayView<2, double> X(vigra::Shape2(currentX.rows(), currentX.cols()), currentX.data());
  auto outLabel = Eigen::MatrixXi(currentX.rows(), 1);
  vigra::MultiArrayView<2, int> Y(vigra::Shape2(currentX.rows(), 1), outLabel.data());
  for (std::size_t i = 2; i < listOfStacks.size(); ++i)
  {
    while (listOfStacks[i].size() > 0)
    {
      auto currentLocation = listOfStacks[i].top();
      listOfStacks[i].pop();
      if (!mask->GetLargestPossibleRegion().IsInside(currentLocation))
      {
        continue;
      }
      if (usedLocation->GetPixel(currentLocation) > i)
      {
        continue;
      }
      usedLocation->SetPixel(currentLocation, i+1);


      for (std::size_t f = 0; f < featureImages.size(); ++f)
      {
        currentX(0, f) = featureImages[f]->GetPixel(currentLocation);
      }

      m_Classifier->GetRandomForest().predictLabels(X, Y);
      ++countPredicted;
      if ((static_cast<unsigned int>(Y(0, 0)) ==  i ) ||
        ((Y(0, 0) > 1) && (connectAllLabels)))
      {
        resultSegmentation->SetPixel(currentLocation, std::abs(Y(0, 0)));
        addNeighbours(listOfStacks[i], currentLocation);
      }
    }
  }
  MITK_INFO << "Number of Predictions: " << countPredicted;

  MITK_INFO << "Finished Segmentation...";
  mitk::Image::Pointer result;
  mitk::CastToMitkImage(resultSegmentation, result);
  result->SetOrigin(raw_image->GetGeometry()->GetOrigin());
  result->SetSpacing(raw_image->GetGeometry()->GetSpacing());
  mitk::LabelSetImage::Pointer labelResult = mitk::LabelSetImage::New();
  labelResult->InitializeByLabeledImage(result);
  mitk::LabelSetImage::Pointer oldLabelSet = dynamic_cast<mitk::LabelSetImage *>(mask_image.GetPointer());
  labelResult->AddLabelSetToLayer(labelResult->GetActiveLayer(),oldLabelSet->GetLabelSet());
  MITK_INFO << "Passing Back...";
  AddAsDataNode(labelResult.GetPointer(), "ResultSegmentation");
}

mitk::DataNode::Pointer ClassificationRegionGrow::AddAsDataNode(const mitk::BaseData::Pointer & data_, const std::string & name )
{
  mitk::DataNode::Pointer node = nullptr;
  node = this->GetDataStorage()->GetNamedNode(name);

  if(node.IsNull())
  {
    node = mitk::DataNode::New();
    node->SetData(data_);
    node->SetName(name);
    this->GetDataStorage()->Add(node);
  }else{

    if(dynamic_cast<mitk::Image*>(node->GetData()) && dynamic_cast<mitk::Image*>(data_.GetPointer()))
    {
      mitk::Image::Pointer target_image = dynamic_cast<mitk::Image*>(node->GetData());
      mitk::Image::Pointer source_image = dynamic_cast<mitk::Image*>(data_.GetPointer());
      mitk::ImageReadAccessor ra(source_image);
      target_image->SetImportVolume(const_cast<void *>(ra.GetData()));
      this->RequestRenderWindowUpdate();
    }

    if(dynamic_cast<mitk::VigraRandomForestClassifier*>(node->GetData()) && dynamic_cast<mitk::VigraRandomForestClassifier*>(data_.GetPointer()))
    {
      node->SetData(data_);
      node->Modified();
    }

  }

  return node;
}
