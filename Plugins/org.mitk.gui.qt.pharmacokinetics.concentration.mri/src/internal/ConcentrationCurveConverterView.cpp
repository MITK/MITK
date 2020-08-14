/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QMessageBox>

#include "mitkWorkbenchUtil.h"

#include "ConcentrationCurveConverterView.h"
#include "mitkConcentrationCurveGenerator.h"
#include "mitkNodePredicateDataType.h"
#include "mitkConvertToConcentrationTurboFlashFunctor.h"
#include "mitkConvertToConcentrationAbsoluteFunctor.h"
#include "mitkConvertToConcentrationRelativeFunctor.h"
#include "itkBinaryFunctorImageFilter.h"
#include "boost/math/constants/constants.hpp"


// Includes for image casting between ITK and MITK
#include "mitkImageTimeSelector.h"
#include "mitkImageCast.h"
#include "mitkITKImageImport.h"
#include <mitkImagePixelReadAccessor.h>

#include <iostream>

const std::string ConcentrationCurveConverterView::VIEW_ID = "org.mitk.ConcentrationCurveConverterView";

void ConcentrationCurveConverterView::SetFocus()
{
    m_Controls.btnConvertToConcentration->setFocus();
}

void ConcentrationCurveConverterView::CreateQtPartControl(QWidget* parent)
{
	m_Controls.setupUi(parent);
    m_Controls.btnConvertToConcentration->setEnabled(false);

    connect(m_Controls.btnConvertToConcentration, SIGNAL(clicked()), this, SLOT(OnConvertToConcentrationButtonClicked()));

    m_Controls.groupBox_T1->hide();
    m_Controls.groupBox_T2->hide();
    m_Controls.groupBox3D->hide();
    m_Controls.groupBox4D->hide();
    m_Controls.groupBoxTurboFlash->hide();
    m_Controls.groupConcentration->hide();

    connect(m_Controls.radioButton_T1, SIGNAL(toggled(bool)),this, SLOT(OnSettingChanged()));
    connect(m_Controls.radioButton_T2, SIGNAL(toggled(bool)),this, SLOT(OnSettingChanged()));

    connect(m_Controls.radioButton3D, SIGNAL(toggled(bool)),this, SLOT(OnSettingChanged()));
    connect(m_Controls.radioButton4D, SIGNAL(toggled(bool)),this, SLOT(OnSettingChanged()));


  //Concentration
    m_Controls.groupConcentration->hide();
    m_Controls.groupBoxEnhancement->hide();
    m_Controls.groupBoxTurboFlash->hide();
    m_Controls.groupBox_T1MapviaVFA->hide();

    m_Controls.spinBox_baselineStartTimeStep->setValue(0);
    m_Controls.spinBox_baselineEndTimeStep->setValue(0);

    m_Controls.spinBox_baselineEndTimeStep->setMinimum(0);
    m_Controls.spinBox_baselineStartTimeStep->setMinimum(0);

    connect(m_Controls.radioButtonTurboFlash, SIGNAL(toggled(bool)), m_Controls.groupBoxTurboFlash, SLOT(setVisible(bool)));
    connect(m_Controls.radioButtonTurboFlash, SIGNAL(toggled(bool)), this, SLOT(OnSettingChanged()));
    connect(m_Controls.relaxationtime, SIGNAL(valueChanged(double)), this, SLOT(OnSettingChanged()));
    connect(m_Controls.recoverytime, SIGNAL(valueChanged(double)), this, SLOT(OnSettingChanged()));
    connect(m_Controls.relaxivity, SIGNAL(valueChanged(double)), this, SLOT(OnSettingChanged()));


    connect(m_Controls.radioButton_absoluteEnhancement, SIGNAL(toggled(bool)), this, SLOT(OnSettingChanged()));
    connect(m_Controls.radioButton_relativeEnchancement, SIGNAL(toggled(bool)), this, SLOT(OnSettingChanged()));
    connect(m_Controls.radioButton_absoluteEnhancement, SIGNAL(toggled(bool)), m_Controls.groupBoxEnhancement, SLOT(setVisible(bool)));
    connect(m_Controls.radioButton_relativeEnchancement, SIGNAL(toggled(bool)), m_Controls.groupBoxEnhancement, SLOT(setVisible(bool)));


    connect(m_Controls.factorSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnSettingChanged()));
    connect(m_Controls.spinBox_baselineStartTimeStep, SIGNAL(valueChanged(int)), this, SLOT(OnSettingChanged()));
    connect(m_Controls.spinBox_baselineEndTimeStep, SIGNAL(valueChanged(int)), this, SLOT(OnSettingChanged()));

    connect(m_Controls.radioButtonUsingT1viaVFA, SIGNAL(toggled(bool)), m_Controls.groupBox_T1MapviaVFA, SLOT(setVisible(bool)));
    connect(m_Controls.radioButtonUsingT1viaVFA, SIGNAL(toggled(bool)), this, SLOT(OnSettingChanged()));
    connect(m_Controls.FlipangleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnSettingChanged()));
    connect(m_Controls.RelaxivitySpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnSettingChanged()));
    connect(m_Controls.TRSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnSettingChanged()));

    //m_Controls.PDWImageNodeSelector->SetNodePredicate(m_isValidPDWImagePredicate);
    m_Controls.PDWImageNodeSelector->SetDataStorage(this->GetDataStorage());
    m_Controls.PDWImageNodeSelector->SetInvalidInfo("Please select PDW Image.");
    m_Controls.PDWImageNodeSelector->setEnabled(false);

    connect(m_Controls.radioButtonUsingT1viaVFA, SIGNAL(toggled(bool)), m_Controls.PDWImageNodeSelector, SLOT(setEnabled(bool)));


}


void ConcentrationCurveConverterView::OnSettingChanged()
{
  bool ok = true;
  m_Controls.groupBox_T1->setVisible(m_Controls.radioButton_T1->isChecked());
  m_Controls.groupBox_T2->setVisible(m_Controls.radioButton_T2->isChecked());

  if(m_Controls.radioButton_T1->isChecked())
  {
      m_Controls.groupBox3D->setVisible(m_Controls.radioButton3D->isChecked());
      m_Controls.groupBox4D->setVisible(m_Controls.radioButton4D->isChecked());

      if(m_Controls.radioButton4D->isChecked())
      {
          m_Controls.groupConcentration->setVisible(true);
          ok = m_selectedImage.IsNotNull() && CheckSettings();
      }
      else if(m_Controls.radioButton3D->isChecked())
      {
          m_Controls.groupConcentration->setVisible(true);
          ok = m_selectedImage.IsNotNull() && m_selectedBaselineImage.IsNotNull() && CheckSettings();

      }
  }
  else if (m_Controls.radioButton_T2->isChecked())
  {
      m_Controls.groupConcentration->setVisible(false);

      ok = m_selectedImage.IsNotNull() && CheckSettings();
  }

  m_Controls.spinBox_baselineStartTimeStep->setEnabled(m_Controls.radioButtonTurboFlash->isChecked() || m_Controls.radioButton_absoluteEnhancement->isChecked() || m_Controls.radioButton_relativeEnchancement->isChecked() || m_Controls.radioButtonUsingT1viaVFA->isChecked());
  m_Controls.spinBox_baselineEndTimeStep->setEnabled(m_Controls.radioButton_absoluteEnhancement->isChecked() || m_Controls.radioButton_relativeEnchancement->isChecked() || m_Controls.radioButtonUsingT1viaVFA->isChecked() || m_Controls.radioButtonTurboFlash->isChecked());

  m_Controls.btnConvertToConcentration->setEnabled(ok);
}




bool ConcentrationCurveConverterView::CheckSettings() const
{
  bool ok = true;

  if (m_Controls.radioButton_T1->isChecked())
  {
    if (this->m_Controls.radioButtonTurboFlash->isChecked())
    {
      ok = ok && (m_Controls.recoverytime->value() > 0);
      ok = ok && (m_Controls.relaxationtime->value() > 0);
      ok = ok && (m_Controls.relaxivity->value() > 0);
      ok = ok && (m_Controls.AifRecoverytime->value() > 0);
      ok = ok && CheckBaselineSelectionSettings();
    }
    else if (this->m_Controls.radioButton_absoluteEnhancement->isChecked()
      || this->m_Controls.radioButton_relativeEnchancement->isChecked())
    {
      ok = ok && (m_Controls.factorSpinBox->value() > 0);
      ok = ok && CheckBaselineSelectionSettings();
    }
    else if (this->m_Controls.radioButtonUsingT1viaVFA->isChecked())
    {
      ok = ok && (m_Controls.FlipangleSpinBox->value() > 0);
      ok = ok && (m_Controls.TRSpinBox->value() > 0);
      ok = ok && (m_Controls.RelaxivitySpinBox->value() > 0);
      ok = ok && (m_Controls.PDWImageNodeSelector->GetSelectedNode().IsNotNull());
      ok = ok && CheckBaselineSelectionSettings();
    }
    else
    {
      ok = false;
    }
  }
  else if (this->m_Controls.radioButton_T2->isChecked())
  {
    ok = ok && m_Controls.T2EchoTimeSpinBox->value() > 0;
    ok = ok && m_Controls.T2FactorSpinBox->value() > 0;
  }
  else
  {
    ok = false;
  }
  return ok;
}

bool ConcentrationCurveConverterView::CheckBaselineSelectionSettings() const
{
  return m_Controls.spinBox_baselineStartTimeStep->value() <= m_Controls.spinBox_baselineEndTimeStep->value();
}

void ConcentrationCurveConverterView::OnConvertToConcentrationButtonClicked()
{

    mitk::Image::Pointer concentrationImage;
    mitk::DataNode::Pointer concentrationNode;

    if(m_Controls.radioButton_T1->isChecked())
    {
        if(m_Controls.radioButton4D->isChecked())
        {
            concentrationImage = this->Convert4DConcentrationImage(this->m_selectedImage);
        }
        else if(m_Controls.radioButton3D->isChecked())
        {
            concentrationImage = Convert3DConcentrationImage(this->m_selectedImage, this->m_selectedBaselineImage);
        }
    }
    else if(m_Controls.radioButton_T2->isChecked())
    {
        concentrationImage = this->ConvertT2ConcentrationImgage(this->m_selectedImage);
    }
    std::string nameOfResultImage = m_selectedNode->GetName();
    nameOfResultImage.append("_Concentration");

    concentrationNode = AddConcentrationImage(concentrationImage,nameOfResultImage);

}


mitk::Image::Pointer ConcentrationCurveConverterView::Convert3DConcentrationImage(mitk::Image::Pointer inputImage,mitk::Image::Pointer baselineImage)
    {
    typedef itk::Image<double, 3> InputImageType;

    InputImageType::Pointer itkInputImage = InputImageType::New();
    InputImageType::Pointer itkBaselineImage = InputImageType::New();

    mitk::CastToItkImage(inputImage, itkInputImage );
    mitk::CastToItkImage(baselineImage, itkBaselineImage );


    mitk::Image::Pointer outputImage;

    if(this->m_Controls.radioButtonTurboFlash->isChecked())
    {
        typedef mitk::ConvertToConcentrationTurboFlashFunctor <double, double, double> ConversionFunctorTurboFlashType;
        typedef itk::BinaryFunctorImageFilter<InputImageType,InputImageType, ConvertedImageType, ConversionFunctorTurboFlashType> FilterTurboFlashType;

        ConversionFunctorTurboFlashType ConversionTurboFlashFunctor;
        ConversionTurboFlashFunctor.initialize(m_Controls.relaxationtime->value(), m_Controls.relaxivity->value(), m_Controls.recoverytime->value());

        FilterTurboFlashType::Pointer ConversionTurboFlashFilter = FilterTurboFlashType::New();

        ConversionTurboFlashFilter->SetFunctor(ConversionTurboFlashFunctor);
        ConversionTurboFlashFilter->SetInput1(itkInputImage);
        ConversionTurboFlashFilter->SetInput2(itkBaselineImage);

        ConversionTurboFlashFilter->Update();
        outputImage = mitk::ImportItkImage(ConversionTurboFlashFilter->GetOutput())->Clone();


    }
    else if(this->m_Controls.radioButton_absoluteEnhancement->isChecked())
    {
        typedef mitk::ConvertToConcentrationAbsoluteFunctor <double, double, double> ConversionFunctorAbsoluteType;
        typedef itk::BinaryFunctorImageFilter<InputImageType,InputImageType, ConvertedImageType, ConversionFunctorAbsoluteType> FilterAbsoluteType;

        ConversionFunctorAbsoluteType ConversionAbsoluteFunctor;
        ConversionAbsoluteFunctor.initialize(m_Controls.factorSpinBox->value());

        FilterAbsoluteType::Pointer ConversionAbsoluteFilter = FilterAbsoluteType::New();

        ConversionAbsoluteFilter->SetFunctor(ConversionAbsoluteFunctor);
        ConversionAbsoluteFilter->SetInput1(itkInputImage);
        ConversionAbsoluteFilter->SetInput2(itkBaselineImage);

        ConversionAbsoluteFilter->Update();

        outputImage = mitk::ImportItkImage(ConversionAbsoluteFilter->GetOutput())->Clone();
    }

    else if(m_Controls.radioButton_relativeEnchancement->isChecked())
    {
        typedef mitk::ConvertToConcentrationRelativeFunctor <double, double, double> ConversionFunctorRelativeType;
        typedef itk::BinaryFunctorImageFilter<InputImageType,InputImageType, ConvertedImageType, ConversionFunctorRelativeType> FilterRelativeType;

        ConversionFunctorRelativeType ConversionRelativeFunctor;
        ConversionRelativeFunctor.initialize(m_Controls.factorSpinBox->value());

        FilterRelativeType::Pointer ConversionRelativeFilter = FilterRelativeType::New();

        ConversionRelativeFilter->SetFunctor(ConversionRelativeFunctor);
        ConversionRelativeFilter->SetInput1(itkInputImage);
        ConversionRelativeFilter->SetInput2(itkBaselineImage);

        ConversionRelativeFilter->Update();

        outputImage = mitk::ImportItkImage(ConversionRelativeFilter->GetOutput())->Clone();
    }



    return outputImage;
}


mitk::DataNode::Pointer ConcentrationCurveConverterView::AddConcentrationImage(mitk::Image* image, std::string nodeName) const
{
  if (!image)
  {
    mitkThrow() << "Cannot generate concentration node. Passed image is null. parameter name: ";
  }

  mitk::DataNode::Pointer result = mitk::DataNode::New();

  result->SetData(image);

  result->SetName(nodeName);

  result->SetVisibility(true);


  this->GetDataStorage()->Add(result, m_selectedNode);

  return result;
};


mitk::Image::Pointer ConcentrationCurveConverterView::Convert4DConcentrationImage(mitk::Image::Pointer inputImage)
{
  //Compute Concentration image
  mitk::ConcentrationCurveGenerator::Pointer concentrationGen =
    mitk::ConcentrationCurveGenerator::New();
  concentrationGen->SetDynamicImage(inputImage);

  concentrationGen->SetisTurboFlashSequence(m_Controls.radioButtonTurboFlash->isChecked());
  concentrationGen->SetAbsoluteSignalEnhancement(m_Controls.radioButton_absoluteEnhancement->isChecked());
  concentrationGen->SetRelativeSignalEnhancement(m_Controls.radioButton_relativeEnchancement->isChecked());
  concentrationGen->SetUsingT1Map(m_Controls.radioButtonUsingT1viaVFA->isChecked());

  concentrationGen->SetisT2weightedImage(false);

  if (m_Controls.radioButtonTurboFlash->isChecked())
  {
    concentrationGen->SetRecoveryTime(m_Controls.recoverytime->value());
    concentrationGen->SetRelaxationTime(m_Controls.relaxationtime->value());
    concentrationGen->SetRelaxivity(m_Controls.relaxivity->value());
    concentrationGen->SetBaselineStartTimeStep(m_Controls.spinBox_baselineStartTimeStep->value());
    concentrationGen->SetBaselineEndTimeStep(m_Controls.spinBox_baselineEndTimeStep->value());
  }
  else if (this->m_Controls.radioButtonUsingT1viaVFA->isChecked())
  {
    concentrationGen->SetRecoveryTime(m_Controls.TRSpinBox->value());
    concentrationGen->SetRelaxivity(m_Controls.RelaxivitySpinBox->value());
    concentrationGen->SetT10Image(dynamic_cast<mitk::Image*>(m_Controls.PDWImageNodeSelector->GetSelectedNode()->GetData()));
    concentrationGen->SetBaselineStartTimeStep(m_Controls.spinBox_baselineStartTimeStep->value());
    concentrationGen->SetBaselineEndTimeStep(m_Controls.spinBox_baselineEndTimeStep->value());
    //Convert Flipangle from degree to radiant
      double alpha = m_Controls.FlipangleSpinBox->value()/360*2* boost::math::constants::pi<double>();
      concentrationGen->SetFlipAngle(alpha);
  }

  else
  {
    concentrationGen->SetFactor(m_Controls.factorSpinBox->value());
  }

  mitk::Image::Pointer concentrationImage = concentrationGen->GetConvertedImage();

  return concentrationImage;
}


mitk::Image::Pointer ConcentrationCurveConverterView::ConvertT2ConcentrationImgage(mitk::Image::Pointer inputImage)
{
    //Compute Concentration image
    mitk::ConcentrationCurveGenerator::Pointer concentrationGen =
      mitk::ConcentrationCurveGenerator::New();
    concentrationGen->SetDynamicImage(inputImage);

    concentrationGen->SetisTurboFlashSequence(false);
    concentrationGen->SetAbsoluteSignalEnhancement(false);
    concentrationGen->SetRelativeSignalEnhancement(false);

    concentrationGen->SetisT2weightedImage(true);


    concentrationGen->SetT2Factor(m_Controls.T2FactorSpinBox->value());
    concentrationGen->SetT2EchoTime(m_Controls.T2EchoTimeSpinBox->value());


    mitk::Image::Pointer concentrationImage = concentrationGen->GetConvertedImage();

    return concentrationImage;
}

void ConcentrationCurveConverterView::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,const QList<mitk::DataNode::Pointer>& selectedNodes ){

    m_selectedNode = nullptr;
    m_selectedImage = nullptr;
    m_selectedBaselineNode = nullptr;
    m_selectedBaselineImage = nullptr;

    m_Controls.timeserieslabel->setText("No (valid) series selected.");
    m_Controls.BaselineImageLabel->setText("No (valid) baseline image selected.");
    m_Controls.ImageLabel->setText("No (valid) image selected.");
    m_Controls.T2_SeriesLabel->setText("No (valid) series selected.");

    m_Controls.btnConvertToConcentration->setEnabled(false);

    QList<mitk::DataNode::Pointer> nodes = selectedNodes;
    mitk::NodePredicateDataType::Pointer imagePredicate = mitk::NodePredicateDataType::New("Image");

    if(m_Controls.radioButton_T1->isChecked())
    {
        if (nodes.size() > 0 && imagePredicate->CheckNode(nodes.front()))
        {
            this->m_selectedNode = nodes.front();
            this->m_selectedImage = dynamic_cast<mitk::Image*>(this->m_selectedNode->GetData());
            m_Controls.timeserieslabel->setText((this->m_selectedNode->GetName()).c_str());
            nodes.pop_front();
        }

        if (nodes.size() > 0 && imagePredicate->CheckNode(nodes.front()))
        {

            this->m_selectedBaselineNode = nodes.front();
            this->m_selectedBaselineImage = dynamic_cast<mitk::Image*>(this->m_selectedBaselineNode->GetData());
            this->m_Controls.BaselineImageLabel->setText((this->m_selectedBaselineNode->GetName()).c_str());
        }
    }
    else if(m_Controls.radioButton_T2->isChecked())
    {
        if (nodes.size() > 0 && imagePredicate->CheckNode(nodes.front()))
        {
            this->m_selectedNode = nodes.front();
            this->m_selectedImage = dynamic_cast<mitk::Image*>(this->m_selectedNode->GetData());
            m_Controls.T2_SeriesLabel->setText((this->m_selectedNode->GetName()).c_str());
            nodes.pop_front();
        }
    }

    if (this->m_selectedImage.IsNotNull())
    {
      m_Controls.spinBox_baselineStartTimeStep->setMaximum((this->m_selectedImage->GetDimension(3)) - 1);
      m_Controls.spinBox_baselineEndTimeStep->setMaximum((this->m_selectedImage->GetDimension(3)) - 1);
    }

    m_Controls.btnConvertToConcentration->setEnabled(m_selectedImage.IsNotNull() && CheckSettings());



}



ConcentrationCurveConverterView::ConcentrationCurveConverterView()
{
}
