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

#include <mitkIOUtil.h>
#include <mitkPointSet.h>
#include <mitkImageCast.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <limits>

#include "mitkCommandLineParser.h"



typedef itk::Image< double, 3 >                  FloatImageType;
typedef itk::Image< unsigned short, 3 >          MaskImageType;

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  // required params
  parser.addArgument("pointset", "p", mitkCommandLineParser::Directory, "Input Polydata", "Path to the input VTK polydata", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("image", "i", mitkCommandLineParser::Directory, "Input Image", "Image which defines the dimensions of the Segmentation", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output file", "Output files. Two files are create, a .nrrd image and a 3d-vtk.", us::Any(), false, false, false, mitkCommandLineParser::Input);
  // Miniapp Infos
  parser.setCategory("Classification Tools");
  parser.setTitle("2D-Polydata to Nrrd Segmentation");
  parser.setDescription("Creates a Nrrd segmentation based on a 2d-vtk polydata.");
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

  mitk::BaseData::Pointer data = mitk::IOUtil::Load(parsedArgs["pointset"].ToString())[0];
  mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(parsedArgs["image"].ToString());

  //MITK_INFO << data;
  mitk::PointSet::Pointer points = dynamic_cast<mitk::PointSet*>(data.GetPointer());
  MaskImageType::Pointer mask = MaskImageType::New();
  mitk::CastToItkImage(image, mask);

  double minX, minY, minZ;
  double maxX, maxY, maxZ;
  minX = minY = minZ = std::numeric_limits<double>::max();
  maxX = maxY = maxZ = std::numeric_limits<double>::lowest();

  for (auto iter = points->Begin(); iter != points->End(); ++iter)
  {
    minX = std::min<double>(minX, iter.Value().GetElement(0));
    minY = std::min<double>(minY, iter.Value().GetElement(1));
    minZ = std::min<double>(minZ, iter.Value().GetElement(2));
    maxX = std::max<double>(maxX, iter.Value().GetElement(0));
    maxY = std::max<double>(maxY, iter.Value().GetElement(1));
    maxZ = std::max<double>(maxZ, iter.Value().GetElement(2));
  }
  MaskImageType::PointType point;
  MaskImageType::IndexType iMin;
  MaskImageType::IndexType iMax;
  point[0] = minX;
  point[1] = minY;
  point[2] = minZ;
  mask->TransformPhysicalPointToIndex(point, iMin);
  point[0] = maxX;
  point[1] = maxY;
  point[2] = maxZ;
  mask->TransformPhysicalPointToIndex(point, iMax);

  itk::ImageRegionIteratorWithIndex<MaskImageType> iter(mask, mask->GetLargestPossibleRegion());
  while (!iter.IsAtEnd())
  {
    MaskImageType::IndexType index = iter.GetIndex();
    if ((index[0] >= iMin[0]) && (index[1] >= iMin[1]) && (index[2] >= iMin[2]) &&
      (index[0] <= iMax[0]) && (index[1] <= iMax[1]) && (index[2] <= iMax[2]))
    {
      iter.Set(1);
    }
    else
    {
      iter.Set(0);
    }
    ++iter;
  }

  mitk::Image::Pointer ergImage = mitk::Image::New();
  mitk::CastToMitkImage(mask, ergImage);

  std::string saveAs = parsedArgs["output"].ToString();
  MITK_INFO << "Save as: " << saveAs;
  mitk::IOUtil::Save(ergImage, saveAs);

  return 0;
}

#endif
