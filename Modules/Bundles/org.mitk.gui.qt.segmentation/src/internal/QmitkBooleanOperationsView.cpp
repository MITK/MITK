/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkBooleanOperationsView.h"
#include <mitkProperties.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkImageCast.h>
#include <itkAndImageFilter.h>
#include <itkNotImageFilter.h>
#include <itkOrImageFilter.h>

QmitkBooleanOperationsView::QmitkBooleanOperationsView()
  : m_Parent(NULL)
{
}

QmitkBooleanOperationsView::~QmitkBooleanOperationsView()
{
}

void QmitkBooleanOperationsView::CreateQtPartControl(QWidget *parent)
{
  mitk::DataStorage::Pointer dataStorage = GetDefaultDataStorage();
  
  mitk::NodePredicateAnd::Pointer segmentationPredicate = mitk::NodePredicateAnd::New(
    mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true)),
    mitk::NodePredicateNot::New(
      mitk::NodePredicateProperty::New("helper object")));
  
  m_Parent = parent;

  m_Controls.setupUi(parent);

  m_Controls.cmbSegmentationImage1->SetDataStorage(dataStorage);
  m_Controls.cmbSegmentationImage1->SetPredicate(segmentationPredicate);
  
  m_Controls.cmbSegmentationImage2->SetPredicate(segmentationPredicate);
  m_Controls.cmbSegmentationImage2->SetDataStorage(dataStorage);

  connect(m_Controls.cmbSegmentationImage1, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(OnSegmentationImage1Changed(const mitk::DataNode *)));
  connect(m_Controls.cmbSegmentationImage2, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(OnSegmentationImage2Changed(const mitk::DataNode *)));
  connect(m_Controls.btnDifference, SIGNAL(clicked()), this, SLOT(OnDifferenceButtonClicked()));
  connect(m_Controls.btnUnion, SIGNAL(clicked()), this, SLOT(OnUnionButtonClicked()));
  connect(m_Controls.btnIntersection, SIGNAL(clicked()), this, SLOT(OnIntersectionButtonClicked()));
}

void QmitkBooleanOperationsView::EnableButtons(bool enable)
{
  m_Controls.lblInputImagesWarning->setVisible(!enable);
  m_Controls.btnDifference->setEnabled(enable);
  m_Controls.btnUnion->setEnabled(enable);
  m_Controls.btnIntersection->setEnabled(enable);
}

void QmitkBooleanOperationsView::DisableButtons(bool disable)
{
  EnableButtons(false);
}

void QmitkBooleanOperationsView::OnSegmentationImage1Changed(const mitk::DataNode *dataNode)
{
  if (dataNode != m_Controls.cmbSegmentationImage2->GetSelectedNode())
    EnableButtons();
  else
    DisableButtons();
}

void QmitkBooleanOperationsView::OnSegmentationImage2Changed(const mitk::DataNode *dataNode)
{
  if (dataNode != m_Controls.cmbSegmentationImage1->GetSelectedNode())
    EnableButtons();
  else
    DisableButtons();
}

void QmitkBooleanOperationsView::OnDifferenceButtonClicked()
{
  mitk::DataNode::Pointer dataNode1 = m_Controls.cmbSegmentationImage1->GetSelectedNode();
  mitk::DataNode::Pointer dataNode2 = m_Controls.cmbSegmentationImage2->GetSelectedNode();
  
  mitk::Image::Pointer image1 = dynamic_cast<mitk::Image *>(dataNode1->GetData());
  mitk::Image::Pointer image2 = dynamic_cast<mitk::Image *>(dataNode2->GetData());

  typedef itk::Image<unsigned char, 3> ImageType;

  ImageType::Pointer itkImage1 = ImageType::New();
  ImageType::Pointer itkImage2 = ImageType::New();

  mitk::CastToItkImage(image1, itkImage1);
  mitk::CastToItkImage(image2, itkImage2);

  itk::NotImageFilter<ImageType, ImageType>::Pointer notFilter = itk::NotImageFilter<ImageType, ImageType>::New();
  notFilter->SetInput(itkImage2);

  itk::AndImageFilter<ImageType>::Pointer andFilter = itk::AndImageFilter<ImageType>::New();
  andFilter->SetInput1(itkImage1);
  andFilter->SetInput2(notFilter->GetOutput());
  andFilter->UpdateLargestPossibleRegion();

  mitk::Image::Pointer image3 = mitk::Image::New();
  mitk::CastToMitkImage<ImageType>(andFilter->GetOutput(), image3);

  image3->DisconnectPipeline();

  mitk::DataNode::Pointer dataNode3 = mitk::DataNode::New();
  dataNode3->SetBoolProperty("binary", true);
  dataNode3->SetName(std::string("Difference_") + dataNode2->GetName());
  dataNode3->SetData(image3);

  GetDefaultDataStorage()->Add(dataNode3, dataNode1);
}

void QmitkBooleanOperationsView::OnUnionButtonClicked()
{
  mitk::DataNode::Pointer dataNode1 = m_Controls.cmbSegmentationImage1->GetSelectedNode();
  mitk::DataNode::Pointer dataNode2 = m_Controls.cmbSegmentationImage2->GetSelectedNode();
  
  mitk::Image::Pointer image1 = dynamic_cast<mitk::Image *>(dataNode1->GetData());
  mitk::Image::Pointer image2 = dynamic_cast<mitk::Image *>(dataNode2->GetData());

  typedef itk::Image<unsigned char, 3> ImageType;

  ImageType::Pointer itkImage1 = ImageType::New();
  ImageType::Pointer itkImage2 = ImageType::New();

  mitk::CastToItkImage(image1, itkImage1);
  mitk::CastToItkImage(image2, itkImage2);

  itk::OrImageFilter<ImageType>::Pointer orFilter = itk::OrImageFilter<ImageType>::New();
  orFilter->SetInput1(itkImage1);
  orFilter->SetInput2(itkImage2);
  orFilter->UpdateLargestPossibleRegion();

  mitk::Image::Pointer image3 = mitk::Image::New();
  mitk::CastToMitkImage<ImageType>(orFilter->GetOutput(), image3);

  image3->DisconnectPipeline();

  mitk::DataNode::Pointer dataNode3 = mitk::DataNode::New();
  dataNode3->SetBoolProperty("binary", true);
  dataNode3->SetName(std::string("Union_") + dataNode2->GetName());
  dataNode3->SetData(image3);

  GetDefaultDataStorage()->Add(dataNode3, dataNode1);
}

void QmitkBooleanOperationsView::OnIntersectionButtonClicked()
{
  mitk::DataNode::Pointer dataNode1 = m_Controls.cmbSegmentationImage1->GetSelectedNode();
  mitk::DataNode::Pointer dataNode2 = m_Controls.cmbSegmentationImage2->GetSelectedNode();
  
  mitk::Image::Pointer image1 = dynamic_cast<mitk::Image *>(dataNode1->GetData());
  mitk::Image::Pointer image2 = dynamic_cast<mitk::Image *>(dataNode2->GetData());

  typedef itk::Image<unsigned char, 3> ImageType;

  ImageType::Pointer itkImage1 = ImageType::New();
  ImageType::Pointer itkImage2 = ImageType::New();

  mitk::CastToItkImage(image1, itkImage1);
  mitk::CastToItkImage(image2, itkImage2);

  itk::AndImageFilter<ImageType>::Pointer andFilter = itk::AndImageFilter<ImageType>::New();
  andFilter->SetInput1(itkImage1);
  andFilter->SetInput2(itkImage2);
  andFilter->UpdateLargestPossibleRegion();

  mitk::Image::Pointer image3 = mitk::Image::New();
  mitk::CastToMitkImage<ImageType>(andFilter->GetOutput(), image3);

  image3->DisconnectPipeline();

  mitk::DataNode::Pointer dataNode3 = mitk::DataNode::New();
  dataNode3->SetBoolProperty("binary", true);
  dataNode3->SetName(std::string("Intersection_") + dataNode2->GetName());
  dataNode3->SetData(image3);

  GetDefaultDataStorage()->Add(dataNode3, dataNode1);
}
