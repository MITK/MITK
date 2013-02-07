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

#include "QmitkBooleanOperationsView.h"
#include <QmitkStdMultiWidget.h>
#include <mitkProperties.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkImageCast.h>
#include <mitkImageTimeSelector.h>
#include <mitkImagePixelWriteAccessor.h>
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

  mitk::Image::Pointer image3 = mitk::Image::New();

  mitk::CastToItkImage(image1, itkImage1);
  mitk::CastToItkImage(image2, itkImage2);

  itk::NotImageFilter<ImageType, ImageType>::Pointer notFilter = itk::NotImageFilter<ImageType, ImageType>::New();

  if ( image1->GetDimensions()[0] != image2->GetDimensions()[0] || image1->GetDimensions()[1] != image2->GetDimensions()[1] ||
       image1->GetDimensions()[2] != image2->GetDimensions()[2] )
  {
    mitk::Image::Pointer smallInvertedImage = mitk::Image::New();
    if (itkImage1->GetLargestPossibleRegion().IsInside(itkImage2->GetLargestPossibleRegion()))
    {
      image3 = image1->Clone();
      notFilter->SetInput(itkImage2);
      notFilter->UpdateLargestPossibleRegion();
      mitk::CastToMitkImage<ImageType>(notFilter->GetOutput(), smallInvertedImage);
      this->ApplyBooleanOperationToImagesDifferentSize(smallInvertedImage, image1, image3, std::logical_and<unsigned char>());
    }
    else
    {
      image3 = image2->Clone();
      notFilter->SetInput(itkImage1);
      notFilter->UpdateLargestPossibleRegion();
      mitk::CastToMitkImage<ImageType>(notFilter->GetOutput(), smallInvertedImage);
      this->ApplyBooleanOperationToImagesDifferentSize(smallInvertedImage, image2, image3, std::logical_and<unsigned char>());
    }
  }
  else
  {
    notFilter->SetInput(itkImage2);

    itk::AndImageFilter<ImageType>::Pointer andFilter = itk::AndImageFilter<ImageType>::New();
    andFilter->SetInput1(itkImage1);
    andFilter->SetInput2(notFilter->GetOutput());
    andFilter->UpdateLargestPossibleRegion();

    mitk::Image::Pointer image3 = mitk::Image::New();
    mitk::CastToMitkImage<ImageType>(andFilter->GetOutput(), image3);

    image3->DisconnectPipeline();
  }


  AddToDataStorage(image3, "Difference_");
}

void QmitkBooleanOperationsView::OnUnionButtonClicked()
{
  if (!CheckSegmentationImages())
    return;

  mitk::Image* image1 = To3D(dynamic_cast<mitk::Image *>(m_Controls.cmbSegmentationImage1->GetSelectedNode()->GetData()));
  mitk::Image* image2 = To3D(dynamic_cast<mitk::Image *>(m_Controls.cmbSegmentationImage2->GetSelectedNode()->GetData()));

  typedef itk::Image<unsigned char, 3> ImageType;

  ImageType::Pointer itkImage1 = ImageType::New();
  ImageType::Pointer itkImage2 = ImageType::New();

  mitk::Image::Pointer image3 = mitk::Image::New();

  mitk::CastToItkImage(image1, itkImage1);
  mitk::CastToItkImage(image2, itkImage2);

  if ( image1->GetDimensions()[0] != image2->GetDimensions()[0] || image1->GetDimensions()[1] != image2->GetDimensions()[1] ||
       image1->GetDimensions()[2] != image2->GetDimensions()[2] )
  {
    if (itkImage1->GetLargestPossibleRegion().IsInside(itkImage2->GetLargestPossibleRegion()))
    {
      image3 = image1->Clone();
      this->ApplyBooleanOperationToImagesDifferentSize(image2, image1, image3, std::logical_or<unsigned char>());
    }
    else
    {
      image3 = image2->Clone();
      this->ApplyBooleanOperationToImagesDifferentSize(image1, image2, image3, std::logical_or<unsigned char>());
    }
  }
  else
  {
    itk::OrImageFilter<ImageType>::Pointer orFilter = itk::OrImageFilter<ImageType>::New();
    orFilter->SetInput1(itkImage1);
    orFilter->SetInput2(itkImage2);
    orFilter->UpdateLargestPossibleRegion();
    mitk::CastToMitkImage<ImageType>(orFilter->GetOutput(), image3);
    image3->DisconnectPipeline();
  }

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

  mitk::Image::Pointer image3 = mitk::Image::New();

  mitk::CastToItkImage(image1, itkImage1);
  mitk::CastToItkImage(image2, itkImage2);

  if ( image1->GetDimensions()[0] != image2->GetDimensions()[0] || image1->GetDimensions()[1] != image2->GetDimensions()[1] ||
       image1->GetDimensions()[2] != image2->GetDimensions()[2] )
  {
    if (itkImage1->GetLargestPossibleRegion().IsInside(itkImage2->GetLargestPossibleRegion()))
    {
      image3 = image1->Clone();
      this->ApplyBooleanOperationToImagesDifferentSize(image2, image1, image3, std::logical_and<unsigned char>());
    }
    else
    {
      image3 = image2->Clone();
      this->ApplyBooleanOperationToImagesDifferentSize(image1, image2, image3, std::logical_and<unsigned char>());
    }
  }
  else
  {
    itk::AndImageFilter<ImageType>::Pointer andFilter = itk::AndImageFilter<ImageType>::New();
    andFilter->SetInput1(itkImage1);
    andFilter->SetInput2(itkImage2);
    andFilter->UpdateLargestPossibleRegion();

    mitk::Image::Pointer image3 = mitk::Image::New();
    mitk::CastToMitkImage<ImageType>(andFilter->GetOutput(), image3);
    image3->DisconnectPipeline();
  }

  AddToDataStorage(image3, "Intersection_");
}

template<typename _BinaryOperation>
void QmitkBooleanOperationsView::ApplyBooleanOperationToImagesDifferentSize(mitk::Image* smallImage, mitk::Image* bigImage, mitk::Image* resultImage,
                                                                            _BinaryOperation logicalOperator)
{
  for (unsigned int i = 0; i < smallImage->GetDimensions()[0]; i++)
  {
    for (unsigned int j = 0; j < smallImage->GetDimensions()[1]; j++)
    {
      for (unsigned int k = 0; k < smallImage->GetDimensions()[2]; k++)
      {
        mitk::Index3D currentIndexSmall;
        currentIndexSmall[0] = i;
        currentIndexSmall[1] = j;
        currentIndexSmall[2] = k;

        unsigned char smallValue = smallImage->GetPixelValueByIndex(currentIndexSmall);
        mitk::Point3D worldPoint;
        mitk::Point3D indexPoint;
        indexPoint[0] = i;
        indexPoint[1] = j;
        indexPoint[2] = k;
        smallImage->GetGeometry()->IndexToWorld(indexPoint, worldPoint);
        itk::Image<unsigned char, 3>::IndexType currentIndexBig;
        resultImage->GetGeometry()->WorldToIndex(worldPoint,currentIndexBig);
        unsigned char bigValue = bigImage->GetPixelValueByIndex(currentIndexBig);
        bool result = logicalOperator (bigValue,smallValue);
        mitk::ImagePixelWriteAccessor<unsigned char, 3> writeAccess(resultImage);

        writeAccess.SetPixelByIndex(currentIndexBig, result);
      }
    }
  }
}
