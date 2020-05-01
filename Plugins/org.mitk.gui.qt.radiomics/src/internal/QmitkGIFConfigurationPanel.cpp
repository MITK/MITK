/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkGIFConfigurationPanel.h"


// QT
#include <qwidget.h>
#include <qboxlayout.h>
#include <qformlayout.h>

#include <qpushbutton.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qlineedit.h>
#include <qspinbox.h>

#include <mitkLog.h>


mitkUI::GIFConfigurationPanel::GIFConfigurationPanel(QWidget * parent, mitk::AbstractGlobalImageFeature::Pointer calculator) :
  QWidget(parent),
  m_FeatureCalculator(calculator)
{
  QVBoxLayout * lay = new QVBoxLayout;

  QHBoxLayout * checkboxLayout = new QHBoxLayout;
  m_CheckCalculateFeature = new QCheckBox;
  m_CheckCalculateFeature->setText(m_FeatureCalculator->GetFeatureClassName().c_str());
  m_ButtonShowAdditionalConfiguration = new QPushButton;
  m_ButtonShowAdditionalConfiguration->setText("+");
  m_ButtonShowAdditionalConfiguration->setMaximumWidth(30);
  m_ButtonShowAdditionalConfiguration->setCheckable(true);

  checkboxLayout->addWidget(m_CheckCalculateFeature);
  checkboxLayout->addWidget(m_ButtonShowAdditionalConfiguration);
  lay->addItem(checkboxLayout);

  QFormLayout * argumentLayout = new QFormLayout;
  QLabel * label1 = new QLabel("Use Argument");
  QLabel * label2 = new QLabel("Argument Value");
  argumentLayout->addRow(label1, label2);

  mitkCommandLineParser parser;
  m_FeatureCalculator->AddArguments(parser);
  std::vector < std::map<std::string, us::Any> > argumentList = parser.getArgumentList();
  for (auto argument : argumentList)
  {
    QString longarg(argument["longarg"].ToString().c_str());
    if (longarg.contains("::"))
    {

      QCheckBox * argumentBox = new QCheckBox;
      QWidget * secondWidget = nullptr;
      argumentBox->setText(longarg);

      mitkCommandLineParser::Type type = us::any_cast<mitkCommandLineParser::Type>(argument["valuetype"]);
      switch (type)
      {
      case mitkCommandLineParser::Bool:
        secondWidget = new QCheckBox;
        break;
      case mitkCommandLineParser::String:
      {
        QLineEdit* lineEdit = new QLineEdit;
        secondWidget = lineEdit;
        break;
      }
      case mitkCommandLineParser::Int:
      {
        QSpinBox* spinBox = new QSpinBox();
        spinBox->setMinimum(std::numeric_limits<int>::min());
        spinBox->setMaximum(std::numeric_limits<int>::max());
        secondWidget = spinBox;
        break;
      }
      case mitkCommandLineParser::Float:
      {
        QDoubleSpinBox* spindBox = new QDoubleSpinBox;
        spindBox->setMinimum(std::numeric_limits<double>::lowest());
        spindBox->setMaximum(std::numeric_limits<double>::max());
        secondWidget = spindBox;
        break;
      }
      default:
        secondWidget = new QLabel("unkonw type");
      }

      argumentLayout->addRow(argumentBox, secondWidget);
    }
  }
  if (argumentList.size() < 2)
  {
    m_ButtonShowAdditionalConfiguration->setVisible(false);
  }
  m_GroupBoxArguments = new QGroupBox;
  m_GroupBoxArguments->setTitle("");
  m_GroupBoxArguments->setLayout(argumentLayout);
  m_GroupBoxArguments->setVisible(false);
  lay->addWidget(m_GroupBoxArguments);
  //lay->addItem(argumentLayout);


  // Buttons see https://joekuan.wordpress.com/2015/09/23/list-of-qt-icons/
  // Resolution https://itk.org/Doxygen/html/classitk_1_1RecursiveMultiResolutionPyramidImageFilter.html
  // Wavelet https://code.google.com/archive/p/nwave/source/default/source

  this->setLayout(lay);

  connect((QObject*)(m_ButtonShowAdditionalConfiguration), SIGNAL(clicked(bool)), this, SLOT(OnSButtonShowAdditionalConfigurationPressed(bool)));
}

void
mitkUI::GIFConfigurationPanel::OnSButtonShowAdditionalConfigurationPressed(bool /*status*/)
{
  if (m_ButtonShowAdditionalConfiguration->isChecked())
  {
    m_ButtonShowAdditionalConfiguration->setText("-");
    m_GroupBoxArguments->setVisible(true);
  }
  else
  {
    m_ButtonShowAdditionalConfiguration->setText("+");
    m_GroupBoxArguments->setVisible(false);
  }
}

void
mitkUI::GIFConfigurationPanel::CalculateFeaturesUsingParameters(const mitk::Image::Pointer & feature, const mitk::Image::Pointer &mask, std::map < std::string, us::Any> parameter, mitk::AbstractGlobalImageFeature::FeatureListType &featureList)
{
  parameter[m_FeatureCalculator->GetLongName()] = us::Any(true);

  if (m_CheckCalculateFeature->isChecked())
  {
    for (int i = 0; i < m_GroupBoxArguments->layout()->count(); i+=2)
    {
      QCheckBox * argumentBox = dynamic_cast<QCheckBox* >(m_GroupBoxArguments->layout()->itemAt(i)->widget());
      if (argumentBox != nullptr)
      {
        if (argumentBox->isChecked() == false)
        {
          continue;
        }
        std::string argumentName = argumentBox->text().toStdString();


        QCheckBox * paramBool = dynamic_cast<QCheckBox*>(m_GroupBoxArguments->layout()->itemAt(i + 1)->widget());
        QLineEdit * paramText = dynamic_cast<QLineEdit*>(m_GroupBoxArguments->layout()->itemAt(i + 1)->widget());
        QSpinBox * paramInt = dynamic_cast<QSpinBox*>(m_GroupBoxArguments->layout()->itemAt(i + 1)->widget());
        QDoubleSpinBox * paramFloat = dynamic_cast<QDoubleSpinBox*>(m_GroupBoxArguments->layout()->itemAt(i + 1)->widget());

        if (paramBool == nullptr && paramText == nullptr && paramInt == nullptr && paramFloat == nullptr)
        {
          continue;
        }

        us::Any value;
        if (paramBool != nullptr)
        {
          value = us::Any(paramBool->isChecked());
        }
        if (paramText != nullptr)
        {
          value = us::Any(paramText->text().toStdString());
        }
        if (paramInt != nullptr)
        {
          value = us::Any(paramInt->value());
        }
        if (paramFloat != nullptr)
        {
          value = us::Any(float(paramFloat->value()));
        }

        parameter[argumentName] = value;
        MITK_INFO << argumentName << " : " << value.ToString();
      }
    }
    auto tmpPointer = m_FeatureCalculator->Clone();
    mitk::AbstractGlobalImageFeature::Pointer tmpCalc = dynamic_cast<mitk::AbstractGlobalImageFeature*>(tmpPointer.GetPointer());
    for (auto item : parameter)
    {
      MITK_INFO << item.first << " : " << item.second.ToString();
    }
    tmpCalc->SetParameters(parameter);
    bool calculateSliceWise = false;
    int slice = 0;
    if (parameter.count("slice-wise"))
    {
      slice = us::any_cast<int>(parameter["slice-wise"]);
      calculateSliceWise = true;
    }
    if (calculateSliceWise)
    {
      tmpCalc->CalculateAndAppendFeaturesSliceWise(feature, mask, slice, featureList);
    }
    else
    {
      tmpCalc->CalculateAndAppendFeatures(feature, mask, mask, featureList);
    }
  }
}

