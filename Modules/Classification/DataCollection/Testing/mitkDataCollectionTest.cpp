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

#include <mitkTestingMacros.h>

#include <mitkDataCollection.h>
#include <mitkDataCollectionImageIterator.h>

#include <mitkImageGenerator.h>
#include <mitkImageCast.h>
#include <mitkIOUtil.h>

class mitkDataCollectionTestClass
{
public:
  mitk::DataCollection::Pointer Fill_DataCollection()
  {
    mitk::DataCollection::Pointer col = mitk::DataCollection::New();
    mitk::Image::Pointer image1 = mitk::ImageGenerator::GenerateGradientImage<float>(10,5,8);
    mitk::IOUtil::SaveImage(image1, "d:\\tmp\\test.nrrd");
    typedef itk::Image<float, 3> ImageType;
    ImageType::Pointer iImage1;
    mitk::CastToItkImage(image1, iImage1);
    col->AddData(iImage1.GetPointer(), "t1","description");
    mitk::Image::Pointer image2 = mitk::ImageGenerator::GenerateGradientImage<float>(10,5,8);
    ImageType::Pointer iImage2;
    mitk::CastToItkImage(image2, iImage2);
    col->AddData(iImage2.GetPointer(), "t2","description");
    mitk::Image::Pointer image3 = mitk::ImageGenerator::GenerateGradientImage<float>(10,5,8);
    ImageType::Pointer iImage3;
    mitk::CastToItkImage(image3, iImage3);
    col->AddData(iImage2.GetPointer(), "t3","description");
    return col;
  };


/*  void Count_EmptyCollection_0()
  {
    mitk::mitkDataCollectionTestClass collection;
    MITK_TEST_CONDITION_REQUIRED(collection.Count()==0, "Count should return 0");
  }

  void Count_With3ImagesInCollection_AsPrecalculated()
  {
    mitk::Image::Pointer image1 = mitk::ImageGenerator::GenerateRandomImage<float>(10,5,8);
    mitk::Image::Pointer image2 = mitk::ImageGenerator::GenerateRandomImage<float>(30,20,5);
    mitk::Image::Pointer image3 = mitk::ImageGenerator::GenerateRandomImage<float>(7,5,1);

    mitk::ImageDataCollections collection;
    collection.SetNumberOfModalities(1);
    collection.SetNumberOfSets(3);
    collection.AddImage(0,0,image1);
    collection.AddImage(1,0,image2);
    collection.AddImage(2,0,image3);

    MITK_INFO << collection.Count();
    MITK_TEST_CONDITION_REQUIRED(collection.Count()==3435, "Count should be 3435");
  }

  void GetDataPoint_InImage_Possible()
  {
    mitk::Image::Pointer image1 = mitk::ImageGenerator::GenerateGradientImage<float>(10,5,8);
    mitk::ImageDataCollections coll;
    coll.SetNumberOfModalities(1);
    coll.SetNumberOfSets(1);
    coll.AddImage(0,0,image1);
    for (int i=0;i<60;i++)
      MITK_INFO << i << ": " << coll.GetDataPoint(i,0);
    mitk::IOUtil::SaveImage(image1,"d:\\tmp\\testbld.nrrd");
  }*/
};

int mitkDataCollectionTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkDataCollectionTest")

  mitkDataCollectionTestClass test;

  mitk::DataCollection::Pointer col = test.Fill_DataCollection();
  mitk::DataCollectionImageIterator<float, 3> iter(col, "t1");
  while (!iter.IsAtEnd())
  {
    MITK_INFO << iter.GetIndex() << " -- " << iter.GetVoxel();
    ++iter;
  }

  int value = 400;
  iter.ToBegin();
  while (!iter.IsAtEnd())
  {
    iter.SetVoxel(value);
    ++iter;
    --value;
  }

  iter.ToBegin();
  while (!iter.IsAtEnd())
  {
    MITK_INFO << iter.GetIndex() << " -- " << iter.GetVoxel();
    ++iter;
  }


  mitk::DataCollection::Pointer col2 = test.Fill_DataCollection();

  mitk::DataCollection::Pointer col3 = mitk::DataCollection::New();
  col->SetName("Eins");
  col2->SetName("Zwei");
  col3->SetName("Drei");
  col3->AddData(col.GetPointer(),"one","desc1");
  col3->AddData(col2.GetPointer(),"two","desc2");

  mitk::DataCollectionImageIterator<float, 3> iter2(col3, "t1");
  int count = 0;
  while (!iter2.IsAtEnd())
  {
    MITK_INFO << iter2.GetIndex() << " -- " << iter2.GetVoxel();
    ++iter2;
    ++count;
  }
  MITK_INFO << "Blub" << count;

  MITK_TEST_CONDITION_REQUIRED(true, "Message");
  MITK_TEST_END();
}
