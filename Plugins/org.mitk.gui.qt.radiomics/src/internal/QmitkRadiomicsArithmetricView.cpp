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

#include "QmitkRadiomicsArithmetricView.h"

// QT includes (GUI)
#include <qlabel.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qmessagebox.h>

// Berry includes (selection service)
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// MITK includes (GUI)
#include <QmitkDataStorageComboBox.h>
#include "QmitkDataNodeSelectionProvider.h"
#include "mitkDataNodeObject.h"

// MITK includes (general
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateAnd.h>
#include <mitkProperties.h>
#include <mitkArithmeticOperation.h>

// Specific GUI Includes
#include "QmitkGIFConfigurationPanel.h"

QmitkRadiomicsArithmetric::QmitkRadiomicsArithmetric()
: QmitkAbstractView(),
  m_Controls(nullptr)
{
}

QmitkRadiomicsArithmetric::~QmitkRadiomicsArithmetric()
{

}

void QmitkRadiomicsArithmetric::CreateQtPartControl(QWidget *parent)
{
  if (m_Controls == nullptr)
  {
    m_Controls = new Ui::QmitkRadiomicsArithmetricViewControls;
    m_Controls->setupUi(parent);

    QLabel * label1 = new QLabel("Image: ");
    QmitkDataStorageComboBox * cb_inputimage = new QmitkDataStorageComboBox(this->GetDataStorage(), mitk::TNodePredicateDataType<mitk::Image>::New());
    m_Controls->m_InputImageGroup->layout()->addWidget(label1);
    m_Controls->m_InputImageGroup->layout()->addWidget(cb_inputimage);

    QLabel * label2 = new QLabel("Second Image: ");
    QmitkDataStorageComboBox * cb_inputimage2 = new QmitkDataStorageComboBox(this->GetDataStorage(), mitk::TNodePredicateDataType<mitk::Image>::New());
    m_Controls->secondImageWidget->layout()->addWidget(label2);
    m_Controls->secondImageWidget->layout()->addWidget(cb_inputimage2);

    this->CreateConnections();
  }
}

void QmitkRadiomicsArithmetric::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->buttonTan), SIGNAL(clicked() ), this, SLOT(TanButton()));
    connect((QObject*)(m_Controls->buttonATan), SIGNAL(clicked()), this, SLOT(ATanButton()));
    connect((QObject*)(m_Controls->buttonCos), SIGNAL(clicked()), this, SLOT(CosButton()));
    connect((QObject*)(m_Controls->buttonACos), SIGNAL(clicked()), this, SLOT(ACosButton()));
    connect((QObject*)(m_Controls->buttonSin), SIGNAL(clicked()), this, SLOT(SinButton()));
    connect((QObject*)(m_Controls->buttonASin), SIGNAL(clicked()), this, SLOT(ASinButton()));
    connect((QObject*)(m_Controls->buttonSquare), SIGNAL(clicked()), this, SLOT(SquareButton()));
    connect((QObject*)(m_Controls->buttonSqrt), SIGNAL(clicked()), this, SLOT(SqrtButton()));
    connect((QObject*)(m_Controls->buttonAbs), SIGNAL(clicked()), this, SLOT(AbsButton()));
    connect((QObject*)(m_Controls->buttonExp), SIGNAL(clicked()), this, SLOT(ExpButton()));
    connect((QObject*)(m_Controls->buttonExpNeg), SIGNAL(clicked()), this, SLOT(ExpNegButton()));
    connect((QObject*)(m_Controls->buttonLog10), SIGNAL(clicked()), this, SLOT(Log10Button()));

    connect((QObject*)(m_Controls->buttonAddLeft), SIGNAL(clicked()), this, SLOT(AddLeftButton()));
    connect((QObject*)(m_Controls->buttonSubLeft), SIGNAL(clicked()), this, SLOT(SubLeftButton()));
    connect((QObject*)(m_Controls->buttonSubRight), SIGNAL(clicked()), this, SLOT(SubRightButton()));
    connect((QObject*)(m_Controls->buttonMulLeft), SIGNAL(clicked()), this, SLOT(MulLeftButton()));
    connect((QObject*)(m_Controls->buttonDivLeft), SIGNAL(clicked()), this, SLOT(DivLeftButton()));
    connect((QObject*)(m_Controls->buttonDivRight), SIGNAL(clicked()), this, SLOT(DivRightButton()));

    connect((QObject*)(m_Controls->buttonAdd), SIGNAL(clicked()), this, SLOT(AddButton()));
    connect((QObject*)(m_Controls->buttonSub), SIGNAL(clicked()), this, SLOT(SubButton()));
    connect((QObject*)(m_Controls->buttonMul), SIGNAL(clicked()), this, SLOT(MulButton()));
    connect((QObject*)(m_Controls->buttonDiv), SIGNAL(clicked()), this, SLOT(DivButton()));
  }
}

mitk::Image::Pointer QmitkRadiomicsArithmetric::GetFirstImage()
{
  QmitkDataStorageComboBox * cb_image = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls->m_InputImageGroup->layout()->itemAt(1)->widget());
  mitk::BaseData* baseDataRawImage = NULL;

  mitk::Image::Pointer raw_image;
  std::string nodeName;
  if ((cb_image->GetSelectedNode().IsNotNull()))
  {
    baseDataRawImage = (cb_image->GetSelectedNode()->GetData());
    nodeName = cb_image->GetSelectedNode()->GetName();
  }
  if ((baseDataRawImage != NULL))
  {
    raw_image = dynamic_cast<mitk::Image *>(baseDataRawImage);
  }
  else {
    QMessageBox msgBox;
    msgBox.setText("Please specify the images that shlould be used.");
    msgBox.exec();
    return NULL;
  }
  if (raw_image.IsNull())
  {
    QMessageBox msgBox;
    msgBox.setText("Error during processing the specified images.");
    msgBox.exec();
    return NULL;
  }

  return raw_image;
}

mitk::Image::Pointer QmitkRadiomicsArithmetric::GetSecondImage()
{
  QmitkDataStorageComboBox * cb_image = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls->secondImageWidget->layout()->itemAt(1)->widget());
  mitk::BaseData* baseDataRawImage = NULL;

  mitk::Image::Pointer raw_image;
  std::string nodeName;
  if ((cb_image->GetSelectedNode().IsNotNull()))
  {
    baseDataRawImage = (cb_image->GetSelectedNode()->GetData());
    nodeName = cb_image->GetSelectedNode()->GetName();
  }
  if ((baseDataRawImage != NULL))
  {
    raw_image = dynamic_cast<mitk::Image *>(baseDataRawImage);
  }
  else {
    QMessageBox msgBox;
    msgBox.setText("Please specify the second images that should be used.");
    msgBox.exec();
    return NULL;
  }
  if (raw_image.IsNull())
  {
    QMessageBox msgBox;
    msgBox.setText("Error during processing the specified images.");
    msgBox.exec();
    return NULL;
  }

  return raw_image;
}


void QmitkRadiomicsArithmetric::AddImageToNode(mitk::Image::Pointer image, std::string nameAddition)
{
  QmitkDataStorageComboBox * cb_image = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls->m_InputImageGroup->layout()->itemAt(1)->widget());
  std::string nodeName;
  if ((cb_image->GetSelectedNode().IsNotNull()))
  {
    nodeName = cb_image->GetSelectedNode()->GetName();
  }

  mitk::DataNode::Pointer result = mitk::DataNode::New();
  result->SetProperty("name", mitk::StringProperty::New(nodeName + nameAddition));
  result->SetData(image);
  GetDataStorage()->Add(result, cb_image->GetSelectedNode());
}


void QmitkRadiomicsArithmetric::TanButton()
{
  mitk::Image::Pointer firstImage = GetFirstImage();
  bool asDouble = m_Controls->m_ResultAsDouble->isChecked();
  if (firstImage.IsNull())
  {
    return;
  }
  auto result = mitk::ArithmeticOperation::Tan(firstImage, asDouble);
  AddImageToNode(result, "::Tan");
}

void QmitkRadiomicsArithmetric::ATanButton()
{
  mitk::Image::Pointer firstImage = GetFirstImage();
  bool asDouble = m_Controls->m_ResultAsDouble->isChecked();
  if (firstImage.IsNull())
  {
    return;
  }
  auto result = mitk::ArithmeticOperation::Tan(firstImage, asDouble);
  AddImageToNode(result, "::ATan");
}

void QmitkRadiomicsArithmetric::CosButton()
{
  mitk::Image::Pointer firstImage = GetFirstImage();
  bool asDouble = m_Controls->m_ResultAsDouble->isChecked();
  if (firstImage.IsNull())
  {
    return;
  }
  auto result = mitk::ArithmeticOperation::Cos(firstImage, asDouble);
  AddImageToNode(result, "::Cos");
}

void QmitkRadiomicsArithmetric::ACosButton()
{
  mitk::Image::Pointer firstImage = GetFirstImage();
  bool asDouble = m_Controls->m_ResultAsDouble->isChecked();
  if (firstImage.IsNull())
  {
    return;
  }
  auto result = mitk::ArithmeticOperation::Acos(firstImage, asDouble);
  AddImageToNode(result, "::ACos");
}

void QmitkRadiomicsArithmetric::SinButton()
{
  mitk::Image::Pointer firstImage = GetFirstImage();
  bool asDouble = m_Controls->m_ResultAsDouble->isChecked();
  if (firstImage.IsNull())
  {
    return;
  }
  auto result = mitk::ArithmeticOperation::Sin(firstImage, asDouble);
  AddImageToNode(result, "::Sin");
}

void QmitkRadiomicsArithmetric::ASinButton()
{
  mitk::Image::Pointer firstImage = GetFirstImage();
  bool asDouble = m_Controls->m_ResultAsDouble->isChecked();
  if (firstImage.IsNull())
  {
    return;
  }
  auto result = mitk::ArithmeticOperation::Asin(firstImage, asDouble);
  AddImageToNode(result, "::ASin");
}

void QmitkRadiomicsArithmetric::SquareButton()
{
  mitk::Image::Pointer firstImage = GetFirstImage();
  bool asDouble = m_Controls->m_ResultAsDouble->isChecked();
  if (firstImage.IsNull())
  {
    return;
  }
  auto result = mitk::ArithmeticOperation::Square(firstImage, asDouble);
  AddImageToNode(result, "::Square");
}

void QmitkRadiomicsArithmetric::SqrtButton()
{
  mitk::Image::Pointer firstImage = GetFirstImage();
  bool asDouble = m_Controls->m_ResultAsDouble->isChecked();
  if (firstImage.IsNull())
  {
    return;
  }
  auto result = mitk::ArithmeticOperation::Sqrt(firstImage, asDouble);
  AddImageToNode(result, "::Sqrt");
}

void QmitkRadiomicsArithmetric::AbsButton()
{
  mitk::Image::Pointer firstImage = GetFirstImage();
  bool asDouble = m_Controls->m_ResultAsDouble->isChecked();
  if (firstImage.IsNull())
  {
    return;
  }
  auto result = mitk::ArithmeticOperation::Abs(firstImage, asDouble);
  AddImageToNode(result, "::Abs");
}

void QmitkRadiomicsArithmetric::ExpButton()
{
  mitk::Image::Pointer firstImage = GetFirstImage();
  bool asDouble = m_Controls->m_ResultAsDouble->isChecked();
  if (firstImage.IsNull())
  {
    return;
  }
  auto result = mitk::ArithmeticOperation::Exp(firstImage, asDouble);
  AddImageToNode(result, "::Exp");
}

void QmitkRadiomicsArithmetric::ExpNegButton()
{
  mitk::Image::Pointer firstImage = GetFirstImage();
  bool asDouble = m_Controls->m_ResultAsDouble->isChecked();
  if (firstImage.IsNull())
  {
    return;
  }
  auto result = mitk::ArithmeticOperation::ExpNeg(firstImage, asDouble);
  AddImageToNode(result, "::ExpNeg");
}

void QmitkRadiomicsArithmetric::Log10Button()
{
  mitk::Image::Pointer firstImage = GetFirstImage();
  bool asDouble = m_Controls->m_ResultAsDouble->isChecked();
  if (firstImage.IsNull())
  {
    return;
  }
  auto result = mitk::ArithmeticOperation::Log10(firstImage, asDouble);
  AddImageToNode(result, "::Log10");
}

void QmitkRadiomicsArithmetric::AddLeftButton()
{
  mitk::Image::Pointer firstImage = GetFirstImage();
  bool asDouble = m_Controls->m_ResultAsDouble->isChecked();
  double value = m_Controls->m_DoubleValue->value();
  if (firstImage.IsNull())
  {
    return;
  }
  auto result = mitk::ArithmeticOperation::Add(firstImage, value, asDouble);
  AddImageToNode(result, "::Add");
}

void QmitkRadiomicsArithmetric::SubLeftButton()
{
  mitk::Image::Pointer firstImage = GetFirstImage();
  bool asDouble = m_Controls->m_ResultAsDouble->isChecked();
  double value = m_Controls->m_DoubleValue->value();
  if (firstImage.IsNull())
  {
    return;
  }
  auto result = mitk::ArithmeticOperation::Subtract(firstImage, value, asDouble);
  AddImageToNode(result, "::Sub");
}

void QmitkRadiomicsArithmetric::SubRightButton()
{
  mitk::Image::Pointer firstImage = GetFirstImage();
  bool asDouble = m_Controls->m_ResultAsDouble->isChecked();
  double value = m_Controls->m_DoubleValue->value();
  if (firstImage.IsNull())
  {
    return;
  }
  auto result = mitk::ArithmeticOperation::Subtract(value, firstImage, asDouble);
  AddImageToNode(result, "::Sub");
}

void QmitkRadiomicsArithmetric::MulLeftButton()
{
  mitk::Image::Pointer firstImage = GetFirstImage();
  bool asDouble = m_Controls->m_ResultAsDouble->isChecked();
  double value = m_Controls->m_DoubleValue->value();
  if (firstImage.IsNull())
  {
    return;
  }
  auto result = mitk::ArithmeticOperation::Multiply(firstImage, value, asDouble);
  AddImageToNode(result, "::Mul");
}

void QmitkRadiomicsArithmetric::DivLeftButton()
{
  mitk::Image::Pointer firstImage = GetFirstImage();
  bool asDouble = m_Controls->m_ResultAsDouble->isChecked();
  double value = m_Controls->m_DoubleValue->value();
  if (firstImage.IsNull())
  {
    return;
  }
  auto result = mitk::ArithmeticOperation::Divide(firstImage, value, asDouble);
  AddImageToNode(result, "::Div");
}

void QmitkRadiomicsArithmetric::DivRightButton()
{
  mitk::Image::Pointer firstImage = GetFirstImage();
  bool asDouble = m_Controls->m_ResultAsDouble->isChecked();
  double value = m_Controls->m_DoubleValue->value();
  if (firstImage.IsNull())
  {
    return;
  }
  auto result = mitk::ArithmeticOperation::Divide(value, firstImage,asDouble);
  AddImageToNode(result, "::Div");
}

void QmitkRadiomicsArithmetric::AddButton()
{
  mitk::Image::Pointer firstImage = GetFirstImage();
  mitk::Image::Pointer secondImage = GetSecondImage();
  bool asDouble = m_Controls->m_ResultAsDouble->isChecked();
  if (firstImage.IsNull())
  {
    return;
  }
  auto result = mitk::ArithmeticOperation::Add(firstImage, secondImage, asDouble);
  AddImageToNode(result, "::Add");
}

void QmitkRadiomicsArithmetric::MulButton()
{
  mitk::Image::Pointer firstImage = GetFirstImage();
  mitk::Image::Pointer secondImage = GetSecondImage();
  bool asDouble = m_Controls->m_ResultAsDouble->isChecked();
  if (firstImage.IsNull())
  {
    return;
  }
  auto result = mitk::ArithmeticOperation::Multiply(firstImage, secondImage, asDouble);
  AddImageToNode(result, "::Mul");
}

void QmitkRadiomicsArithmetric::SubButton()
{
  mitk::Image::Pointer firstImage = GetFirstImage();
  mitk::Image::Pointer secondImage = GetSecondImage();
  bool asDouble = m_Controls->m_ResultAsDouble->isChecked();
  if (firstImage.IsNull())
  {
    return;
  }
  auto result = mitk::ArithmeticOperation::Subtract(firstImage, secondImage, asDouble);
  AddImageToNode(result, "::Sub");
}

void QmitkRadiomicsArithmetric::DivButton()
{
  mitk::Image::Pointer firstImage = GetFirstImage();
  mitk::Image::Pointer secondImage = GetSecondImage();
  bool asDouble = m_Controls->m_ResultAsDouble->isChecked();
  if (firstImage.IsNull())
  {
    return;
  }
  auto result = mitk::ArithmeticOperation::Divide(firstImage, secondImage, asDouble);
  AddImageToNode(result, "::Div");
}

void QmitkRadiomicsArithmetric::SetFocus()
{
}

//datamanager selection changed
void QmitkRadiomicsArithmetric::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& nodes)
{
  //any nodes there?
  if (!nodes.empty())
  {

  }
}
