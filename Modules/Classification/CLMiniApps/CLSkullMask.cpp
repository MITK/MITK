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
#ifndef mitkCLPolyToNrrd_cpp
#define mitkCLPolyToNrrd_cpp

#include "time.h"
#include <sstream>
#include <fstream>

#include <mitkIOUtil.h>
#include "mitkCommandLineParser.h"

#include "itkImageRegionIterator.h"
// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>
// ITK
#include <itkLabelStatisticsImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>

typedef itk::Image< double, 3 >                 FloatImageType;
typedef itk::Image< unsigned char, 3 >          MaskImageType;

template<typename TPixel, unsigned int VImageDimension>
static void
  DetectSkull(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer im2, mitk::Image::Pointer mask1, std::string output)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<unsigned char, VImageDimension> MaskType;

  typename ImageType::Pointer itkIm2 = ImageType::New();
  typename MaskType::Pointer itkMask1 = MaskType::New();
  mitk::CastToItkImage(im2, itkIm2);
  mitk::CastToItkImage(mask1, itkMask1);

  itk::ImageRegionIterator<ImageType> iterI1(itkImage, itkImage->GetLargestPossibleRegion());
  itk::ImageRegionIterator<ImageType> iterI2(itkIm2, itkImage->GetLargestPossibleRegion());
  itk::ImageRegionIterator<MaskType> iter(itkMask1, itkImage->GetLargestPossibleRegion());
  while (! iter.IsAtEnd())
  {
    unsigned char maskV = 0;
    if (iterI1.Value() > 0.0001 && iterI2.Value() > 0.00001)
      maskV = 1;
    iter.Set(maskV);
    ++iter;
    ++iterI1;
    ++iterI2;
  }

  mitk::Image::Pointer img = mitk::ImportItkImage(itkMask1);
  mitk::IOUtil::Save(img, output);
}

int main(int argc, char* argv[])
{
  typedef itk::Image<double, 3>         ImageType;
  typedef itk::Image<unsigned short, 3> MaskType;


  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  // required params
  parser.addArgument("image", "i", mitkCommandLineParser::StringList, "Input Image", "Path to the input images. Mask covers area of all images", us::Any(), false);
  parser.addArgument("mask", "m", mitkCommandLineParser::Image, "Input Mask", "The median of the area covered by this mask will be set to 1", us::Any(), false, false, false, mitkCommandLineParser::Input);

  // Miniapp Infos
  parser.setCategory("Classification Tools");
  parser.setTitle("MR Normalization Tool");
  parser.setDescription("Normalizes a MR image. Sets the Median of the tissue covered by mask 0 to 0 and the median of the area covered by mask 1 to 1.");
  parser.setContributor("MBI");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size()==0)
  {
    return EXIT_FAILURE;
  }
  if ( parsedArgs.count("help") || parsedArgs.count("h"))
  {
    return EXIT_SUCCESS;
  }
  us::Any listAny = parsedArgs["image"];
  auto inputImageList = us::any_cast<mitkCommandLineParser::StringContainerType>(listAny);

  std::vector<ImageType::Pointer> imageList;
  for (std::size_t i = 0; i < inputImageList.size(); ++i)
  {
    mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(inputImageList[i]);
    ImageType::Pointer itkImage = ImageType::New();
    mitk::CastToItkImage(image, itkImage);
    imageList.push_back(itkImage);
  }
  mitk::Image::Pointer mitkMask = mitk::IOUtil::Load<mitk::Image>(inputImageList[0]);
  MaskType::Pointer mask = MaskType::New();
  mitk::CastToItkImage(mitkMask, mask);

  itk::ImageRegionIterator<MaskType> maskIter(mask, mask->GetLargestPossibleRegion());
  while (!maskIter.IsAtEnd())
  {
    maskIter.Set(0);
    ++maskIter;
  }

  std::vector<ImageType::IndexType> listOfIndexes;
  listOfIndexes.reserve(1000);

  // Find Start Location for the case that one corner is "blocked" by content. Works only on the first image!
  ImageType::IndexType tmpIndex;
  ImageType::IndexType startIndex;
  startIndex.Fill(0);
  for (unsigned char i = 0; i < 8; ++i)
  {
    tmpIndex.Fill(0);
    if ((i & 1) > 0) tmpIndex[0] = mask->GetLargestPossibleRegion().GetSize(0)-1;
    if ((i & 2) > 0) tmpIndex[1] = mask->GetLargestPossibleRegion().GetSize(1)-1;
    if ((i & 4) > 0) tmpIndex[2] = mask->GetLargestPossibleRegion().GetSize(2)-1;

    MITK_INFO << tmpIndex;
    if (imageList[0]->GetPixel(tmpIndex) < imageList[0]->GetPixel(startIndex))
    {
      startIndex = tmpIndex;
    }
  }
  listOfIndexes.push_back(tmpIndex);

  while (listOfIndexes.size() > 0)
  {
    ImageType::IndexType currentIndex = listOfIndexes.back();
    listOfIndexes.pop_back();
    if (!(mask->GetLargestPossibleRegion().IsInside(currentIndex)))
    {
      continue;
    }
    if (mask->GetPixel(currentIndex) == 0)
    {
      mask->SetPixel(currentIndex, 1);
      double minimum = std::numeric_limits<double>::max();
      for (std::size_t i = 0; i < imageList.size(); ++i)
      {
        minimum = std::min<double>(minimum, imageList[i]->GetPixel(currentIndex));
      }
      if (minimum < 35)
      {
        mask->SetPixel(currentIndex, 2);
        tmpIndex = currentIndex;
        tmpIndex[0] += 1;
        listOfIndexes.push_back(tmpIndex);
        tmpIndex[0] -= 2;
        listOfIndexes.push_back(tmpIndex);
        tmpIndex[0] += 1;
        tmpIndex[1] += 1;
        listOfIndexes.push_back(tmpIndex);
        tmpIndex[1] -= 2;
        listOfIndexes.push_back(tmpIndex);
        tmpIndex[1] += 1;
        tmpIndex[2] += 1;
        listOfIndexes.push_back(tmpIndex);
        tmpIndex[2] -= 2;
        listOfIndexes.push_back(tmpIndex);
      }
    }
  }
  MITK_INFO << "Im here";
  maskIter.GoToBegin();
  while (!maskIter.IsAtEnd())
  {
    if (maskIter.Get() == 2)
      maskIter.Set(0);
    else
      maskIter.Set(1);
    ++maskIter;
  }

  mitk::Image::Pointer ergMask = mitk::ImportItkImage(mask);

  std::string maskPath = parsedArgs["mask"].ToString();
  mitk::IOUtil::Save(ergMask, maskPath);

  //AccessByItk_3(image, Normalize, im2, mask, parsedArgs["output"].ToString());

  return 0;
}

#endif
