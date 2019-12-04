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
    m_Controls.factorSpinBox->setEnabled(false);
    m_Controls.groupBox_ConcentrationParameters->hide();

    connect(m_Controls.radioButton_T1, SIGNAL(toggled(bool)),this, SLOT(OnSettingChanged()));
    connect(m_Controls.radioButton_T2, SIGNAL(toggled(bool)),this, SLOT(OnSettingChanged()));

    connect(m_Controls.radioButton3D, SIGNAL(toggled(bool)),this, SLOT(OnSettingChanged()));
    connect(m_Controls.radioButton4D, SIGNAL(toggled(bool)),this, SLOT(OnSettingChanged()));

    connect(m_Controls.radioButtonTurboFlash, SIGNAL(toggled(bool)), m_Controls.groupBoxTurboFlash,
            SLOT(setVisible(bool)));

    connect(m_Controls.radioButtonTurboFlash, SIGNAL(toggled(bool)), this,
            SLOT(OnSettingChanged()));
    connect(m_Controls.relaxationTime, SIGNAL(valueChanged(double)), this,
            SLOT(OnSettingChanged()));
    connect(m_Controls.recoveryTime, SIGNAL(valueChanged(double)), this, SLOT(OnSettingChanged()));
    connect(m_Controls.relaxivity, SIGNAL(valueChanged(double)), this, SLOT(OnSettingChanged()));

    connect(m_Controls.radioButton_absoluteEnhancement, SIGNAL(toggled(bool)), this,
            SLOT(OnSettingChanged()));
    connect(m_Controls.radioButton_relativeEnchancement, SIGNAL(toggled(bool)), this,
            SLOT(OnSettingChanged()));
    connect(m_Controls.radioButton_absoluteEnhancement, SIGNAL(toggled(bool)), m_Controls.factorSpinBox,
            SLOT(setEnabled(bool)));
    connect(m_Controls.radioButton_relativeEnchancement, SIGNAL(toggled(bool)),
            m_Controls.factorSpinBox,
            SLOT(setEnabled(bool)));
    connect(m_Controls.factorSpinBox, SIGNAL(valueChanged(double)), this,
            SLOT(OnSettingChanged()));

}


void ConcentrationCurveConverterView::OnSettingChanged()
{
  bool ok = true;
  m_Controls.groupBox_T1->setVisible(m_Controls.radioButton_T1->isChecked());
  m_Controls.groupBox_T2->setVisible(m_Controls.radioButton_T2->isChecked());

  if(m_Controls.radioButton_T1->isChecked())
  {
      m_Controls.groupBox_ConcentrationParameters->setVisible(true);
      m_Controls.groupBox3D->setVisible(m_Controls.radioButton3D->isChecked());
      m_Controls.groupBox4D->setVisible(m_Controls.radioButton4D->isChecked());

      if(m_Controls.radioButton4D->isChecked())
      {
          ok = m_selectedImage.IsNotNull() && CheckSettings();
      }
      else if(m_Controls.radioButton3D->isChecked())
      {
          ok = m_selectedImage.IsNotNull() && m_selectedBaselineImage.IsNotNull() && CheckSettings();

      }
  }
  else if (m_Controls.radioButton_T2->isChecked())
  {
      m_Controls.groupBox_ConcentrationParameters->setVisible(false);

      ok = m_selectedImage.IsNotNull() && CheckSettings();

  }

  m_Controls.btnConvertToConcentration->setEnabled(ok);
}


bool ConcentrationCurveConverterView::CheckSettings() const
{
  bool ok = true;

  if(m_Controls.radioButton_T1->isChecked())
  {
      if (this->m_Controls.radioButtonTurboFlash->isChecked())
      {
          ok = ok && (m_Controls.recoveryTime->value() > 0);
          ok = ok && (m_Controls.relaxationTime->value() > 0);
          ok = ok && (m_Controls.relaxivity->value() > 0);

      }
      else if (this->m_Controls.radioButton_absoluteEnhancement->isChecked()
               || this->m_Controls.radioButton_relativeEnchancement->isChecked())
      {
          ok = ok && (m_Controls.factorSpinBox->value() > 0);
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
      ok=false;
  }


  return ok;
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
        ConversionTurboFlashFunctor.initialize(m_Controls.relaxationTime->value(), m_Controls.relaxivity->value(), m_Controls.recoveryTime->value());

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

  concentrationGen->SetisT2weightedImage(false);

  if (m_Controls.radioButtonTurboFlash->isChecked())
  {
    concentrationGen->SetRecoveryTime(m_Controls.recoveryTime->value());
    concentrationGen->SetRelaxationTime(m_Controls.relaxationTime->value());
    concentrationGen->SetRelaxivity(m_Controls.relaxivity->value());
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


    m_Controls.btnConvertToConcentration->setEnabled(m_selectedImage.IsNotNull() && CheckSettings());



}



ConcentrationCurveConverterView::ConcentrationCurveConverterView()
{
}
