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

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "ClassificationSegmentation.h"

// Qt
#include <QMessageBox>

//mitk image
#include <mitkImage.h>
#include <QmitkDataStorageComboBox.h>
#include <ctkSliderWidget.h>
#include <mitkPointSet.h>
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

#include <mitkLookupTable.h>
#include <mitkLookupTableProperty.h>

#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <mitkPointSetDataInteractor.h>
#include <QmitkIOUtil.h>
const std::string ClassificationSegmentation::VIEW_ID = "org.mitk.views.classificationsegmentation";

void ClassificationSegmentation::SetFocus()
{
  //  m_Controls.buttonPerformImageProcessing->setFocus();
}

void ClassificationSegmentation::OnButtonCSFToggle(bool checked)
{
  //  m_PointListWidget->SetPointSet(dynamic_cast<mitk::PointSet *>(m_PointSetList[0]->GetData()));
  if(checked)
    m_PointSetDataInteractor->SetDataNode(m_PointSetList[0]);
  else
    m_PointSetDataInteractor->SetDataNode(nullptr);
}

void ClassificationSegmentation::OnButtonLESToggle(bool checked)
{
  //  m_PointListWidget->SetPointSet(dynamic_cast<mitk::PointSet *>(m_PointSetList[1]->GetData()));
  if(checked)
    m_PointSetDataInteractor->SetDataNode(m_PointSetList[1]);
  else
    m_PointSetDataInteractor->SetDataNode(nullptr);
}

void ClassificationSegmentation::OnButtonBRAToggle(bool checked)
{
  //  m_PointListWidget->SetPointSet(dynamic_cast<mitk::PointSet *>(m_PointSetList[2]->GetData()));
  if(checked)
    m_PointSetDataInteractor->SetDataNode(m_PointSetList[2]);
  else
    m_PointSetDataInteractor->SetDataNode(nullptr);
}

void ClassificationSegmentation::OnButtonNoInteractionToggle(bool checked)
{
  if(checked)
    m_PointSetDataInteractor->SetDataNode(nullptr);
}

void ClassificationSegmentation::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  m_CalculateFeatures = true;
  m_BlockManualSegmentation = false;
  m_BlockPostProcessing = false;

  QmitkDataStorageComboBox * cb_inputimage = new QmitkDataStorageComboBox(this->GetDataStorage(), mitk::TNodePredicateDataType<mitk::Image>::New());
  QmitkDataStorageComboBox * cb_maskimage= new QmitkDataStorageComboBox(this->GetDataStorage(),mitk::TNodePredicateDataType<mitk::Image>::New());
  QmitkDataStorageComboBox * cb_classifer= new QmitkDataStorageComboBox(this->GetDataStorage(),mitk::TNodePredicateDataType<mitk::VigraRandomForestClassifier>::New());

  m_Controls.m_InputImageLayout->addWidget(cb_inputimage);
  m_Controls.m_MaskImageLayout->addWidget(cb_maskimage);
  m_Controls.m_ClassifierLayout->addWidget(cb_classifer);

  m_Controls.m_ParameterLayout->layout()->addWidget(new QLabel("Gauss Sigma"));
  m_GaussSlider = new ctkSliderWidget();
  m_GaussSlider->setMinimum(0);
  m_GaussSlider->setMaximum(10);
  m_GaussSlider->setValue(1);
  m_Controls.m_ParameterLayout->layout()->addWidget(m_GaussSlider);

  m_Controls.m_ParameterLayout->layout()->addWidget(new QLabel("Hessian Sigma"));
  m_HessianSlider = new ctkSliderWidget();
  m_HessianSlider->setMinimum(0);
  m_HessianSlider->setMaximum(10);
  m_HessianSlider->setValue(3);
  m_Controls.m_ParameterLayout->layout()->addWidget(m_HessianSlider);

  m_Controls.m_ParameterLayout->layout()->addWidget(new QLabel("Structure Tensor Inner and Outer Scale"));
  m_STInnerSlider = new ctkSliderWidget();
  m_STInnerSlider->setMinimum(0);
  m_STInnerSlider->setMaximum(10);
  m_STInnerSlider->setValue(1.5);
  m_Controls.m_ParameterLayout->layout()->addWidget(m_STInnerSlider);

  m_STOuterSlider = new ctkSliderWidget();
  m_STOuterSlider->setMinimum(0);
  m_STOuterSlider->setMaximum(10);
  m_STOuterSlider->setValue(3);
  m_Controls.m_ParameterLayout->layout()->addWidget(m_STOuterSlider);

  m_Controls.m_PostProcessingLayout->layout()->addWidget(new QLabel("Probability map smoothing"));
  m_GaussCSFSlider = new ctkSliderWidget;
  m_GaussCSFSlider->setMinimum(0);
  m_GaussCSFSlider->setMaximum(10);
  m_GaussCSFSlider->setValue(1.5);
  m_Controls.m_PostProcessingLayout->layout()->addWidget(m_GaussCSFSlider);

  m_GaussLESSlider = new ctkSliderWidget;
  m_GaussLESSlider->setMinimum(0);
  m_GaussLESSlider->setMaximum(10);
  m_GaussLESSlider->setValue(3);
  m_Controls.m_PostProcessingLayout->layout()->addWidget(m_GaussLESSlider);

  m_GaussBRASlider = new ctkSliderWidget;
  m_GaussBRASlider->setMinimum(0);
  m_GaussBRASlider->setMaximum(10);
  m_GaussBRASlider->setValue(0.5);
  m_Controls.m_PostProcessingLayout->layout()->addWidget(m_GaussBRASlider);

  m_Controls.m_PostProcessingLayout->layout()->addWidget(new QLabel("Probability map weighting"));
  m_WeightCSFSlider = new ctkSliderWidget;
  m_WeightCSFSlider->setMinimum(0.0);
  m_WeightCSFSlider->setMaximum(2.0);
  m_WeightCSFSlider->setValue(1.0);
  m_WeightCSFSlider->setSingleStep(0.1);
  m_Controls.m_PostProcessingLayout->layout()->addWidget(m_WeightCSFSlider);

  m_WeightLESSlider = new ctkSliderWidget;
  m_WeightLESSlider->setMinimum(0.0);
  m_WeightLESSlider->setMaximum(2.0);
  m_WeightLESSlider->setValue(1.0);
  m_WeightLESSlider->setSingleStep(0.1);
  m_Controls.m_PostProcessingLayout->layout()->addWidget(m_WeightLESSlider);

  m_WeightBRASlider = new ctkSliderWidget;
  m_WeightBRASlider->setMinimum(0.0);
  m_WeightBRASlider->setMaximum(2.0);
  m_WeightBRASlider->setValue(1.0);
  m_WeightBRASlider->setSingleStep(0.1);
  m_Controls.m_PostProcessingLayout->layout()->addWidget(m_WeightBRASlider);

  m_PointSetDataInteractor = mitk::PointSetDataInteractor::New();
  m_PointSetDataInteractor->LoadStateMachine("PointSet.xml");
  m_PointSetDataInteractor->SetEventConfig("PointSetConfig.xml");

  connect( cb_inputimage, SIGNAL(OnSelectionChanged(const mitk::DataNode*)), this, SLOT(OnInitializeSession(const mitk::DataNode*)));
  connect( cb_maskimage, SIGNAL(OnSelectionChanged(const mitk::DataNode*)), this, SLOT(OnInitializeSession(const mitk::DataNode*)));

  connect( m_Controls.m_SavePointsButton, SIGNAL(clicked(bool)), this, SLOT(DoSavePointsAsMask()));

  connect( m_Controls.m_ButtonCSFToggle, SIGNAL(toggled(bool)), this, SLOT(OnButtonCSFToggle(bool)));
  connect( m_Controls.m_ButtonLESToggle, SIGNAL(toggled(bool)), this, SLOT(OnButtonLESToggle(bool)));
  connect( m_Controls.m_ButtonBRAToggle, SIGNAL(toggled(bool)), this, SLOT(OnButtonBRAToggle(bool)));
  connect( m_Controls.m_ButtonNoInteraction, SIGNAL(toggled(bool)), this, SLOT(OnButtonNoInteractionToggle(bool)));
  connect( &m_ManualSegmentationFutureWatcher, SIGNAL(finished()), this, SLOT(ManualSegmentationFinished()));
  connect( &m_PostProcessingFutureWatcher, SIGNAL(finished()), this, SLOT(PostProcessingFinished()));

  //connect( m_Controls.m_AddPointSets, SIGNAL(clicked()),this, SLOT(OnAddPointSets()) );
  connect( m_GaussSlider, SIGNAL(valueChanged(double)), this, SLOT(OnFeatureSettingsChanged()));
  connect( m_HessianSlider, SIGNAL(valueChanged(double)), this, SLOT(OnFeatureSettingsChanged()));
  connect( m_STInnerSlider, SIGNAL(valueChanged(double)), this, SLOT(OnFeatureSettingsChanged()));
  connect( m_STOuterSlider, SIGNAL(valueChanged(double)), this, SLOT(OnFeatureSettingsChanged()));

  connect( m_GaussCSFSlider, SIGNAL(valueChanged(double)), this, SLOT(OnPostProcessingSettingsChanged()));
  connect( m_GaussLESSlider, SIGNAL(valueChanged(double)), this, SLOT(OnPostProcessingSettingsChanged()));
  connect( m_GaussBRASlider, SIGNAL(valueChanged(double)), this, SLOT(OnPostProcessingSettingsChanged()));
  connect( m_WeightCSFSlider, SIGNAL(valueChanged(double)), this, SLOT(OnPostProcessingSettingsChanged()));
  connect( m_WeightLESSlider, SIGNAL(valueChanged(double)), this, SLOT(OnPostProcessingSettingsChanged()));
  connect( m_WeightBRASlider, SIGNAL(valueChanged(double)), this, SLOT(OnPostProcessingSettingsChanged()));

  connect(m_Controls.m_DoAutomaticSecmentation, SIGNAL(clicked(bool)), this, SLOT(DoAutomSegmentation()));
  connect(m_Controls.m_AddForestToDataManager, SIGNAL(clicked(bool)), this, SLOT(OnAddForestToDataManager()));

  mitk::DataNode::Pointer pointSetNode = mitk::DataNode::New();
  pointSetNode->SetName("CSF_Points.");
  pointSetNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  pointSetNode->SetProperty("layer", mitk::IntProperty::New(1024));
  pointSetNode->SetColor(0,1.0,0);
  pointSetNode->SetDataInteractor(m_PointSetDataInteractor.GetPointer());
  m_PointSetList.push_back(pointSetNode);
  GetDataStorage()->Add(pointSetNode);

  pointSetNode = mitk::DataNode::New();
  pointSetNode->SetName("LES_Points.");
  pointSetNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  pointSetNode->SetProperty("layer", mitk::IntProperty::New(1024));
  pointSetNode->SetColor(1.0,0,0);
  pointSetNode->SetDataInteractor(m_PointSetDataInteractor.GetPointer());
  m_PointSetList.push_back(pointSetNode);
  GetDataStorage()->Add(pointSetNode);

  pointSetNode = mitk::DataNode::New();
  pointSetNode->SetName("BRA_Points.");
  pointSetNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  pointSetNode->SetProperty("layer", mitk::IntProperty::New(1024));
  pointSetNode->SetColor(0,0,1.0);
  pointSetNode->SetDataInteractor(m_PointSetDataInteractor.GetPointer());
  m_PointSetList.push_back(pointSetNode);
  GetDataStorage()->Add(pointSetNode);


  m_Controls.m_PostProcessingLayout->hide();
  m_Controls.m_ParameterLayout->hide();
}

void ClassificationSegmentation::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                                     const QList<mitk::DataNode::Pointer>& nodes )
{
  // iterate all selected objects, adjust warning visibility
  foreach( mitk::DataNode::Pointer node, nodes )
  {
    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      //      m_Controls.labelWarning->setVisible( false );
      //      m_Controls.buttonPerformImageProcessing->setEnabled( true );
      return;
    }
  }

  //  m_Controls.labelWarning->setVisible( true );
  //  m_Controls.buttonPerformImageProcessing->setEnabled( false );
}

void ClassificationSegmentation::OnAddForestToDataManager()
{
  AddAsDataNode(m_TempClassifier.GetPointer(),"ManualSegmentation_Classifier");
}

void ClassificationSegmentation::OnInitializeSession(const mitk::DataNode *)
{
  MITK_WARN << "Data selection changed! New session initialized.";

  m_PointSetDataInteractor->SetDataNode(nullptr);

  /// CSF POINTSET
  mitk::PointSet::Pointer pntset = mitk::PointSet::New();
  m_PointSetList[0]->SetData(pntset);

  itk::SimpleMemberCommand<ClassificationSegmentation>::Pointer command = itk::SimpleMemberCommand<ClassificationSegmentation>::New();
  command->SetCallbackFunction(this, &ClassificationSegmentation::ManualSegmentationTrigger);
  pntset->AddObserver( mitk::PointSetAddEvent(), command );
  pntset->AddObserver( mitk::PointSetRemoveEvent(), command );
  pntset->AddObserver( mitk::PointSetMoveEvent(), command );

  /// LES POINTSET
  pntset = mitk::PointSet::New();
  m_PointSetList[1]->SetData(pntset);

  command = itk::SimpleMemberCommand<ClassificationSegmentation>::New();
  command->SetCallbackFunction(this, &ClassificationSegmentation::ManualSegmentationTrigger);
  pntset->AddObserver( mitk::PointSetAddEvent(), command );
  pntset->AddObserver( mitk::PointSetRemoveEvent(), command );
  pntset->AddObserver( mitk::PointSetMoveEvent(), command );

  /// BRA POINTSET
  pntset = mitk::PointSet::New();
  m_PointSetList[2]->SetData(pntset);

  command = itk::SimpleMemberCommand<ClassificationSegmentation>::New();
  command->SetCallbackFunction(this, &ClassificationSegmentation::ManualSegmentationTrigger);
  pntset->AddObserver( mitk::PointSetAddEvent(), command );
  pntset->AddObserver( mitk::PointSetRemoveEvent(), command );
  pntset->AddObserver( mitk::PointSetMoveEvent(), command );

}

void ClassificationSegmentation::DoSavePointsAsMask()
{
  mitk::Image::Pointer sampled_image = nullptr;
  QmitkDataStorageComboBox * cb_image = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls.m_InputImageLayout->itemAt(1)->widget());
  mitk::Image::Pointer raw_image = dynamic_cast<mitk::Image *>(cb_image->GetSelectedNode()->GetData());

  int label = 1;
  for( auto datanode : m_PointSetList)
  {
    mitk::PointSet::Pointer point_set = dynamic_cast<mitk::PointSet *>(datanode->GetData());
    mitk::Image::Pointer temp_img;
    SampleClassMaskByPointSet(raw_image, point_set,temp_img);
    mitk::CLUtil::InsertLabel(temp_img,sampled_image,label++);
    QmitkIOUtil::Save(datanode->GetData(),QString("PointSet.mps"));
  }

  QmitkIOUtil::Save(sampled_image,QString("PointSetMask.nrrd"));
}
void ClassificationSegmentation::ProcessFeatureImages(const mitk::Image::Pointer & raw_image, const mitk::Image::Pointer & brain_mask)
{
  typedef itk::Image<double,3> DoubleImageType;
  typedef itk::Image<short,3> ShortImageType;
  typedef itk::ConstNeighborhoodIterator<DoubleImageType> NeighborhoodType; // Neighborhood iterator to access image
  typedef itk::Functor::NeighborhoodFirstOrderStatistics<NeighborhoodType, double> FunctorType;
  typedef itk::NeighborhoodFunctorImageFilter<DoubleImageType, DoubleImageType, FunctorType> FOSFilerType;

  m_FeatureImageVector.clear();

  // RAW
  m_FeatureImageVector.push_back(raw_image);

  // GAUSS
  mitk::Image::Pointer smoothed;
  mitk::CLUtil::GaussianFilter(raw_image,smoothed,m_GaussSlider->value());
  m_FeatureImageVector.push_back(smoothed);

  // Calculate Probability maps (parameters used from literatur)
  // CSF
  mitk::Image::Pointer csf_prob = mitk::Image::New();
  mitk::CLUtil::ProbabilityMap(smoothed,13.9, 8.3,csf_prob);
  m_FeatureImageVector.push_back(csf_prob);

  // Lesion

  mitk::Image::Pointer les_prob = mitk::Image::New();
  mitk::CLUtil::ProbabilityMap(smoothed,59, 11.6,les_prob);
  m_FeatureImageVector.push_back(les_prob);

  // Barin (GM/WM)

  mitk::Image::Pointer brain_prob = mitk::Image::New();
  mitk::CLUtil::ProbabilityMap(smoothed,32, 5.6,brain_prob);
  m_FeatureImageVector.push_back(brain_prob);

  std::vector<unsigned int> FOS_sizes;
  FOS_sizes.push_back(1);

  DoubleImageType::Pointer input;
  ShortImageType::Pointer mask;
  mitk::CastToItkImage(smoothed, input);
  mitk::CastToItkImage(brain_mask, mask);

  for(unsigned int i = 0 ; i < FOS_sizes.size(); i++)
  {
    FOSFilerType::Pointer filter = FOSFilerType::New();
    filter->SetNeighborhoodSize(FOS_sizes[i]);
    filter->SetInput(input);
    filter->SetMask(mask);

    filter->Update();
    FOSFilerType::DataObjectPointerArray array = filter->GetOutputs();

    for( unsigned int i = 0; i < FunctorType::OutputCount; i++)
    {
      mitk::Image::Pointer featureimage;
      mitk::CastToMitkImage(dynamic_cast<DoubleImageType *>(array[i].GetPointer()),featureimage);
      m_FeatureImageVector.push_back(featureimage);
      //      AddImageAsDataNode(featureimage,FunctorType::GetFeatureName(i))->SetVisibility(show_nodes);
    }
  }

  {
    itk::HessianMatrixEigenvalueImageFilter< DoubleImageType >::Pointer filter = itk::HessianMatrixEigenvalueImageFilter< DoubleImageType >::New();
    filter->SetInput(input);
    filter->SetImageMask(mask);
    filter->SetSigma(m_HessianSlider->value());
    filter->Update();

    mitk::Image::Pointer o1,o2,o3;
    mitk::CastToMitkImage(filter->GetOutput(0),o1);
    mitk::CastToMitkImage(filter->GetOutput(1),o2);
    mitk::CastToMitkImage(filter->GetOutput(2),o3);

    m_FeatureImageVector.push_back(o1);
    m_FeatureImageVector.push_back(o2);
    m_FeatureImageVector.push_back(o3);

    //    AddImageAsDataNode(o1,"HE_1")->SetVisibility(show_nodes);
    //    AddImageAsDataNode(o2,"HE_2")->SetVisibility(show_nodes);
    //    AddImageAsDataNode(o3,"HE_3")->SetVisibility(show_nodes);
  }

  {
    itk::StructureTensorEigenvalueImageFilter< DoubleImageType >::Pointer filter = itk::StructureTensorEigenvalueImageFilter< DoubleImageType >::New();
    filter->SetInput(input);
    filter->SetImageMask(mask);
    filter->SetInnerScale(m_STInnerSlider->value());
    filter->SetOuterScale(m_STOuterSlider->value());
    filter->Update();

    mitk::Image::Pointer o1,o2,o3;
    mitk::CastToMitkImage(filter->GetOutput(0),o1);
    mitk::CastToMitkImage(filter->GetOutput(1),o2);
    mitk::CastToMitkImage(filter->GetOutput(2),o3);

    m_FeatureImageVector.push_back(o1);
    m_FeatureImageVector.push_back(o2);
    m_FeatureImageVector.push_back(o3);

    //    AddImageAsDataNode(o1,"ST_1")->SetVisibility(show_nodes);
    //    AddImageAsDataNode(o2,"ST_2")->SetVisibility(show_nodes);
    //    AddImageAsDataNode(o3,"ST_3")->SetVisibility(show_nodes);
  }

  {
    itk::LineHistogramBasedMassImageFilter< DoubleImageType >::Pointer filter = itk::LineHistogramBasedMassImageFilter< DoubleImageType >::New();
    filter->SetInput(input);
    filter->SetImageMask(mask);
    filter->Update();

    mitk::Image::Pointer o1;
    mitk::CastToMitkImage(filter->GetOutput(0),o1);

    m_FeatureImageVector.push_back(o1);
  }
}

void ClassificationSegmentation::OnFeatureSettingsChanged()
{
  MITK_INFO << "FeatureSettingsChanged";
  m_CalculateFeatures = true;
  ManualSegmentationTrigger();
}

void ClassificationSegmentation::OnPostProcessingSettingsChanged()
{
  MITK_INFO << "PostProcessingSettigsChanged";
  PostProcessingTrigger();
}

void ClassificationSegmentation::DoAutomSegmentation()
{
  QmitkDataStorageComboBox * cb_image = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls.m_InputImageLayout->itemAt(1)->widget());
  mitk::Image::Pointer raw_image = dynamic_cast<mitk::Image *>(cb_image->GetSelectedNode()->GetData());
  QmitkDataStorageComboBox * cb_maskimage = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls.m_MaskImageLayout->itemAt(1)->widget());
  QmitkDataStorageComboBox * cb_rf = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls.m_ClassifierLayout->itemAt(1)->widget());

  mitk::Image::Pointer mask_image = dynamic_cast<mitk::Image *>(cb_maskimage->GetSelectedNode()->GetData());
  mitk::VigraRandomForestClassifier::Pointer classifier = dynamic_cast<mitk::VigraRandomForestClassifier *>(cb_rf->GetSelectedNode()->GetData());

  if(m_CalculateFeatures)
  {
    ProcessFeatureImages(raw_image,mask_image);
    m_CalculateFeatures = false;
  }

  Eigen::MatrixXd X_test;

  unsigned int count_test = 0;
  mitk::CLUtil::CountVoxel(mask_image, count_test);
  X_test = Eigen::MatrixXd(count_test, m_FeatureImageVector.size());

  unsigned int index = 0;
  for( const auto & image : m_FeatureImageVector)
  {
    X_test.col(index) = mitk::CLUtil::Transform<double>(image,mask_image);
    ++index;
  }

  Eigen::MatrixXi Y_test = classifier->Predict(X_test);
  mitk::Image::Pointer result_mask = mitk::CLUtil::Transform<int>(Y_test,mask_image);

  auto node = AddAsDataNode(result_mask.GetPointer(),"Autom-ResultMask");

  auto lut = mitk::LookupTable::New();
  lut->SetType(mitk::LookupTable::PET_20);
  auto * lut_prop = dynamic_cast<mitk::LookupTableProperty *>(node->GetProperty("LookupTable"));
  lut_prop->SetLookupTable(lut);

  mitk::LevelWindow lw(1,3);
  node->SetLevelWindow(lw);
  node->SetOpacity(0.3);

  std::map<unsigned int, unsigned int> perlabelvoxelcount;
  mitk::CLUtil::CountVoxel(result_mask, perlabelvoxelcount);

  double voxel_volume = result_mask->GetGeometry()->GetSpacing().GetVnlVector().inf_norm();
  QString newtext;
  newtext += "Name\tVolume\tUnit\n";
  for(const auto & pair: perlabelvoxelcount)
  {
    newtext += "Label" + QString::number(pair.first) + "\t" + QString::number(pair.second*voxel_volume* 0.001) + "\tml\n";
  }

  m_Controls.m_ResultTextEdit->setText(newtext);
}

std::vector<mitk::Image::Pointer> ClassificationSegmentation::ManualSegmentationCallback()
{
  QmitkDataStorageComboBox * cb_image = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls.m_InputImageLayout->itemAt(1)->widget());
  QmitkDataStorageComboBox * cb_maskimage = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls.m_MaskImageLayout->itemAt(1)->widget());

  mitk::Image::Pointer raw_image = dynamic_cast<mitk::Image *>(cb_image->GetSelectedNode()->GetData());
  mitk::Image::Pointer mask_image = dynamic_cast<mitk::Image *>(cb_maskimage->GetSelectedNode()->GetData());

  if(raw_image.IsNull() || mask_image.IsNull())
  {
    MITK_WARN << "Please provide input iamge and mask image";
    //return nullptr;
  }

  if(m_CalculateFeatures)
  {
    ProcessFeatureImages(raw_image,mask_image);
    m_CalculateFeatures = false;
  }

  mitk::Image::Pointer sampled_image = nullptr;
  int label = 1;
  for( auto datanode : m_PointSetList)
  {
    mitk::PointSet::Pointer point_set = dynamic_cast<mitk::PointSet *>(datanode->GetData());
    mitk::Image::Pointer temp_img;
    SampleClassMaskByPointSet(raw_image, point_set,temp_img);
    mitk::CLUtil::InsertLabel(temp_img,sampled_image,label++);
  }

  m_TempClassifier = mitk::VigraRandomForestClassifier::New();
  m_TempClassifier->SetTreeCount(50);
  m_TempClassifier->SetSamplesPerTree(0.66);

  Eigen::MatrixXd X_train;
  Eigen::MatrixXd X_test;

  unsigned int count_train = 0;
  unsigned int count_test = 0;
  mitk::CLUtil::CountVoxel(sampled_image, count_train);
  mitk::CLUtil::CountVoxel(mask_image, count_test);
  X_train = Eigen::MatrixXd(count_train, m_FeatureImageVector.size() );
  X_test = Eigen::MatrixXd(count_test, m_FeatureImageVector.size());

  unsigned int index = 0;
  for( const auto & image : m_FeatureImageVector)
  {
    X_train.col(index) = mitk::CLUtil::Transform<double>(image,sampled_image);
    X_test.col(index) = mitk::CLUtil::Transform<double>(image,mask_image);
    ++index;
  }

  Eigen::MatrixXi Y = mitk::CLUtil::Transform<int>(sampled_image,sampled_image);
  m_TempClassifier->Train(X_train,Y);

  Eigen::MatrixXi Y_test = m_TempClassifier->Predict(X_test);
  Eigen::MatrixXd Yp_test = m_TempClassifier->GetPointWiseProbabilities();
  //  mitk::Image::Pointer result_mask = mitk::CLUtil::Transform<int>(Y_test,mask_image);

  std::vector<mitk::Image::Pointer> resultvector;
  resultvector.push_back( mitk::CLUtil::Transform<int>(Y_test,mask_image) );
  resultvector.push_back( mitk::CLUtil::Transform<double>(Yp_test.col(0),mask_image) );
  resultvector.push_back( mitk::CLUtil::Transform<double>(Yp_test.col(1),mask_image) );
  resultvector.push_back( mitk::CLUtil::Transform<double>(Yp_test.col(2),mask_image) );

  return resultvector;
}

void ClassificationSegmentation::ManualSegmentationFinished()
{
  // Receive Future result
  m_ResultImageVector = m_ManualSegmentationFutureWatcher.result();

  // Add result to Datastorage
  mitk::DataNode::Pointer node = AddAsDataNode(m_ResultImageVector[0].GetPointer(),"Manual-ResultMask");
  mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
  lut->SetType(mitk::LookupTable::PET_20);
  mitk::LookupTableProperty * lut_prop = dynamic_cast<mitk::LookupTableProperty *>(node->GetProperty("LookupTable"));
  lut_prop->SetLookupTable(lut);

  mitk::LevelWindow lw(1,3);
  node->SetLevelWindow(lw);
  node->SetOpacity(0.3);

  m_BlockManualSegmentation = false;
  this->PostProcessingTrigger();

  // Update Volume data
  std::map<unsigned int, unsigned int> perlabelvoxelcount;
  mitk::CLUtil::CountVoxel(m_ResultImageVector[0], perlabelvoxelcount);
  double voxel_volume = m_ResultImageVector[0]->GetGeometry()->GetSpacing().GetVnlVector().inf_norm();
  QString newtext;
  newtext += "Name\tVolume\tUnit\n";
  for(const auto & pair: perlabelvoxelcount)
    newtext += "Label" + QString::number(pair.first) + "\t" + QString::number(pair.second*voxel_volume* 0.001) + "\tml\n";
  m_Controls.m_ResultTextEdit->setText(newtext);


}

void ClassificationSegmentation::ManualSegmentationTrigger()
{
  unsigned int samplecounter = 0;
  for( auto datanode : m_PointSetList)
  {
    mitk::PointSet::Pointer point_set = dynamic_cast<mitk::PointSet *>(datanode->GetData());
    samplecounter += point_set->GetSize();
  }
  if(samplecounter < 10) return;

  if(!m_BlockManualSegmentation){
    // Start GUI Thread
    m_ManualSegmentationFutureWatcher.setFuture(
          QtConcurrent::run(this, &ClassificationSegmentation::ManualSegmentationCallback)); // on finish call OnManualSegmentationFinished();
    m_BlockManualSegmentation = true;
  }
}

void ClassificationSegmentation::SampleClassMaskByPointSet(const mitk::Image::Pointer & ref_img, mitk::PointSet::Pointer & pointset, mitk::Image::Pointer & outimage)
{
  outimage = ref_img->Clone();
  itk::Image<unsigned short, 3>::Pointer itk_out;
  mitk::CastToItkImage(outimage,itk_out);

  itk_out->FillBuffer(0);

  typedef itk::ImageRegionIteratorWithIndex<itk::Image<unsigned short, 3> > IteratorType;
  IteratorType oit(itk_out, itk_out->GetLargestPossibleRegion());

  for(int i = 0 ; i < pointset->GetSize(); ++i)
  {
    IteratorType::IndexType index;
    ref_img->GetGeometry()->WorldToIndex(pointset->GetPoint(i), index);
    oit.SetIndex(index);
    oit.Set(1);
  }

  mitk::CastToMitkImage(itk_out,outimage);
}

// old version
//void ClassificationSegmentation::DoImageProcessing()
//{
//  QmitkDataStorageComboBox * cb_image = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls.m_InputImageLayout->itemAt(1)->widget());
//  QmitkDataStorageComboBox * cb_maskimage = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls.m_MaskImageLayout->itemAt(1)->widget());
//  QmitkDataStorageComboBox * cb_classifier = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls.m_RandomForestLayout->itemAt(1)->widget());

//  if (cb_image == nullptr || cb_classifier == nullptr || cb_maskimage == nullptr)
//  {
//    QMessageBox::information( nullptr, "Template", "Please load and select an image before starting image processing.");
//    return;
//  }

//  mitk::VigraRandomForestClassifier::Pointer rf = dynamic_cast<mitk::VigraRandomForestClassifier *>(cb_classifier->GetSelectedNode()->GetData());
//  mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(cb_image->GetSelectedNode()->GetData());
//  mitk::Image::Pointer mask = dynamic_cast<mitk::Image *>(cb_maskimage->GetSelectedNode()->GetData());

//  unsigned int n_samples = 0;
//  mitk::CLUtil::CountVoxel(mask, n_samples);
//  unsigned int n_features = 13; // InputImage, PROBA_CSF, PROBA_LES, PROBA_BRA, FOS_MEAN,FOS_VARIANCE,FOS_SKEWNESS, FOS_KURTOSIS, FOS_MIN, FOS_MAX, HE_1, HE_2, HE_3

//  Eigen::MatrixXd feature_matrix = Eigen::MatrixXd(n_samples, n_features);

//  MITK_INFO << "Remove voxels outside the mask";
//  //  mitk::CLUtil::LogicalAndImages(image,brain_mask,image);
//  feature_matrix.block(0,0,n_samples,1) = mitk::CLUtil::Transform<double>(image,mask);
//  AddImageAsDataNode(image, "UI_InputImage");

//  mitk::Image::Pointer csf_prob = mitk::Image::New();
//  mitk::CLUtil::ProbabilityMap(image,13.9, 8.3,csf_prob);
//  feature_matrix.block(0,1,n_samples,1) = mitk::CLUtil::Transform<double>(csf_prob,mask);
//  AddImageAsDataNode(csf_prob, "UI_CSFProb");

//  mitk::Image::Pointer les_prob = mitk::Image::New();
//  mitk::CLUtil::ProbabilityMap(image,59, 11.6,les_prob);
//  feature_matrix.block(0,2,n_samples,1) = mitk::CLUtil::Transform<double>(les_prob,mask);
//  AddImageAsDataNode(les_prob, "UI_LESProb");

//  mitk::Image::Pointer brain_prob = mitk::Image::New();
//  mitk::CLUtil::ProbabilityMap(image,32, 5.6,brain_prob);
//  feature_matrix.block(0,3,n_samples,1) = mitk::CLUtil::Transform<double>(brain_prob,mask);
//  AddImageAsDataNode(brain_prob, "UI_BRAProb");

//  int n = 0;

//  std::vector<mitk::Image::Pointer> res;
//  DoFirstOrderFeatureCalculation(image, mask, 1, res);

//  for( const auto & img: res)
//  {
//    feature_matrix.block(0,3+(++n),n_samples,1) = mitk::CLUtil::Transform<double>(img,mask);
//    std::string name;
//    img->GetPropertyList()->GetStringProperty("name", name);
//    AddImageAsDataNode(img.GetPointer(), name );
//  }

//  res.clear();
//  DoHessianEigenvaluesFeatureCalculation(image, mask, 3.0, res);
//  for( const auto & img: res)
//  {
//    feature_matrix.block(0,3+(++n),n_samples,1) = mitk::CLUtil::Transform<double>(img,mask);
//    std::string name;
//    img->GetPropertyList()->GetStringProperty("name", name);
//    AddImageAsDataNode(img.GetPointer(), name );
//  }

//  MITK_INFO << "Start Prediction";
//  rf->Predict(feature_matrix);
//  MITK_INFO << "End Prediction";

//  mitk::Image::Pointer result_image = mitk::CLUtil::Transform<int>(rf->GetLabels(),mask);
//  mitk::Image::Pointer proba1_image = mitk::CLUtil::Transform<double>(rf->GetPointWiseProbabilities().col(0),mask);
//  mitk::Image::Pointer proba2_image = mitk::CLUtil::Transform<double>(rf->GetPointWiseProbabilities().col(1),mask);
//  mitk::Image::Pointer proba3_image = mitk::CLUtil::Transform<double>(rf->GetPointWiseProbabilities().col(2),mask);

//  AddImageAsDataNode(result_image, "ClassificationResult");
//  AddImageAsDataNode(proba1_image, "CSF_Proba_CL");
//  AddImageAsDataNode(proba2_image, "LES_Proba_CL");
//  AddImageAsDataNode(proba3_image, "BRA_Proba_CL");

//  PostProcessing(rf,mask);

//}

mitk::DataNode::Pointer ClassificationSegmentation::AddAsDataNode(const mitk::BaseData::Pointer & data_, const std::string & name )
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

void ClassificationSegmentation::PostProcessingTrigger()
{
  if(m_ResultImageVector.empty() || m_ResultImageVector.size() < 4)
  {
    MITK_ERROR("PostProcessingCallback") << "Result image vector not initialized!";
    return;
  }

  if(!m_BlockPostProcessing){
    m_PostProcessingFutureWatcher.setFuture(
          QtConcurrent::run(this, &ClassificationSegmentation::PostProcessingCallback)); // on finish call OnManualSegmentationFinished();
    m_BlockPostProcessing = true;
  }
}

void ClassificationSegmentation::PostProcessingFinished()
{
  // Receive Future result
  m_PostProcessingImageVector = m_PostProcessingFutureWatcher.result();

  // Add result to Datastorage
  mitk::DataNode::Pointer node = AddAsDataNode(m_PostProcessingImageVector[0].GetPointer(),"Manual-ResultMask-PostProcessing");
  mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
  lut->SetType(mitk::LookupTable::PET_20);
  mitk::LookupTableProperty * lut_prop = dynamic_cast<mitk::LookupTableProperty *>(node->GetProperty("LookupTable"));
  lut_prop->SetLookupTable(lut);

  mitk::LevelWindow lw(1,3);
  node->SetLevelWindow(lw);
  node->SetOpacity(0.3);

  m_BlockPostProcessing = false;
}

std::vector<mitk::Image::Pointer> ClassificationSegmentation::PostProcessingCallback()
{
  std::vector<mitk::Image::Pointer> resultvector;

  mitk::Image::Pointer CSF_PMap = m_ResultImageVector[1]->Clone();
  mitk::Image::Pointer LES_PMap = m_ResultImageVector[2]->Clone();
  mitk::Image::Pointer BRA_PMap = m_ResultImageVector[3]->Clone();
  mitk::Image::Pointer mask = m_ResultImageVector[0]->Clone();

  MITK_INFO("PostProcessingCallback") << "ProbabilityMap merg strat ...";

  {
    mitk::Image::Pointer resultmask = mask->Clone();
    mitk::CLUtil::GaussianFilter(CSF_PMap, CSF_PMap, m_GaussCSFSlider->value());
    mitk::CLUtil::GaussianFilter(LES_PMap, LES_PMap, m_GaussLESSlider->value());
    mitk::CLUtil::GaussianFilter(BRA_PMap, BRA_PMap, m_GaussBRASlider->value());

    itk::Image<double, 3>::Pointer itk_csf, itk_les, itk_bra;
    itk::Image<unsigned char, 3>::Pointer itk_result;

    mitk::CastToItkImage(CSF_PMap, itk_csf);
    mitk::CastToItkImage(LES_PMap, itk_les);
    mitk::CastToItkImage(BRA_PMap, itk_bra);
    mitk::CastToItkImage(resultmask, itk_result);

    itk::ImageRegionIterator<itk::Image<double, 3> > it_csf(itk_csf,itk_csf->GetLargestPossibleRegion());
    itk::ImageRegionIterator<itk::Image<double, 3> > it_les(itk_les,itk_les->GetLargestPossibleRegion());
    itk::ImageRegionIterator<itk::Image<double, 3> > it_bra(itk_bra,itk_bra->GetLargestPossibleRegion());
    itk::ImageRegionIterator<itk::Image<unsigned char, 3> > it_res(itk_result,itk_result->GetLargestPossibleRegion());

    while (!it_csf.IsAtEnd()) {
      double csf = it_csf.Value() * m_WeightCSFSlider->value();
      double les = it_les.Value() * m_WeightLESSlider->value();
      double bra = it_bra.Value() * m_WeightBRASlider->value();

      if(csf > les && csf > bra) it_res.Set(1);
      if(les > csf && les > bra) it_res.Set(2);
      if(bra > les && bra > csf) it_res.Set(3);

      ++it_csf;
      ++it_les;
      ++it_bra;
      ++it_res;
    }

    mitk::CastToMitkImage(itk_result, resultmask);

    {
      std::map<unsigned int, unsigned int> mergeinstruction ;
      mergeinstruction[1] = 1;
      mergeinstruction[2] = 1;
      mergeinstruction[3] = 1;
      mergeinstruction[4] = 1;
      mergeinstruction[5] = 1;
      mergeinstruction[6] = 1;

      // {{1,1},{2,1},{3,1},{4,1},{5,1},{6,1}};
      mitk::Image::Pointer brain_mask = mask->Clone();
      mitk::CLUtil::MergeLabels(brain_mask, mergeinstruction);
      mitk::CLUtil::ClosingBinary(brain_mask,brain_mask,2,mitk::CLUtil::All);
      mitk::CLUtil::LogicalAndImages(resultmask, brain_mask, resultmask);
    }
    MITK_INFO("PostProcessingCallback") << "ProbabilityMap merg end!";

    resultvector.push_back(resultmask);
  }

  return resultvector;

  //  {
  //    MITK_INFO << "Morphological processing strat ...";
  //    mitk::Image::Pointer resultmask =  mask->Clone();

  //    mitk::Image::Pointer csf_mask;
  //    mitk::CLUtil::GrabLabel(resultmask, csf_mask, 1);
  //    mitk::CLUtil::ClosingBinary(csf_mask,csf_mask,1,mitk::CLUtil::Axial);

  //    mitk::CLUtil::ErodeBinary(csf_mask, csf_mask, 2, mitk::CLUtil::Axial);
  //    mitk::CLUtil::DilateBinary(csf_mask, csf_mask, 1, mitk::CLUtil::Axial);

  //    std::map<unsigned int, unsigned int> merge_instruction = {{0,0},{1,3},{2,2},{3,3}};
  //    mitk::CLUtil::MergeLabels(resultmask, merge_instruction);

  //    mitk::CLUtil::InsertLabel(resultmask, csf_mask, 1/*as csf mask*/); // add morpological manipulated csf_mask

  //    // ------------

  //    mitk::Image::Pointer les_mask;
  //    mitk::CLUtil::GrabLabel(resultmask, les_mask, 2);
  //    mitk::CLUtil::ClosingBinary(les_mask,les_mask,1,mitk::CLUtil::Axial);

  //    mitk::Image::Pointer les_cc_mask; unsigned int num = 0;
  //    mitk::CLUtil::ConnectedComponentsImage(les_mask, mask, les_cc_mask, num);
  //    std::map<unsigned int, unsigned int> map;
  //    mitk::CLUtil::CountVoxel(les_cc_mask,map);
  //    unsigned int counter = 0;

  //    while(map.size() > 2)
  //    {
  //      mitk::CLUtil::ErodeBinary(les_mask, les_mask, 1, mitk::CLUtil::Axial);
  //      mitk::CLUtil::LogicalAndImages(les_cc_mask,les_mask,les_cc_mask);
  //      map.clear();
  //      mitk::CLUtil::CountVoxel(les_cc_mask,map);
  //      MITK_INFO("PostProcessing") << map.size();
  //      counter++;
  //    }

  //    while(counter != 0)
  //    {
  //      mitk::CLUtil::DilateBinary(les_mask, les_mask, 1, mitk::CLUtil::Axial);
  //      counter--;
  //    }

  //    merge_instruction = {{0,0},{1,1},{2,3},{3,3}};
  //    mitk::CLUtil::MergeLabels(resultmask, merge_instruction);
  //    mitk::CLUtil::InsertLabel(resultmask, les_mask, 2/*as les mask*/);

  //    MITK_INFO << "Morphological processing end";

  //    // ------------
  //    mitk::CLUtil::LogicalAndImages(resultmask,mask,resultmask);

  //    AddImageAsDataNode(resultmask,"SmoothedMaskMorphed");

  //  }
}

//  mitk::Image::Pointer r= mitk::Image::New(),g= mitk::Image::New(),b = mitk::Image::New();

//     unsigned int* m_ImageDimensions;
//     m_ImageDimensions = new unsigned int[3];
//     m_ImageDimensions[0] = raw_image->GetDimensions()[0];
//     m_ImageDimensions[1] = raw_image->GetDimensions()[1];
//     m_ImageDimensions[2] = 1;

//     r->Initialize( mitk::MakePixelType<unsigned char,unsigned char,1>() ,3,m_ImageDimensions);
//     g->Initialize( mitk::MakePixelType<unsigned char,unsigned char,1>() ,3,m_ImageDimensions);
//     b->Initialize( mitk::MakePixelType<unsigned char,unsigned char,1>() ,3,m_ImageDimensions);

//     mitk::ImageReadAccessor inputAccForRGB(raw_image, raw_image->GetSliceData(0,0,0));
//     unsigned char* rbgSlice = (unsigned char*)inputAccForRGB.GetData();

//     mitk::ImageReadAccessor inputAcc(r, r->GetSliceData(0,0,0));
//     unsigned char* rData = (unsigned char*)inputAcc.GetData();

//     mitk::ImageReadAccessor inputAcc2(g, g->GetSliceData(0,0,0));
//     unsigned char* gData = (unsigned char*)inputAcc2.GetData();

//     mitk::ImageReadAccessor inputAcc3(b, b->GetSliceData(0,0,0));
//     unsigned char*
//         bData = (unsigned char*)inputAcc3.GetData();

//     for(int i = 0; i <  m_ImageDimensions[0] * m_ImageDimensions[1]*3; i+=3)
//     {
//       rData[i/3] = rbgSlice[i];
//       gData[i/3] = rbgSlice[i+1];
//       bData[i/3] = rbgSlice[i+2];

//     }

//  m_FeatureImageVector.push_back(r);
//  m_FeatureImageVector.push_back(g);
//  m_FeatureImageVector.push_back(b);

//  AddImageAsDataNode(r,"r");
//  AddImageAsDataNode(g,"g");
//  AddImageAsDataNode(b,"b");
