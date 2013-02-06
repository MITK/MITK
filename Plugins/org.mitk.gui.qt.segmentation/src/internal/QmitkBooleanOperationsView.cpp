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

  mitk::CastToItkImage(image1, itkImage1);
  mitk::CastToItkImage(image2, itkImage2);

  for (unsigned int i = 0; i < image1->GetDimensions()[0]; i++)
  {
    for (unsigned int j = 0; j < image1->GetDimensions()[1]; j++)
    {
      for (unsigned int k = 0; k < image1->GetDimensions()[2]; k++)
      {
        mitk::Index3D p;
        p[0] = i;
        p[1] = j;
        p[2] = k;

        if (image1->GetPixelValueByIndex(p) != 0)
        {
          mitk::Point3D wp;
          mitk::Point3D ip;
          ip[0] = i;
          ip[1] = j;
          ip[2] = k;
          image1->GetGeometry()->IndexToWorld(ip, wp);
          ImageType::IndexType index;
          image2->GetGeometry()->WorldToIndex(wp,index);
          itkImage2->SetPixel(index, 0);
        }

      }
    }
  }




//  itk::OrImageFilter<ImageType>::Pointer orFilter = itk::OrImageFilter<ImageType>::New();
//  orFilter->SetInput1(itkImage1);
//  orFilter->SetInput2(itkImage2);
//  orFilter->UpdateLargestPossibleRegion();

  mitk::Image::Pointer image3 = mitk::Image::New();
  mitk::CastToMitkImage<ImageType>(itkImage2, image3);

  image3->DisconnectPipeline();



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

  MITK_INFO << "Size: " <<image1->GetDimensions()[0]<<","<<image1->GetDimensions()[1]<<","<<image1->GetDimensions()[2];
  MITK_INFO << "Size: " <<image2->GetDimensions()[0]<<","<<image2->GetDimensions()[1]<<","<<image2->GetDimensions()[2];

  if ( image1->GetDimensions()[0] != image2->GetDimensions()[0] || image1->GetDimensions()[1] != image2->GetDimensions()[1] ||
       image1->GetDimensions()[2] != image2->GetDimensions()[2] )
  {

    mitk::Image* smallMitkImage;

    if (itkImage1->GetLargestPossibleRegion().IsInside(itkImage2->GetLargestPossibleRegion()))
    {
      image3 = image1->Clone();
      smallMitkImage = image2;
    }
    else
    {
      image3 = image2->Clone();
      smallMitkImage = image1;
    }

    for (unsigned int i = 0; i < smallMitkImage->GetDimensions()[0]; i++)
    {
      for (unsigned int j = 0; j < smallMitkImage->GetDimensions()[1]; j++)
      {
        for (unsigned int k = 0; k < smallMitkImage->GetDimensions()[2]; k++)
        {
          mitk::Index3D currentIndexSmall;
          currentIndexSmall[0] = i;
          currentIndexSmall[1] = j;
          currentIndexSmall[2] = k;

          if (smallMitkImage->GetPixelValueByIndex(currentIndexSmall) != 0)
          {
            mitk::Point3D worldPoint;
            mitk::Point3D indexPoint;
            indexPoint[0] = i;
            indexPoint[1] = j;
            indexPoint[2] = k;
            smallMitkImage->GetGeometry()->IndexToWorld(indexPoint, worldPoint);
            ImageType::IndexType currentIndexBig;
            image3->GetGeometry()->WorldToIndex(worldPoint,currentIndexBig);
            mitk::ImagePixelWriteAccessor<unsigned char, 3> writeAccess(image3);
            writeAccess.SetPixelByIndex(currentIndexBig, 1);
          }

        }
      }
    }
    smallMitkImage = 0;
  }
  else
  {
    itk::OrImageFilter<ImageType>::Pointer orFilter = itk::OrImageFilter<ImageType>::New();
    orFilter->SetInput1(itkImage1);
    orFilter->SetInput2(itkImage2);
    orFilter->UpdateLargestPossibleRegion();
    mitk::CastToMitkImage<ImageType>(orFilter->GetOutput(), image3);
  }

  image3->DisconnectPipeline();

  AddToDataStorage(image3, "Union_");
}

//template<typename TPixel, unsigned int VImageDimension>
//void ItkImageProcessing( itk::Image<TPixel,VImageDimension>* imageBig, itk::Image<TPixel,VImageDimension>* imageSmall)
//{
//  typedef itk::Image<TPixel, VImageDimension> ImageType;
//  typedef itk::ImageRegionIteratorWithIndex<ImageType> ImageIterator;

//  ImageIterator imgRegionIterator (imageSmall, imageSmall->GetLargestPossibleRegion());
//  imgRegionIterator.GoToBegin();

//  while (!imgRegionIterator.IsAtEnd())
//  {
//    if (imgRegionIterator.GetPixel() == 1)
//    {
//      typename ImageType::IndexType index = imgRegionIterator.GetIndex();
//      imageBig->Transform
//      mitk::Point3D worldP;
//      m_geoSmall->IndexToWorld(index, worldP);
//      m_geoBig->WorldToIndex(worldP, index);
//      imageBig->SetPixel(index, 1);
//    }
//  }
//}

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
