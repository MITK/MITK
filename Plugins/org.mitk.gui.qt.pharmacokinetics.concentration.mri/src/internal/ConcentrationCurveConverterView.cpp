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

#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateDimension.h>
#include "mitkNodePredicateFunction.h"
#include <mitkModelFitResultRelationRule.h>

#include <mitkModelFitConstants.h>
// Includes for image casting between ITK and MITK
#include "mitkImageTimeSelector.h"
#include "mitkImageCast.h"
#include "mitkITKImageImport.h"
#include <mitkImagePixelReadAccessor.h>

#include <iostream>

const std::string ConcentrationCurveConverterView::VIEW_ID = "org.mitk.views.pharmacokinetics.concentration.mri";

void ConcentrationCurveConverterView::SetFocus()
{
    m_Controls.btnConvertToConcentration->setFocus();
}

void ConcentrationCurveConverterView::CreateQtPartControl(QWidget* parent)
{
	m_Controls.setupUi(parent);
    m_Controls.btnConvertToConcentration->setEnabled(false);

    connect(m_Controls.btnConvertToConcentration, SIGNAL(clicked()), this, SLOT(OnConvertToConcentrationButtonClicked()));

    m_Controls.timeSeriesNodeSelector->SetNodePredicate(this->m_isValidTimeSeriesImagePredicate);
    m_Controls.timeSeriesNodeSelector->SetDataStorage(this->GetDataStorage());
    m_Controls.timeSeriesNodeSelector->SetSelectionIsOptional(false);
    m_Controls.timeSeriesNodeSelector->SetInvalidInfo("Please select time series.");

    m_Controls.image3DNodeSelector->SetNodePredicate(this->m_isValidPDWImagePredicate);
    m_Controls.image3DNodeSelector->SetDataStorage(this->GetDataStorage());
    m_Controls.image3DNodeSelector->SetSelectionIsOptional(false);
    m_Controls.image3DNodeSelector->SetInvalidInfo("Please select 3D image.");

    m_Controls.baselineImageNodeSelector->SetNodePredicate(this->m_isValidPDWImagePredicate);
    m_Controls.baselineImageNodeSelector->SetDataStorage(this->GetDataStorage());
    m_Controls.baselineImageNodeSelector->SetSelectionIsOptional(false);
    m_Controls.baselineImageNodeSelector->SetInvalidInfo("Please select baseline image.");

    m_Controls.t2TimeSeriesNodeSelector->SetNodePredicate(this->m_isValidTimeSeriesImagePredicate);
    m_Controls.t2TimeSeriesNodeSelector->SetDataStorage(this->GetDataStorage());
    m_Controls.t2TimeSeriesNodeSelector->SetSelectionIsOptional(false);
    m_Controls.t2TimeSeriesNodeSelector->SetInvalidInfo("Please select time series.");

    m_Controls.PDWImageNodeSelector->SetNodePredicate(m_isValidPDWImagePredicate);
    m_Controls.PDWImageNodeSelector->SetDataStorage(this->GetDataStorage());
    m_Controls.PDWImageNodeSelector->SetInvalidInfo("Please select PDW Image.");
    m_Controls.PDWImageNodeSelector->setEnabled(false);

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

    m_Controls.spinBox_baselineStartTimeStepT2->setValue(0);
    m_Controls.spinBox_baselineEndTimeStepT2->setValue(0);

    m_Controls.spinBox_baselineEndTimeStepT2->setMinimum(0);
    m_Controls.spinBox_baselineStartTimeStepT2->setMinimum(0);

    connect(m_Controls.radioButtonTurboFlash, SIGNAL(toggled(bool)), m_Controls.groupBoxTurboFlash, SLOT(setVisible(bool)));
    connect(m_Controls.radioButtonTurboFlash, SIGNAL(toggled(bool)), this, SLOT(OnSettingChanged()));
    connect(m_Controls.relaxationtime, SIGNAL(valueChanged(double)), this, SLOT(OnSettingChanged()));
    connect(m_Controls.recoverytime, SIGNAL(valueChanged(double)), this, SLOT(OnSettingChanged()));
    connect(m_Controls.relaxivity, SIGNAL(valueChanged(double)), this, SLOT(OnSettingChanged()));

    connect(m_Controls.timeSeriesNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, &ConcentrationCurveConverterView::OnNodeSelectionChanged);
    connect(m_Controls.image3DNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, &ConcentrationCurveConverterView::OnNodeSelectionChanged);
    connect(m_Controls.baselineImageNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, &ConcentrationCurveConverterView::OnNodeSelectionChanged);
    connect(m_Controls.t2TimeSeriesNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, &ConcentrationCurveConverterView::OnNodeSelectionChanged);
    connect(m_Controls.PDWImageNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, &ConcentrationCurveConverterView::OnSettingChanged);

    connect(m_Controls.radioButton_absoluteEnhancement, SIGNAL(toggled(bool)), this, SLOT(OnSettingChanged()));
    connect(m_Controls.radioButton_relativeEnchancement, SIGNAL(toggled(bool)), this, SLOT(OnSettingChanged()));
    connect(m_Controls.radioButton_absoluteEnhancement, SIGNAL(toggled(bool)), m_Controls.groupBoxEnhancement, SLOT(setVisible(bool)));
    connect(m_Controls.radioButton_relativeEnchancement, SIGNAL(toggled(bool)), m_Controls.groupBoxEnhancement, SLOT(setVisible(bool)));

    connect(m_Controls.factorSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnSettingChanged()));
    connect(m_Controls.spinBox_baselineStartTimeStep, SIGNAL(valueChanged(int)), this, SLOT(OnSettingChanged()));
    connect(m_Controls.spinBox_baselineEndTimeStep, SIGNAL(valueChanged(int)), this, SLOT(OnSettingChanged()));
    connect(m_Controls.spinBox_baselineStartTimeStepT2, SIGNAL(valueChanged(int)), this, SLOT(OnSettingChanged()));
    connect(m_Controls.spinBox_baselineEndTimeStepT2, SIGNAL(valueChanged(int)), this, SLOT(OnSettingChanged()));

    connect(m_Controls.radioButtonUsingT1viaVFA, SIGNAL(toggled(bool)), m_Controls.groupBox_T1MapviaVFA, SLOT(setVisible(bool)));
    connect(m_Controls.radioButtonUsingT1viaVFA, SIGNAL(toggled(bool)), this, SLOT(OnSettingChanged()));
    connect(m_Controls.FlipangleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnSettingChanged()));
    connect(m_Controls.RelaxivitySpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnSettingChanged()));
    connect(m_Controls.TRSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnSettingChanged()));

    connect(m_Controls.T2EchoTimeSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnSettingChanged()));
    connect(m_Controls.T2FactorSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnSettingChanged()));

    connect(m_Controls.radioButtonUsingT1viaVFA, SIGNAL(toggled(bool)), m_Controls.PDWImageNodeSelector, SLOT(setEnabled(bool)));


}


void ConcentrationCurveConverterView::OnSettingChanged()
{
  bool ok = false;
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
    ok = ok && CheckBaselineSelectionSettings();
  }
  else
  {
    ok = false;
  }
  return ok;
}

bool ConcentrationCurveConverterView::CheckBaselineSelectionSettings() const
{
  if (this->m_Controls.radioButton_T1->isChecked())
  {
    return m_Controls.spinBox_baselineStartTimeStep->value() <= m_Controls.spinBox_baselineEndTimeStep->value();
  }
  else if (this->m_Controls.radioButton_T2->isChecked())
  {
    return m_Controls.spinBox_baselineStartTimeStepT2->value() <= m_Controls.spinBox_baselineEndTimeStepT2->value();
  }
  else
  {
    return 0;
  }
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
  concentrationGen->SetBaselineStartTimeStep(m_Controls.spinBox_baselineStartTimeStepT2->value());
  concentrationGen->SetBaselineEndTimeStep(m_Controls.spinBox_baselineEndTimeStepT2->value());

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
    concentrationGen->SetBaselineStartTimeStep(m_Controls.spinBox_baselineStartTimeStep->value());
    concentrationGen->SetBaselineEndTimeStep(m_Controls.spinBox_baselineEndTimeStep->value());
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

    concentrationGen->SetBaselineStartTimeStep(m_Controls.spinBox_baselineStartTimeStep->value());
    concentrationGen->SetBaselineEndTimeStep(m_Controls.spinBox_baselineEndTimeStep->value());

    mitk::Image::Pointer concentrationImage = concentrationGen->GetConvertedImage();

    return concentrationImage;
}

void ConcentrationCurveConverterView::OnNodeSelectionChanged(QList<mitk::DataNode::Pointer>/*nodes*/)
{
    m_selectedNode = nullptr;
    m_selectedImage = nullptr;
    m_selectedBaselineNode = nullptr;
    m_selectedBaselineImage = nullptr;

    if (m_Controls.radioButton_T1->isChecked())
    {
      if (m_Controls.radioButton4D->isChecked())
      {
        if (m_Controls.timeSeriesNodeSelector->GetSelectedNode().IsNotNull())
        {
          this->m_selectedNode = m_Controls.timeSeriesNodeSelector->GetSelectedNode();
          m_selectedImage = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());

        }
        else
        {
          this->m_selectedNode = nullptr;
          this->m_selectedImage = nullptr;
        }
      }
      else if (m_Controls.radioButton3D->isChecked())
      {
        if (m_Controls.image3DNodeSelector->GetSelectedNode().IsNotNull() && m_Controls.baselineImageNodeSelector->GetSelectedNode().IsNotNull())
        {
          this->m_selectedNode = m_Controls.image3DNodeSelector->GetSelectedNode();
          m_selectedImage = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
          this->m_selectedBaselineNode = m_Controls.baselineImageNodeSelector->GetSelectedNode();
          m_selectedBaselineImage = dynamic_cast<mitk::Image*>(m_selectedBaselineNode->GetData());
        }
        else
        {
          this->m_selectedNode = nullptr;
          this->m_selectedImage = nullptr;
          m_selectedBaselineNode = nullptr;
          m_selectedBaselineImage = nullptr;
        }
      }

      if (this->m_selectedImage.IsNotNull())
      {
        m_Controls.spinBox_baselineStartTimeStep->setMaximum((this->m_selectedImage->GetDimension(3)) - 1);
        m_Controls.spinBox_baselineEndTimeStep->setMaximum((this->m_selectedImage->GetDimension(3)) - 1);
      }
    }
    if (m_Controls.radioButton_T2->isChecked())
    {
      if (m_Controls.t2TimeSeriesNodeSelector->GetSelectedNode().IsNotNull())
      {
        this->m_selectedNode = m_Controls.t2TimeSeriesNodeSelector->GetSelectedNode();
        m_selectedImage = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
      }
      else
      {
        this->m_selectedNode = nullptr;
        this->m_selectedImage = nullptr;
      }
      if (this->m_selectedImage.IsNotNull())
      {
        m_Controls.spinBox_baselineStartTimeStepT2->setMaximum((this->m_selectedImage->GetDimension(3)) - 1);
        m_Controls.spinBox_baselineEndTimeStepT2->setMaximum((this->m_selectedImage->GetDimension(3)) - 1);
      }
    }

    m_Controls.btnConvertToConcentration->setEnabled(m_selectedImage.IsNotNull() && CheckSettings());



}



ConcentrationCurveConverterView::ConcentrationCurveConverterView()
{
  mitk::NodePredicateDataType::Pointer isLabelSet = mitk::NodePredicateDataType::New("LabelSetImage");
  mitk::NodePredicateDataType::Pointer isImage = mitk::NodePredicateDataType::New("Image");
  mitk::NodePredicateProperty::Pointer isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
  mitk::NodePredicateAnd::Pointer isLegacyMask = mitk::NodePredicateAnd::New(isImage, isBinary);
  mitk::NodePredicateDimension::Pointer is3D = mitk::NodePredicateDimension::New(3);
  mitk::NodePredicateOr::Pointer isMask = mitk::NodePredicateOr::New(isLegacyMask, isLabelSet);
  mitk::NodePredicateAnd::Pointer isNoMask = mitk::NodePredicateAnd::New(isImage, mitk::NodePredicateNot::New(isMask));
  mitk::NodePredicateAnd::Pointer is3DImage = mitk::NodePredicateAnd::New(isImage, is3D, isNoMask);

  this->m_IsMaskPredicate = mitk::NodePredicateAnd::New(isMask, mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))).GetPointer();

  this->m_IsNoMaskImagePredicate = mitk::NodePredicateAnd::New(isNoMask, mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))).GetPointer();

  auto isDynamicData = mitk::NodePredicateFunction::New([](const mitk::DataNode* node)
  {
    return  (node && node->GetData() && node->GetData()->GetTimeSteps() > 1);
  });

  auto modelFitResultRelationRule = mitk::ModelFitResultRelationRule::New();
  auto isNoModelFitNodePredicate = mitk::NodePredicateNot::New(modelFitResultRelationRule->GetConnectedSourcesDetector());

  this->m_isValidPDWImagePredicate = mitk::NodePredicateAnd::New(is3DImage, isNoModelFitNodePredicate);
  this->m_isValidTimeSeriesImagePredicate = mitk::NodePredicateAnd::New(isDynamicData, isImage, isNoMask);
}
