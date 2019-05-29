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

#include <QMessageBox>

#include "mitkWorkbenchUtil.h"

#include "PerfusionDataSimulationView.h"

#include "itkUnaryFunctorImageFilter.h"
#include <boost/tokenizer.hpp>
#include "mitkImageCast.h"
#include "mitkImageTimeSelector.h"
#include "mitkITKImageImport.h"
#include "mitkGaussianNoiseFunctor.h"
#include "mitkTwoCompartmentExchangeModel.h"
#include "mitkTwoCompartmentExchangeModelParameterizer.h"
#include "mitkTwoCompartmentExchangeModelFactory.h"
#include "mitkNumericTwoCompartmentExchangeModel.h"
#include "mitkNumericTwoCompartmentExchangeModelParameterizer.h"
#include "mitkNumericTwoCompartmentExchangeModelFactory.h"
#include "mitkExtendedToftsModel.h"
#include "mitkExtendedToftsModelFactory.h"
#include "mitkExtendedToftsModelParameterizer.h"
#include "mitkTwoTissueCompartmentModel.h"
#include "mitkTwoTissueCompartmentModelParameterizer.h"
#include "mitkTwoTissueCompartmentModelFactory.h"
#include "mitkOneTissueCompartmentModel.h"
#include "mitkOneTissueCompartmentModelParameterizer.h"
#include "mitkOneTissueCompartmentModelFactory.h"
#include "mitkModelSignalImageGenerator.h"
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateDataType.h>
#include "mitkSimpleFunctorBase.h"
#include "mitkArbitraryTimeGeometry.h"
#include <iostream>
 #include <QFileDialog>

const std::string PerfusionDataSimulationView::VIEW_ID = "org.mitk.gui.qt.pharmacokinetics.simulation";

inline double convertToDouble(const std::string& data)
{
  std::istringstream stepStream(data);
  stepStream.imbue(std::locale("C"));
  double value = 0.0;

  if (!(stepStream >> value) || !(stepStream.eof()))
  {
    mitkThrow() << "Cannot convert string to double. String: " << data;
  }
  return value;
}

inline double FindeMaxConcentration(std::vector<double> Concentration)
{
    double maximum = Concentration[0];
    for (std::vector<double>::size_type i =0; i<Concentration.size(); ++i)
    {
        maximum = std::max(maximum, Concentration[i]);
    }
return maximum;
}

void PerfusionDataSimulationView::SetFocus()
{
    m_Controls.GenerateBtn->setFocus();
}

void PerfusionDataSimulationView::CreateQtPartControl(QWidget* parent)
{
	m_Controls.setupUi(parent);
    m_Controls.GenerateBtn->setEnabled(false);

    m_Controls.groupBox_TM->hide();
    m_Controls.groupBox_2CXM->hide();
    m_Controls.groupBox_2TCM->hide();
    m_Controls.groupBox_1TCM->hide();
    m_Controls.groupBox_CNR->hide();

    this->InitModelComboBox();
    /** #2 @todo Reduce code ?
     */

    m_Controls.comboBox_F->SetDataStorage(this->GetDataStorage());
    m_Controls.comboBox_F->SetPredicate(m_IsNotABinaryImagePredicate);

    m_Controls.comboBox_PS->SetDataStorage(this->GetDataStorage());
    m_Controls.comboBox_PS->SetPredicate(m_IsNotABinaryImagePredicate);

    m_Controls.comboBox_fp->SetDataStorage(this->GetDataStorage());
    m_Controls.comboBox_fp->SetPredicate(m_IsNotABinaryImagePredicate);

    m_Controls.comboBox_fi->SetDataStorage(this->GetDataStorage());
    m_Controls.comboBox_fi->SetPredicate(m_IsNotABinaryImagePredicate);

    m_Controls.comboBox_Ktrans->SetDataStorage(this->GetDataStorage());
    m_Controls.comboBox_Ktrans->SetPredicate(m_IsNotABinaryImagePredicate);

    m_Controls.comboBox_vp->SetDataStorage(this->GetDataStorage());
    m_Controls.comboBox_vp->SetPredicate(m_IsNotABinaryImagePredicate);

    m_Controls.comboBox_ve->SetDataStorage(this->GetDataStorage());
    m_Controls.comboBox_ve->SetPredicate(m_IsNotABinaryImagePredicate);

    m_Controls.comboBox_K1->SetDataStorage(this->GetDataStorage());
    m_Controls.comboBox_K1->SetPredicate(m_IsNotABinaryImagePredicate);

    m_Controls.comboBox_K2->SetDataStorage(this->GetDataStorage());
    m_Controls.comboBox_K2->SetPredicate(m_IsNotABinaryImagePredicate);

    m_Controls.comboBox_K3->SetDataStorage(this->GetDataStorage());
    m_Controls.comboBox_K3->SetPredicate(m_IsNotABinaryImagePredicate);

    m_Controls.comboBox_K4->SetDataStorage(this->GetDataStorage());
    m_Controls.comboBox_K4->SetPredicate(m_IsNotABinaryImagePredicate);

    m_Controls.comboBox_VB->SetDataStorage(this->GetDataStorage());
    m_Controls.comboBox_VB->SetPredicate(m_IsNotABinaryImagePredicate);

    m_Controls.comboBox_k1->SetDataStorage(this->GetDataStorage());
    m_Controls.comboBox_k1->SetPredicate(m_IsNotABinaryImagePredicate);

    m_Controls.comboBox_k2->SetDataStorage(this->GetDataStorage());
    m_Controls.comboBox_k2->SetPredicate(m_IsNotABinaryImagePredicate);

    connect(m_Controls.AifFileBtn, SIGNAL(clicked()), this, SLOT(LoadAIFFile()));
    connect(m_Controls.ModelSelection, SIGNAL(currentIndexChanged(int)), this, SLOT(OnModellSet(int)));
    connect(m_Controls.GenerateBtn, SIGNAL(clicked()), this, SLOT(OnGenerateDataButtonClicked()));
    connect(m_Controls.comboBox_F, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSimulationConfigurationChanged()));
    connect(m_Controls.comboBox_PS, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSimulationConfigurationChanged()));
    connect(m_Controls.comboBox_fp, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSimulationConfigurationChanged()));
    connect(m_Controls.comboBox_fi, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSimulationConfigurationChanged()));
    connect(m_Controls.comboBox_VB, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSimulationConfigurationChanged()));
    connect(m_Controls.comboBox_K1, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSimulationConfigurationChanged()));
    connect(m_Controls.comboBox_K2, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSimulationConfigurationChanged()));
    connect(m_Controls.comboBox_K3, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSimulationConfigurationChanged()));
    connect(m_Controls.comboBox_K4, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSimulationConfigurationChanged()));
    connect(m_Controls.comboBox_k1, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSimulationConfigurationChanged()));
    connect(m_Controls.comboBox_k2, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSimulationConfigurationChanged()));
    connect(m_Controls.comboBox_Ktrans, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSimulationConfigurationChanged()));
    connect(m_Controls.comboBox_vp, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSimulationConfigurationChanged()));
    connect(m_Controls.comboBox_ve, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSimulationConfigurationChanged()));
    connect(m_Controls.CNRSpinBox, SIGNAL(valueChanged(double)),this, SLOT(OnSimulationConfigurationChanged()));
    connect(m_Controls.NoiseCheckBox, SIGNAL(stateChanged(int)),this, SLOT(OnSimulationConfigurationChanged()));

//    UpdateDataSelection();

  }

void PerfusionDataSimulationView::UpdateDataSelection()
{


}

//void PerfusionDataSimulationView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
//    const QList<mitk::DataNode::Pointer>& selectedNodes)
//{
//    UpdateDataSelection();
//    m_Controls.GenerateBtn->setEnabled(CheckModelSettings());

//}

void PerfusionDataSimulationView::OnSimulationConfigurationChanged()
{
//    UpdateDataSelection();

    m_Controls.groupBox_CNR->setVisible(m_Controls.NoiseCheckBox->isChecked());

    this->m_CNR = m_Controls.CNRSpinBox->value();

    m_Controls.GenerateBtn->setEnabled( CheckModelSettings() );


}


void PerfusionDataSimulationView::OnModellSet(int index)
{
  m_selectedModelFactory = NULL;

  if (index > 0)
  {
    if (static_cast<ModelFactoryStackType::size_type>(index) <= m_FactoryStack.size() )
    {
        m_selectedModelFactory = m_FactoryStack[index - 1];
    }
    else
    {
        MITK_WARN << "Invalid model index. Index outside of the factory stack. Factory stack size: "<< m_FactoryStack.size() << "; invalid index: "<< index;
    }
  }

  bool isToftsFactory = dynamic_cast<mitk::ExtendedToftsModelFactory*>(m_selectedModelFactory.GetPointer()) != NULL;
  bool is2CXMFactory = dynamic_cast<mitk::TwoCompartmentExchangeModelFactory*>(m_selectedModelFactory.GetPointer()) != NULL || dynamic_cast<mitk::NumericTwoCompartmentExchangeModelFactory*>(m_selectedModelFactory.GetPointer()) != NULL;
  bool is2TCMFactory = dynamic_cast<mitk::TwoTissueCompartmentModelFactory*>(m_selectedModelFactory.GetPointer()) != NULL;
  bool is1TCMFactory = dynamic_cast<mitk::OneTissueCompartmentModelFactory*>(m_selectedModelFactory.GetPointer()) != NULL;

  m_Controls.groupBox_TM->setVisible(isToftsFactory);
  m_Controls.groupBox_2CXM->setVisible(is2CXMFactory );
  m_Controls.groupBox_2TCM->setVisible(is2TCMFactory );
  m_Controls.groupBox_1TCM->setVisible(is1TCMFactory );


  m_Controls.GenerateBtn->setEnabled( CheckModelSettings() );
//  UpdateDataSelection();


}

bool PerfusionDataSimulationView::CheckModelSettings()
{
    bool ok = true;
    if(m_selectedModelFactory.IsNull())
    {
        return false;
    }

    if(this->m_AterialInputFunction.GetSize() == 0 || this->m_TimeGrid.GetSize() == 0)
    {
        return false;

    }

    bool isToftsFactory = dynamic_cast<mitk::ExtendedToftsModelFactory*>(m_selectedModelFactory.GetPointer()) != NULL;
    bool is2CXMFactory = dynamic_cast<mitk::TwoCompartmentExchangeModelFactory*>(m_selectedModelFactory.GetPointer()) != NULL || dynamic_cast<mitk::NumericTwoCompartmentExchangeModelFactory*>(m_selectedModelFactory.GetPointer()) != NULL;
    bool is2TCMFactory = dynamic_cast<mitk::TwoTissueCompartmentModelFactory*>(m_selectedModelFactory.GetPointer()) != NULL;
    bool is1TCMFactory = dynamic_cast<mitk::OneTissueCompartmentModelFactory*>(m_selectedModelFactory.GetPointer()) != NULL;


    if(isToftsFactory)
    {
        ok = ok && m_Controls.comboBox_Ktrans->GetSelectedNode().IsNotNull();
        ok = ok && m_Controls.comboBox_vp->GetSelectedNode().IsNotNull();
        ok = ok && m_Controls.comboBox_ve->GetSelectedNode().IsNotNull();
    }
    else if(is2CXMFactory)
    {
        ok = ok && m_Controls.comboBox_F->GetSelectedNode().IsNotNull();
        ok = ok && m_Controls.comboBox_PS->GetSelectedNode().IsNotNull();
        ok = ok && m_Controls.comboBox_fp->GetSelectedNode().IsNotNull();
        ok = ok && m_Controls.comboBox_fi->GetSelectedNode().IsNotNull();
    }

    else if(is2TCMFactory)
    {
        ok = ok && m_Controls.comboBox_K1->GetSelectedNode().IsNotNull();
        ok = ok && m_Controls.comboBox_K2->GetSelectedNode().IsNotNull();
        ok = ok && m_Controls.comboBox_K3->GetSelectedNode().IsNotNull();
        ok = ok && m_Controls.comboBox_K4->GetSelectedNode().IsNotNull();
        ok = ok && m_Controls.comboBox_VB->GetSelectedNode().IsNotNull();
    }
    else if(is1TCMFactory)
    {
        ok = ok && m_Controls.comboBox_k1->GetSelectedNode().IsNotNull();
        ok = ok && m_Controls.comboBox_k2->GetSelectedNode().IsNotNull();
     }

    else
    {
        return false;

    }
    if(m_Controls.NoiseCheckBox->isChecked())
    {
        if(m_CNR !=0 && m_MaxConcentration !=0)
        {
            this->m_Sigma = m_MaxConcentration/m_CNR;
        }
        if(m_Sigma==0)
        {
            return false;
        }
    }



    return ok;

}


void PerfusionDataSimulationView::InitModelComboBox() const
{
  this->m_Controls.ModelSelection->clear();
  this->m_Controls.ModelSelection->addItem(tr("No model selected"));

  for (ModelFactoryStackType::const_iterator pos = m_FactoryStack.begin(); pos != m_FactoryStack.end(); ++pos)
  {
    this->m_Controls.ModelSelection->addItem(QString::fromStdString((*pos)->GetClassID()));
  }
  this->m_Controls.ModelSelection->setCurrentIndex(0);
};


void PerfusionDataSimulationView::LoadAIFFile()
{
  QFileDialog dialog;
  dialog.setNameFilter(tr("Images (*.csv"));

  QString fileName = dialog.getOpenFileName();

  m_Controls.AifFilePath->setText( fileName );

  std::string aifFilePath = fileName.toStdString();

  //Read Input
  typedef boost::tokenizer< boost::escaped_list_separator<char> > Tokenizer;
  /////////////////////////////////////////////////////////////////////////////////////////////////
  //AIF Data

  std::vector<double> inputFunction;
  std::vector<double> inputGrid;

  std::ifstream in1(aifFilePath.c_str());

  if(!in1.is_open())
  {
      m_Controls.errorMessageLabel->setText("Could not open AIF File!");
  }


  std::vector< std::string > vec1;
  std::string line1;

  while (getline(in1,line1))
  {
      Tokenizer tok(line1);
      vec1.assign(tok.begin(),tok.end());

//        if (vec1.size() < 3) continue;

      inputGrid.push_back(convertToDouble(vec1[0]));
      inputFunction.push_back(convertToDouble(vec1[1]));

  }

  this->m_MaxConcentration = FindeMaxConcentration(inputFunction);

  itk::Array<double> aif;
  itk::Array<double> grid;

  aif.SetSize(inputFunction.size());
  grid.SetSize(inputGrid.size());

  aif.fill(0.0);
  grid.fill(0.0);

  itk::Array<double>::iterator aifPos = aif.begin();
  for(std::vector<double>::const_iterator pos = inputFunction.begin(); pos != inputFunction.end(); ++pos, ++aifPos)
  {
      *aifPos = *pos;
  }
  itk::Array<double>::iterator gridPos = grid.begin();
  for(std::vector<double>::const_iterator pos = inputGrid.begin(); pos != inputGrid.end(); ++pos, ++gridPos)
  {
      *gridPos = *pos;
  }

  this->m_AterialInputFunction = aif;
  this->m_TimeGrid = grid;


  m_Controls.GenerateBtn->setEnabled( CheckModelSettings() );
//  UpdateDataSelection();



}
/** @todo #2 Same function for Numeric and analytic version of FillParameterMap2CXM
 */
void PerfusionDataSimulationView::FillParameterMap2CXM()
{
    ParameterMapType stack;

    mitk::DataNode::Pointer m_selectedNode = m_Controls.comboBox_F->GetSelectedNode();
    mitk::Image::Pointer m_selectedImage = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
    stack.insert(std::make_pair(mitk::TwoCompartmentExchangeModel::NAME_PARAMETER_F,m_selectedImage));

    m_selectedNode = m_Controls.comboBox_PS->GetSelectedNode();
    m_selectedImage = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
    stack.insert(std::make_pair(mitk::TwoCompartmentExchangeModel::NAME_PARAMETER_PS,m_selectedImage));

    m_selectedNode = m_Controls.comboBox_fp->GetSelectedNode();
    m_selectedImage = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
    stack.insert(std::make_pair(mitk::TwoCompartmentExchangeModel::NAME_PARAMETER_vp,m_selectedImage));

    m_selectedNode = m_Controls.comboBox_fi->GetSelectedNode();
    m_selectedImage = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
    stack.insert(std::make_pair(mitk::TwoCompartmentExchangeModel::NAME_PARAMETER_ve,m_selectedImage));

    this->m_ParameterImageMap = stack;

}


void PerfusionDataSimulationView::FillParameterMapNumeric2CXM()
{
    ParameterMapType stack;

    mitk::DataNode::Pointer m_selectedNode = m_Controls.comboBox_F->GetSelectedNode();
    mitk::Image::Pointer m_selectedImage = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
    stack.insert(std::make_pair(mitk::NumericTwoCompartmentExchangeModel::NAME_PARAMETER_F,m_selectedImage));

    m_selectedNode = m_Controls.comboBox_PS->GetSelectedNode();
    m_selectedImage = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
    stack.insert(std::make_pair(mitk::NumericTwoCompartmentExchangeModel::NAME_PARAMETER_PS,m_selectedImage));

    m_selectedNode = m_Controls.comboBox_fp->GetSelectedNode();
    m_selectedImage = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
    stack.insert(std::make_pair(mitk::NumericTwoCompartmentExchangeModel::NAME_PARAMETER_vp,m_selectedImage));

    m_selectedNode = m_Controls.comboBox_fi->GetSelectedNode();
    m_selectedImage = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
    stack.insert(std::make_pair(mitk::NumericTwoCompartmentExchangeModel::NAME_PARAMETER_ve,m_selectedImage));

    this->m_ParameterImageMap = stack;

}


void PerfusionDataSimulationView::FillParameterMapETM()
{
    ParameterMapType stack;

    mitk::DataNode::Pointer m_selectedNode = m_Controls.comboBox_Ktrans->GetSelectedNode();
    mitk::Image::Pointer m_selectedImage = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
    stack.insert(std::make_pair(mitk::ExtendedToftsModel::NAME_PARAMETER_Ktrans,m_selectedImage));

    m_selectedNode = m_Controls.comboBox_vp->GetSelectedNode();
    m_selectedImage = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
    stack.insert(std::make_pair(mitk::ExtendedToftsModel::NAME_PARAMETER_vp,m_selectedImage));

    m_selectedNode = m_Controls.comboBox_ve->GetSelectedNode();
    m_selectedImage = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
    stack.insert(std::make_pair(mitk::ExtendedToftsModel::NAME_PARAMETER_ve,m_selectedImage));


    this->m_ParameterImageMap = stack;

}

void PerfusionDataSimulationView::FillParameterMap2TCM()
{
    ParameterMapType stack;

    mitk::DataNode::Pointer m_selectedNode = m_Controls.comboBox_K1->GetSelectedNode();
    mitk::Image::Pointer m_selectedImage = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
    stack.insert(std::make_pair(mitk::TwoTissueCompartmentModel::NAME_PARAMETER_K1,m_selectedImage));

    m_selectedNode = m_Controls.comboBox_K2->GetSelectedNode();
    m_selectedImage = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
    stack.insert(std::make_pair(mitk::TwoTissueCompartmentModel::NAME_PARAMETER_k2,m_selectedImage));

    m_selectedNode = m_Controls.comboBox_K3->GetSelectedNode();
    m_selectedImage = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
    stack.insert(std::make_pair(mitk::TwoTissueCompartmentModel::NAME_PARAMETER_k3,m_selectedImage));

    m_selectedNode = m_Controls.comboBox_K4->GetSelectedNode();
    m_selectedImage = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
    stack.insert(std::make_pair(mitk::TwoTissueCompartmentModel::NAME_PARAMETER_k4,m_selectedImage));

    m_selectedNode = m_Controls.comboBox_VB->GetSelectedNode();
    m_selectedImage = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
    stack.insert(std::make_pair(mitk::TwoTissueCompartmentModel::NAME_PARAMETER_VB,m_selectedImage));

    this->m_ParameterImageMap = stack;

}

void PerfusionDataSimulationView::FillParameterMap1TCM()
{
    ParameterMapType stack;

    mitk::DataNode::Pointer m_selectedNode = m_Controls.comboBox_k1->GetSelectedNode();
    mitk::Image::Pointer m_selectedImage = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
    stack.insert(std::make_pair(mitk::OneTissueCompartmentModel::NAME_PARAMETER_k1,m_selectedImage));

    m_selectedNode = m_Controls.comboBox_k2->GetSelectedNode();
    m_selectedImage = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
    stack.insert(std::make_pair(mitk::OneTissueCompartmentModel::NAME_PARAMETER_k2,m_selectedImage));

    this->m_ParameterImageMap = stack;

}

void PerfusionDataSimulationView::OnGenerateDataButtonClicked()
{
    mitk::Image::Pointer m_DynamicImage = mitk::Image::New();

    bool isToftsFactory = dynamic_cast<mitk::ExtendedToftsModelFactory*>(m_selectedModelFactory.GetPointer()) != NULL;
    bool isPhysBrixFactory = dynamic_cast<mitk::TwoCompartmentExchangeModelFactory*>(m_selectedModelFactory.GetPointer()) != NULL;
    bool isNumPhysBrixFactory = dynamic_cast<mitk::NumericTwoCompartmentExchangeModelFactory*>(m_selectedModelFactory.GetPointer()) != NULL;
    bool is2TCMFactory = dynamic_cast<mitk::TwoTissueCompartmentModelFactory*>(m_selectedModelFactory.GetPointer()) != NULL;
    bool is1TCMFactory = dynamic_cast<mitk::OneTissueCompartmentModelFactory*>(m_selectedModelFactory.GetPointer()) != NULL;


    if(isToftsFactory)
    {
       this->FillParameterMapETM();
        m_DynamicImage = this->GenerateETModelData();
    }

    if (isPhysBrixFactory)
    {
        this->FillParameterMap2CXM();
        m_DynamicImage = this->Generate2CXModelData();
    }

    if (isNumPhysBrixFactory)
    {
        this->FillParameterMapNumeric2CXM();
        m_DynamicImage = this->GenerateNumeric2CXModelData();
    }

    if (is2TCMFactory)
    {
        this->FillParameterMap2TCM();
        m_DynamicImage = this->Generate2TCModelData();
    }
    if (is1TCMFactory)
    {
        this->FillParameterMap1TCM();
        m_DynamicImage = this->Generate1TCModelData();
    }



    mitk::DataNode::Pointer resultNode = mitk::DataNode::New();
    std::string nameOfResultImage = "SimulationData_";
    nameOfResultImage.append(m_selectedModelFactory->GetModelDisplayName());
    resultNode->SetProperty("name", mitk::StringProperty::New(nameOfResultImage) );
    resultNode->SetData(m_DynamicImage); // set data of new node
    this->GetDataStorage()->Add(resultNode);


}

mitk::Image::Pointer PerfusionDataSimulationView::Generate2CXModelData()
{
mitk::ModelSignalImageGenerator::Pointer generator = mitk::ModelSignalImageGenerator::New();
mitk::TwoCompartmentExchangeModelParameterizer::Pointer modelParameterizer = mitk::TwoCompartmentExchangeModelParameterizer::New();

/** @todo #2  necessary? Generator need to have a map with Parameters in order of Model in order to pass vector parameters correctly to Model.
 * I wanted to make it independend from the order the images are passed on
 */
for(ParameterMapType::const_iterator pos = this->m_ParameterImageMap.begin(); pos != this->m_ParameterImageMap.end(); ++pos)
{
    if(pos->first == mitk::TwoCompartmentExchangeModel::NAME_PARAMETER_F)
    {
        generator->SetParameterInputImage(mitk::TwoCompartmentExchangeModel::POSITION_PARAMETER_F, pos->second);
    }
    if(pos->first == mitk::TwoCompartmentExchangeModel::NAME_PARAMETER_PS)
    {
        generator->SetParameterInputImage(mitk::TwoCompartmentExchangeModel::POSITION_PARAMETER_PS, pos->second);
    }
    if(pos->first == mitk::TwoCompartmentExchangeModel::NAME_PARAMETER_ve)
    {
        generator->SetParameterInputImage(mitk::TwoCompartmentExchangeModel::POSITION_PARAMETER_ve, pos->second);
    }
    if(pos->first == mitk::TwoCompartmentExchangeModel::NAME_PARAMETER_vp)
    {
        generator->SetParameterInputImage(mitk::TwoCompartmentExchangeModel::POSITION_PARAMETER_vp, pos->second);
    }
}

modelParameterizer->SetAIF(this->m_AterialInputFunction);
modelParameterizer->SetAIFTimeGrid(this->m_TimeGrid);
modelParameterizer->SetDefaultTimeGrid(this->m_TimeGrid);

generator->SetParameterizer(modelParameterizer);

mitk::Image::Pointer generatedImage = generator->GetGeneratedImage();

 mitk::Image::Pointer resultImage = mitk::Image::New();
if(m_Controls.NoiseCheckBox->isChecked())
{
    typedef itk::Image<double,3> ImageType;

    mitk::Image::Pointer tempImage = mitk::Image::New();
    tempImage->Initialize(generatedImage);

    mitk::ArbitraryTimeGeometry* timeGeometry = dynamic_cast<mitk::ArbitraryTimeGeometry*> (generatedImage->GetTimeGeometry());

    tempImage->SetTimeGeometry(timeGeometry);

    ImageType::Pointer itkImage = ImageType::New();
    mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();
    imageTimeSelector->SetInput(generatedImage);

    for(unsigned int i = 0; i< generatedImage->GetTimeSteps(); ++i)
    {
        imageTimeSelector->SetTimeNr(i);
        imageTimeSelector->UpdateLargestPossibleRegion();

        mitk::Image::Pointer mitkInputImage = imageTimeSelector->GetOutput();

        mitk::CastToItkImage(mitkInputImage, itkImage );

        typedef mitk::GaussianNoiseFunctor<double,double> NoiseFunctorType;
        typedef itk::UnaryFunctorImageFilter<ImageType,ImageType, NoiseFunctorType > NoiseFilterType;
        NoiseFilterType::Pointer noiseFilter = NoiseFilterType::New();
        NoiseFunctorType noiseFunctor;
        noiseFunctor.SetMean(0.0);
        noiseFunctor.SetSigma(this->m_Sigma);
        noiseFilter->SetFunctor(noiseFunctor);

        noiseFilter->SetInput(itkImage);

        mitk::Image::Pointer outputImage = mitk::ImportItkImage(noiseFilter->GetOutput())->Clone();

        mitk::ImageReadAccessor accessor(outputImage);
        tempImage->SetVolume(accessor.GetData(), i);
    }

   resultImage = tempImage->Clone();
}
else
{
    resultImage = generatedImage;
}
return resultImage;
}


/** @todo #2  Synergie? Function implementation for every Model to complicated?
 */
mitk::Image::Pointer PerfusionDataSimulationView::GenerateNumeric2CXModelData()
{
mitk::ModelSignalImageGenerator::Pointer generator = mitk::ModelSignalImageGenerator::New();
mitk::NumericTwoCompartmentExchangeModelParameterizer::Pointer modelParameterizer = mitk::NumericTwoCompartmentExchangeModelParameterizer::New();

for(ParameterMapType::const_iterator pos = this->m_ParameterImageMap.begin(); pos != this->m_ParameterImageMap.end(); ++pos)
{
    if(pos->first == mitk::NumericTwoCompartmentExchangeModel::NAME_PARAMETER_F)
    {
        generator->SetParameterInputImage(mitk::NumericTwoCompartmentExchangeModel::POSITION_PARAMETER_F, pos->second);
    }
    if(pos->first == mitk::NumericTwoCompartmentExchangeModel::NAME_PARAMETER_PS)
    {
        generator->SetParameterInputImage(mitk::NumericTwoCompartmentExchangeModel::POSITION_PARAMETER_PS, pos->second);
    }
    if(pos->first == mitk::NumericTwoCompartmentExchangeModel::NAME_PARAMETER_ve)
    {
        generator->SetParameterInputImage(mitk::NumericTwoCompartmentExchangeModel::POSITION_PARAMETER_ve, pos->second);
    }
    if(pos->first == mitk::NumericTwoCompartmentExchangeModel::NAME_PARAMETER_vp)
    {
        generator->SetParameterInputImage(mitk::NumericTwoCompartmentExchangeModel::POSITION_PARAMETER_vp, pos->second);
    }
}

modelParameterizer->SetAIF(this->m_AterialInputFunction);
modelParameterizer->SetAIFTimeGrid(this->m_TimeGrid);
modelParameterizer->SetDefaultTimeGrid(this->m_TimeGrid);
modelParameterizer->SetODEINTStepSize(0.05);

generator->SetParameterizer(modelParameterizer);

mitk::Image::Pointer generatedImage = generator->GetGeneratedImage();


mitk::Image::Pointer resultImage = mitk::Image::New();
if(m_Controls.NoiseCheckBox->isChecked())
{
   typedef itk::Image<double,3> ImageType;

   mitk::Image::Pointer tempImage = mitk::Image::New();
   tempImage->Initialize(generatedImage);

   mitk::ArbitraryTimeGeometry* timeGeometry = dynamic_cast<mitk::ArbitraryTimeGeometry*> (generatedImage->GetTimeGeometry());

   tempImage->SetTimeGeometry(timeGeometry);

   ImageType::Pointer itkImage = ImageType::New();
   mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();
   imageTimeSelector->SetInput(generatedImage);

   for(unsigned int i = 0; i< generatedImage->GetTimeSteps(); ++i)
   {
       imageTimeSelector->SetTimeNr(i);
       imageTimeSelector->UpdateLargestPossibleRegion();

       mitk::Image::Pointer mitkInputImage = imageTimeSelector->GetOutput();

       mitk::CastToItkImage(mitkInputImage, itkImage );

       typedef mitk::GaussianNoiseFunctor<double,double> NoiseFunctorType;
       typedef itk::UnaryFunctorImageFilter<ImageType,ImageType, NoiseFunctorType > NoiseFilterType;
       NoiseFilterType::Pointer noiseFilter = NoiseFilterType::New();
       NoiseFunctorType noiseFunctor;
       noiseFunctor.SetMean(0.0);
       noiseFunctor.SetSigma(this->m_Sigma);
       noiseFilter->SetFunctor(noiseFunctor);

       noiseFilter->SetInput(itkImage);

       mitk::Image::Pointer outputImage = mitk::ImportItkImage(noiseFilter->GetOutput())->Clone();

       mitk::ImageReadAccessor accessor(outputImage);
       tempImage->SetVolume(accessor.GetData(), i);
   }

  resultImage = tempImage->Clone();
}
else
{
   resultImage = generatedImage;
}
return resultImage;
}

mitk::Image::Pointer PerfusionDataSimulationView::GenerateETModelData()
{
mitk::ModelSignalImageGenerator::Pointer generator = mitk::ModelSignalImageGenerator::New();
mitk::ExtendedToftsModelParameterizer::Pointer modelParameterizer = mitk::ExtendedToftsModelParameterizer::New();

for(ParameterMapType::const_iterator pos = this->m_ParameterImageMap.begin(); pos != this->m_ParameterImageMap.end(); ++pos)
{
    if(pos->first == mitk::ExtendedToftsModel::NAME_PARAMETER_Ktrans)
    {
        generator->SetParameterInputImage(mitk::ExtendedToftsModel::POSITION_PARAMETER_Ktrans, pos->second);
    }
    if(pos->first == mitk::ExtendedToftsModel::NAME_PARAMETER_vp)
    {
        generator->SetParameterInputImage(mitk::ExtendedToftsModel::POSITION_PARAMETER_vp, pos->second);
    }
    if(pos->first == mitk::ExtendedToftsModel::NAME_PARAMETER_ve)
    {
        generator->SetParameterInputImage(mitk::ExtendedToftsModel::POSITION_PARAMETER_ve, pos->second);
    }

}

modelParameterizer->SetAIF(this->m_AterialInputFunction);
modelParameterizer->SetAIFTimeGrid(this->m_TimeGrid);
modelParameterizer->SetDefaultTimeGrid(this->m_TimeGrid);

generator->SetParameterizer(modelParameterizer);

mitk::Image::Pointer generatedImage = generator->GetGeneratedImage();


mitk::Image::Pointer resultImage = mitk::Image::New();
if(m_Controls.NoiseCheckBox->isChecked())
{
   typedef itk::Image<double,3> ImageType;

   mitk::Image::Pointer tempImage = mitk::Image::New();
   tempImage->Initialize(generatedImage);

   mitk::ArbitraryTimeGeometry* timeGeometry = dynamic_cast<mitk::ArbitraryTimeGeometry*> (generatedImage->GetTimeGeometry());

   tempImage->SetTimeGeometry(timeGeometry);

   ImageType::Pointer itkImage = ImageType::New();
   mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();
   imageTimeSelector->SetInput(generatedImage);

   for(unsigned int i = 0; i< generatedImage->GetTimeSteps(); ++i)
   {
       imageTimeSelector->SetTimeNr(i);
       imageTimeSelector->UpdateLargestPossibleRegion();

       mitk::Image::Pointer mitkInputImage = imageTimeSelector->GetOutput();

       mitk::CastToItkImage(mitkInputImage, itkImage );

       typedef mitk::GaussianNoiseFunctor<double,double> NoiseFunctorType;
       typedef itk::UnaryFunctorImageFilter<ImageType,ImageType, NoiseFunctorType > NoiseFilterType;
       NoiseFilterType::Pointer noiseFilter = NoiseFilterType::New();
       NoiseFunctorType noiseFunctor;
       noiseFunctor.SetMean(0.0);
       noiseFunctor.SetSigma(this->m_Sigma);
       noiseFilter->SetFunctor(noiseFunctor);

       noiseFilter->SetInput(itkImage);

       mitk::Image::Pointer outputImage = mitk::ImportItkImage(noiseFilter->GetOutput())->Clone();

       mitk::ImageReadAccessor accessor(outputImage);
       tempImage->SetVolume(accessor.GetData(), i);
   }

  resultImage = tempImage->Clone();
}
else
{
   resultImage = generatedImage;
}
return resultImage;
}


mitk::Image::Pointer PerfusionDataSimulationView::Generate2TCModelData()
{
mitk::ModelSignalImageGenerator::Pointer generator = mitk::ModelSignalImageGenerator::New();
mitk::TwoTissueCompartmentModelParameterizer::Pointer modelParameterizer = mitk::TwoTissueCompartmentModelParameterizer::New();


for(ParameterMapType::const_iterator pos = this->m_ParameterImageMap.begin(); pos != this->m_ParameterImageMap.end(); ++pos)
{
    if(pos->first == mitk::TwoTissueCompartmentModel::NAME_PARAMETER_K1)
    {
        generator->SetParameterInputImage(mitk::TwoTissueCompartmentModel::POSITION_PARAMETER_K1, pos->second);
    }
    if(pos->first == mitk::TwoTissueCompartmentModel::NAME_PARAMETER_k2)
    {
        generator->SetParameterInputImage(mitk::TwoTissueCompartmentModel::POSITION_PARAMETER_k2, pos->second);
    }
    if(pos->first == mitk::TwoTissueCompartmentModel::NAME_PARAMETER_k3)
    {
        generator->SetParameterInputImage(mitk::TwoTissueCompartmentModel::POSITION_PARAMETER_k3, pos->second);
    }
    if(pos->first == mitk::TwoTissueCompartmentModel::NAME_PARAMETER_k4)
    {
        generator->SetParameterInputImage(mitk::TwoTissueCompartmentModel::POSITION_PARAMETER_k4, pos->second);
    }
    if(pos->first == mitk::TwoTissueCompartmentModel::NAME_PARAMETER_VB)
    {
        generator->SetParameterInputImage(mitk::TwoTissueCompartmentModel::POSITION_PARAMETER_VB, pos->second);
    }

}

modelParameterizer->SetAIF(this->m_AterialInputFunction);
modelParameterizer->SetAIFTimeGrid(this->m_TimeGrid);
modelParameterizer->SetDefaultTimeGrid(this->m_TimeGrid);

generator->SetParameterizer(modelParameterizer);

mitk::Image::Pointer generatedImage = generator->GetGeneratedImage();

 mitk::Image::Pointer resultImage = mitk::Image::New();
if(m_Controls.NoiseCheckBox->isChecked())
{
    typedef itk::Image<double,3> ImageType;

    mitk::Image::Pointer tempImage = mitk::Image::New();
    tempImage->Initialize(generatedImage);

    mitk::ArbitraryTimeGeometry* timeGeometry = dynamic_cast<mitk::ArbitraryTimeGeometry*> (generatedImage->GetTimeGeometry());

    tempImage->SetTimeGeometry(timeGeometry);

    ImageType::Pointer itkImage = ImageType::New();
    mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();
    imageTimeSelector->SetInput(generatedImage);

    for(unsigned int i = 0; i< generatedImage->GetTimeSteps(); ++i)
    {
        imageTimeSelector->SetTimeNr(i);
        imageTimeSelector->UpdateLargestPossibleRegion();

        mitk::Image::Pointer mitkInputImage = imageTimeSelector->GetOutput();

        mitk::CastToItkImage(mitkInputImage, itkImage );

        typedef mitk::GaussianNoiseFunctor<double,double> NoiseFunctorType;
        typedef itk::UnaryFunctorImageFilter<ImageType,ImageType, NoiseFunctorType > NoiseFilterType;
        NoiseFilterType::Pointer noiseFilter = NoiseFilterType::New();
        NoiseFunctorType noiseFunctor;
        noiseFunctor.SetMean(0.0);
        noiseFunctor.SetSigma(this->m_Sigma);
        noiseFilter->SetFunctor(noiseFunctor);

        noiseFilter->SetInput(itkImage);

        mitk::Image::Pointer outputImage = mitk::ImportItkImage(noiseFilter->GetOutput())->Clone();

        mitk::ImageReadAccessor accessor(outputImage);
        tempImage->SetVolume(accessor.GetData(), i);
    }

   resultImage = tempImage->Clone();
}
else
{
    resultImage = generatedImage;
}
return resultImage;
}


mitk::Image::Pointer PerfusionDataSimulationView::Generate1TCModelData()
{
mitk::ModelSignalImageGenerator::Pointer generator = mitk::ModelSignalImageGenerator::New();
mitk::OneTissueCompartmentModelParameterizer::Pointer modelParameterizer = mitk::OneTissueCompartmentModelParameterizer::New();


for(ParameterMapType::const_iterator pos = this->m_ParameterImageMap.begin(); pos != this->m_ParameterImageMap.end(); ++pos)
{
    if(pos->first == mitk::OneTissueCompartmentModel::NAME_PARAMETER_k1)
    {
        generator->SetParameterInputImage(mitk::OneTissueCompartmentModel::POSITION_PARAMETER_k1, pos->second);
    }
    if(pos->first == mitk::OneTissueCompartmentModel::NAME_PARAMETER_k2)
    {
        generator->SetParameterInputImage(mitk::OneTissueCompartmentModel::POSITION_PARAMETER_k2, pos->second);
    }

}

modelParameterizer->SetAIF(this->m_AterialInputFunction);
modelParameterizer->SetAIFTimeGrid(this->m_TimeGrid);
modelParameterizer->SetDefaultTimeGrid(this->m_TimeGrid);

generator->SetParameterizer(modelParameterizer);

mitk::Image::Pointer generatedImage = generator->GetGeneratedImage();

 mitk::Image::Pointer resultImage = mitk::Image::New();
if(m_Controls.NoiseCheckBox->isChecked())
{
    typedef itk::Image<double,3> ImageType;

    mitk::Image::Pointer tempImage = mitk::Image::New();
    tempImage->Initialize(generatedImage);

    mitk::ArbitraryTimeGeometry* timeGeometry = dynamic_cast<mitk::ArbitraryTimeGeometry*> (generatedImage->GetTimeGeometry());

    tempImage->SetTimeGeometry(timeGeometry);

    ImageType::Pointer itkImage = ImageType::New();
    mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();
    imageTimeSelector->SetInput(generatedImage);

    for(unsigned int i = 0; i< generatedImage->GetTimeSteps(); ++i)
    {
        imageTimeSelector->SetTimeNr(i);
        imageTimeSelector->UpdateLargestPossibleRegion();

        mitk::Image::Pointer mitkInputImage = imageTimeSelector->GetOutput();

        mitk::CastToItkImage(mitkInputImage, itkImage );

        typedef mitk::GaussianNoiseFunctor<double,double> NoiseFunctorType;
        typedef itk::UnaryFunctorImageFilter<ImageType,ImageType, NoiseFunctorType > NoiseFilterType;
        NoiseFilterType::Pointer noiseFilter = NoiseFilterType::New();
        NoiseFunctorType noiseFunctor;
        noiseFunctor.SetMean(0.0);
        noiseFunctor.SetSigma(this->m_Sigma);
        noiseFilter->SetFunctor(noiseFunctor);

        noiseFilter->SetInput(itkImage);

        mitk::Image::Pointer outputImage = mitk::ImportItkImage(noiseFilter->GetOutput())->Clone();

        mitk::ImageReadAccessor accessor(outputImage);
        tempImage->SetVolume(accessor.GetData(), i);
    }

   resultImage = tempImage->Clone();
}
else
{
    resultImage = generatedImage;
}
return resultImage;
}


PerfusionDataSimulationView::PerfusionDataSimulationView()
{
     m_Sigma = 0;
     m_CNR=0;
     m_MaxConcentration=0;
     m_selectedModelFactory = NULL;


     mitk::ModelFactoryBase::Pointer factory = mitk::ExtendedToftsModelFactory::New().GetPointer();
     m_FactoryStack.push_back(factory);
     factory = mitk::TwoCompartmentExchangeModelFactory::New().GetPointer();
     m_FactoryStack.push_back(factory);
     factory = mitk::NumericTwoCompartmentExchangeModelFactory::New().GetPointer();
     m_FactoryStack.push_back(factory);
     factory = mitk::TwoTissueCompartmentModelFactory::New().GetPointer();
     m_FactoryStack.push_back(factory);
     factory = mitk::OneTissueCompartmentModelFactory::New().GetPointer();
     m_FactoryStack.push_back(factory);



     m_IsNotABinaryImagePredicate = mitk::NodePredicateAnd::New(
                 mitk::TNodePredicateDataType<mitk::Image>::New(),
                 mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true))),
                 mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))).GetPointer();

}
