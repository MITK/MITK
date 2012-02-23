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
#include <QmitkStdMultiWidget.h>
#include <mitkProperties.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkImageCast.h>
#include <mitkImageTimeSelector.h>
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

bool QmitkBooleanOperationsView::CheckSegmentationImages()
{
  mitk::Image::Pointer image1 = dynamic_cast<mitk::Image *>(m_Controls.cmbSegmentationImage1->GetSelectedNode()->GetData());
  mitk::Image::Pointer image2 = dynamic_cast<mitk::Image *>(m_Controls.cmbSegmentationImage2->GetSelectedNode()->GetData());

  if (image1.IsNull() || image2.IsNull())
  {
    MITK_ERROR << "At least one input segmentation image is invalid!";
    return false;
  }

  if (image1->GetDimension() != image2->GetDimension())
  {
    MITK_ERROR << "Dimensions of input segmentation images are different!";
    return false;
  }

  if (image1->GetDimension() == 4)
  {
    unsigned int time = GetActiveStdMultiWidget()->GetTimeNavigationController()->GetTime()->GetPos();

    if (time >= image1->GetDimension(3) || time >= image2->GetDimension(3))
    {
      MITK_ERROR << "At least one input segmentation image has no data for current time slice!";
      return false;
    }
  }

  return true;
}

mitk::Image::Pointer QmitkBooleanOperationsView::To3D(const mitk::Image::Pointer &image)
{
  if (image->GetDimension() == 4)
  {
    mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();

    imageTimeSelector->SetInput(image);
    imageTimeSelector->SetTimeNr(static_cast<int>(GetActiveStdMultiWidget()->GetTimeNavigationController()->GetTime()->GetPos()));
    
    imageTimeSelector->UpdateLargestPossibleRegion();

    return imageTimeSelector->GetOutput();
  }
  else
  {
    return image;
  }
}

void QmitkBooleanOperationsView::AddToDataStorage(const mitk::Image::Pointer &image, const std::string &prefix) const
{
  mitk::DataNode::Pointer dataNode = mitk::DataNode::New();

  dataNode->SetBoolProperty("binary", true);
  dataNode->SetName(prefix + m_Controls.cmbSegmentationImage2->GetSelectedNode()->GetName());
  dataNode->SetData(image);

  GetDefaultDataStorage()->Add(dataNode, m_Controls.cmbSegmentationImage1->GetSelectedNode());
}

void QmitkBooleanOperationsView::EnableButtons(bool enable)
{
  m_Controls.lblInputImagesWarning->setVisible(!enable);
  m_Controls.btnDifference->setEnabled(enable);
  m_Controls.btnUnion->setEnabled(enable);
  m_Controls.btnIntersection->setEnabled(enable);
}

void QmitkBooleanOperationsView::DisableButtons()
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
  if (!CheckSegmentationImages())
    return;

  mitk::Image::Pointer image1 = To3D(dynamic_cast<mitk::Image *>(m_Controls.cmbSegmentationImage1->GetSelectedNode()->GetData()));
  mitk::Image::Pointer image2 = To3D(dynamic_cast<mitk::Image *>(m_Controls.cmbSegmentationImage2->GetSelectedNode()->GetData()));

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

  AddToDataStorage(image3, "Difference_");
}

void QmitkBooleanOperationsView::OnUnionButtonClicked()
{
  if (!CheckSegmentationImages())
    return;

  mitk::Image::Pointer image1 = To3D(dynamic_cast<mitk::Image *>(m_Controls.cmbSegmentationImage1->GetSelectedNode()->GetData()));
  mitk::Image::Pointer image2 = To3D(dynamic_cast<mitk::Image *>(m_Controls.cmbSegmentationImage2->GetSelectedNode()->GetData()));

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

  AddToDataStorage(image3, "Union_");
}

void QmitkBooleanOperationsView::OnIntersectionButtonClicked()
{
  if (!CheckSegmentationImages())
    return;

  mitk::Image::Pointer image1 = To3D(dynamic_cast<mitk::Image *>(m_Controls.cmbSegmentationImage1->GetSelectedNode()->GetData()));
  mitk::Image::Pointer image2 = To3D(dynamic_cast<mitk::Image *>(m_Controls.cmbSegmentationImage2->GetSelectedNode()->GetData()));

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

  AddToDataStorage(image3, "Intersection_");
}
