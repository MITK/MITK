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
#include "QmitkOtsuAction.h"

// MITK
#include <itkOtsuMultipleThresholdsImageFilter.h>
#include <mitkRenderingManager.h>
#include <mitkImage.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>
#include <mitkLevelWindowProperty.h>

#include <mitkLookupTable.h>
#include <mitkLookupTableProperty.h>

// ITK
#include <itkMultiplyImageFilter.h>

// Qt
#include <QDialog>
#include <QVBoxLayout>
#include <QApplication>
#include <QList>
#include <QLabel>
#include <QMessageBox>

using namespace berry;
using namespace mitk;
using namespace std;

QmitkOtsuAction::QmitkOtsuAction()
: m_OtsuSegmentationDialog(NULL)
{
}

QmitkOtsuAction::~QmitkOtsuAction()
{
}

void QmitkOtsuAction::Run(const QList<DataNode::Pointer> &selectedNodes)
{
  this->m_DataNode = selectedNodes[0];
  //this->m_selectedNodes = selectedNodes;

  m_OtsuSegmentationDialog = new QDialog(QApplication::activeWindow(),Qt::WindowTitleHint | Qt::WindowSystemMenuHint);


  QVBoxLayout *layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  QHBoxLayout* spinBoxLayout = new QHBoxLayout;
  QHBoxLayout* buttonLayout = new QHBoxLayout;

  m_OtsuSpinBox = new QSpinBox;
  m_OtsuSpinBox->setRange(2, 32);
  m_OtsuSpinBox->setValue(2);

  m_OtsuPushButton = new QPushButton("OK");
  QPushButton* CancelButton = new QPushButton("Cancel");

  connect(m_OtsuPushButton, SIGNAL(clicked()), this, SLOT(OtsuSegmentationDone()));
  connect(CancelButton, SIGNAL(clicked()), m_OtsuSegmentationDialog, SLOT(reject()));

  QLabel* numberOfThresholdsLabel = new QLabel("Select number of Regions of Interest:");
  numberOfThresholdsLabel->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
  layout->addWidget(numberOfThresholdsLabel);
  layout->addLayout(spinBoxLayout);
  spinBoxLayout->addSpacing(50);
  spinBoxLayout->addWidget(m_OtsuSpinBox);
  spinBoxLayout->addSpacing(50);
  layout->addLayout(buttonLayout);
  buttonLayout->addWidget(m_OtsuPushButton);
  buttonLayout->addWidget(CancelButton);

  m_OtsuSegmentationDialog->setLayout(layout);
  m_OtsuSegmentationDialog->setFixedSize(300, 80);

  m_OtsuSegmentationDialog->open();
}

void QmitkOtsuAction::OtsuSegmentationDone()
{

  /*
  if (result == QDialog::Rejected)
  m_ThresholdingToolManager->ActivateTool(-1);*/

  this->PerformOtsuSegmentation();

  m_OtsuSegmentationDialog->deleteLater();
  m_OtsuSegmentationDialog = NULL;

  //m_ThresholdingToolManager->SetReferenceData(NULL);
  //m_ThresholdingToolManager->SetWorkingData(NULL);

  RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkOtsuAction::SetDataStorage(DataStorage *dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkOtsuAction::SetFunctionality(QtViewPart* /*functionality*/)
{
}

void QmitkOtsuAction::PerformOtsuSegmentation()
{
  this->m_OtsuSegmentationDialog->setCursor(Qt::WaitCursor);

  int numberOfThresholds = this->m_OtsuSpinBox->value() - 1;
  int proceed;

  QMessageBox* messageBox = new QMessageBox(QMessageBox::Question, NULL, "The otsu segmentation computation may take several minutes depending on the number of Regions you selected. Proceed anyway?", QMessageBox::Ok | QMessageBox::Cancel);
  if (numberOfThresholds >= 5)
  {
    proceed = messageBox->exec();
    if (proceed != QMessageBox::Ok) return;
  }

  mitk::Image::Pointer mitkImage = 0;

  mitkImage = dynamic_cast<mitk::Image*>( this->m_DataNode->GetData() );

  try
  {
    // get selected mitk image
    const unsigned short dim = 3;
    typedef short InputPixelType;
    typedef unsigned char OutputPixelType;

    typedef itk::Image< InputPixelType, dim > InputImageType;
    typedef itk::Image< OutputPixelType, dim > OutputImageType;

    //typedef itk::OtsuThresholdImageFilter< InputImageType, OutputImageType > FilterType;
    typedef itk::OtsuMultipleThresholdsImageFilter< InputImageType, OutputImageType > FilterType;
    //typedef itk::MultiplyImageFilter< OutputImageType, OutputImageType, OutputImageType> MultiplyFilterType;
    //typedef itk::RandomImageSource< OutputImageType> RandomImageSourceType;
    //typedef itk::ImageRegionIterator< OutputImageType > ImageIteratorType;


    FilterType::Pointer filter = FilterType::New();
    //MultiplyFilterType::Pointer multiplyImageFilter = MultiplyFilterType::New();
    //RandomImageSourceType::Pointer randomImageSource = RandomImageSourceType::New();

    filter->SetNumberOfThresholds(numberOfThresholds);
    //filter->SetLabelOffset(0);
    /*
    filter->SetOutsideValue( 1 );
    filter->SetInsideValue( 0 );*/

    InputImageType::Pointer itkImage;
    mitk::CastToItkImage(mitkImage, itkImage);

    filter->SetInput( itkImage );
//    filter->UpdateOutputInformation();

    //multiplyImageFilter->SetInput1(filter->GetOutput());
    //OutputImageType::Pointer constantImage = OutputImageType::New();
    //constantImage->SetLargestPossibleRegion(filter->GetOutput()->GetLargestPossibleRegion());
    //constantImage->SetBufferedRegion(filter->GetOutput()->GetLargestPossibleRegion());
    //constantImage->Allocate();
    //ImageIteratorType it(constantImage, constantImage->GetLargestPossibleRegion());
    //while (!it.IsAtEnd())
    //{
    //  it.Set(1);
    //  ++it;
    //}

    ////randomImageSource->SetSize(filter->GetOutput()->GetLargestPossibleRegion().GetSize());
    ////multiplyImageFilter->SetInput2(randomImageSource->GetOutput());
    //multiplyImageFilter->SetInput2(constantImage);

    filter->Update();
    //multiplyImageFilter->Update();

    mitk::DataNode::Pointer resultNode = mitk::DataNode::New();
    std::string nameOfResultImage = this->m_DataNode->GetName();
    nameOfResultImage.append("Otsu");
    resultNode->SetProperty("name", mitk::StringProperty::New(nameOfResultImage) );
    resultNode->SetProperty("binary", mitk::BoolProperty::New(false) );
    resultNode->SetProperty("use color", mitk::BoolProperty::New(false) );

    mitk::LookupTable::Pointer lut = mitk::LookupTable::New();

    mitk::LookupTableProperty::Pointer prop = mitk::LookupTableProperty::New(lut);

    //resultNode->GetProperty(prop, "LookupTable");

    vtkLookupTable *lookupTable = vtkLookupTable::New();
    lookupTable->SetHueRange(1.0, 0.0);
    lookupTable->SetSaturationRange(1.0, 1.0);
    lookupTable->SetValueRange(1.0, 1.0);
    lookupTable->SetTableRange(-1.0, 1.0);
    lookupTable->Build();

    lut->SetVtkLookupTable(lookupTable);

    prop->SetLookupTable(lut);
    resultNode->SetProperty("LookupTable",prop);

    mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
    mitk::LevelWindow levelwindow;
    levelwindow.SetRangeMinMax(0, numberOfThresholds);
    levWinProp->SetLevelWindow( levelwindow );
    resultNode->SetProperty( "levelwindow", levWinProp );

    //resultNode->SetData( mitk::ImportItkImage ( filter->GetOutput() ) );
    resultNode->SetData( mitk::ImportItkImage ( filter->GetOutput() ) );


    this->m_DataStorage->Add(resultNode, this->m_DataNode);

    this->m_OtsuSegmentationDialog->setCursor(Qt::ArrowCursor);

  }
  catch( std::exception& err )
  {
    MITK_ERROR(this->GetClassName()) << err.what();
  }
  }