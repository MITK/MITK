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

#include <itkImageFileWriter.h>
#include <mitkCollectionWriter.h>
#include <mitkTestingMacros.h>

#include <mitkDataCollection.h>
#include <mitkDataCollectionImageIterator.h>
#include <mitkDataCollectionSingleImageIterator.h>

#include <mitkImageGenerator.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <mitkIOUtil.h>

#include "mitkNumericTypes.h"
class mitkDataCollectionTestClass
{
private:
  mitk::Image::Pointer im1;
  mitk::Image::Pointer im2;
  mitk::Image::Pointer im3;
  mitk::Image::Pointer im4;

  mitk::DataCollection::Pointer dataCol1;
  mitk::DataCollection::Pointer dataCol2;

  mitk::DataCollection::Pointer col1;


public:
  mitk::DataCollection::Pointer m_Collection;


  void Init()
  {

    im1 = mitk::ImageGenerator::GenerateRandomImage<double>(2,2,2,1,2,2,2,1,0);
    im2 = mitk::ImageGenerator::GenerateRandomImage<double>(3,3,3,1,3,3,3,3,2);
    im3 = mitk::ImageGenerator::GenerateRandomImage<double>(4,4,4,1,4,4,4,5,4);
    im4 = mitk::ImageGenerator::GenerateRandomImage<double>(5,5,5,1,5,5,5,7,6);

    dataCol1 = mitk::DataCollection::New();
    dataCol2 = mitk::DataCollection::New();

    dataCol1->AddData(im1.GetPointer(),"T1");

    dataCol1->AddData(im2.GetPointer(),"T2");

    dataCol2->AddData(im3.GetPointer(),"T1");
    dataCol2->AddData(im4.GetPointer(),"T2");

    col1 = mitk::DataCollection::New();

    col1->SetName("GnaBla");
    col1->AddData(dataCol1.GetPointer(), "0001");
    col1->AddData(dataCol2.GetPointer(), "0002");

    m_Collection = mitk::DataCollection::New();

    m_Collection->SetName("DummyCollection");
    m_Collection->AddData(col1.GetPointer(), "C1");


  }

  void IteratorReachesEnd()
  {
    Init();
    mitk::DataCollectionSingleImageIterator<double, 3> iter(m_Collection, "T1");
    int count = 0;
    while ( ! iter.IsAtEnd())
    {
      ++iter;
      ++count;
    }
    MITK_TEST_CONDITION_REQUIRED(count == 2, "Number of Iterations match number of elements");
  }

  void IteratorGivesRightFirstImage()
  {
    Init();
    mitk::DataCollectionSingleImageIterator<double, 3> iter(m_Collection, "T1");

    int spacing = iter.GetImage()->GetSpacing()[0];

    MITK_INFO << "Spacing: " << spacing;
    MITK_TEST_CONDITION_REQUIRED(spacing == 2, "Image is the first image (Checked by Spacing)");
  }

  void IteratorGivesRightSecondImage()
  {
    Init();
    mitk::DataCollectionSingleImageIterator<double, 3> iter(m_Collection, "T1");
    ++iter;

    int spacing = iter.GetImage()->GetSpacing()[0];
    MITK_INFO << "Spacing: " << spacing;
    MITK_TEST_CONDITION_REQUIRED(spacing == 4, "Image is the second image (Checked by Spacing)");
  }

  void AddingImageAtFirstPositionIsPossible()
  {
    Init();
    mitk::DataCollectionSingleImageIterator<double, 3> iter(m_Collection, "T1");

    mitk::Image::Pointer im = mitk::ImageGenerator::GenerateRandomImage<double>(6,6,6,1,6,6,6,9,8);
    itk::Image<double, 3>::Pointer itkImage;
    mitk::CastToItkImage(im,itkImage);
    iter.AddImage(itkImage,"T3");

    itk::DataObject::Pointer obj = (dynamic_cast<mitk::DataCollection*> (col1->GetData("0001").GetPointer()))->GetData("T3");
    int spacing = dynamic_cast<itk::Image<double, 3> *> (obj.GetPointer())->GetSpacing()[0];
//    MITK_INFO << "Spacing: " << spacing;
    MITK_TEST_CONDITION_REQUIRED(spacing == 6, "Image is the first image (Checked by Spacing)");
  }

  void AddingImageAtSecondPositionIsPossible()
  {
    Init();
    mitk::DataCollectionSingleImageIterator<double, 3> iter(m_Collection, "T1");
    ++iter;

    mitk::Image::Pointer im = mitk::ImageGenerator::GenerateRandomImage<double>(6,6,6,1,6,6,6,9,8);
    itk::Image<double, 3>::Pointer itkImage;
    mitk::CastToItkImage(im,itkImage);
    iter.AddImage(itkImage,"T3");

    itk::DataObject::Pointer obj = (dynamic_cast<mitk::DataCollection*> (col1->GetData("0002").GetPointer()))->GetData("T3");
    int spacing = dynamic_cast<itk::Image<double, 3> *> (obj.GetPointer())->GetSpacing()[0];
//    MITK_INFO << "Spacing: " << spacing;
    MITK_TEST_CONDITION_REQUIRED(spacing == 6, "Image is the first image (Checked by Spacing)");
  }

};

int mitkDataCollectionImageIteratorTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkDataCollectionImageIteratorTest");

//MITK_TEST_CONDITION_REQUIRED(true,"Here we test our condition");
  mitkDataCollectionTestClass test;

  test.IteratorReachesEnd();
  test.IteratorGivesRightFirstImage();
  //test.IteratorGivesRightSecondImage(); // TODO: activate this test again as soon as bug 18365 is fixed.
  test.AddingImageAtFirstPositionIsPossible();
  test.AddingImageAtSecondPositionIsPossible();

  MITK_TEST_END();
}
