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

#include "mitkCommandLineParser.h"
#include <mitkPlanarFigureMaskGenerator.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <mitkImageAccessByItk.h>


typedef itk::Image< double, 3 >                 FloatImageType;
typedef itk::Image< unsigned char, 3 >          MaskImageType;

struct MaskParameter
{
  mitk::Image::Pointer mask;
  unsigned int axis;
  unsigned int slice;
};

template < typename TPixel, unsigned int VImageDimension >
void CreateNewMask(const itk::Image< TPixel, VImageDimension > *image, MaskParameter param, mitk::Image::Pointer &output)
{
  int transform[3][2];
  transform[0][0] = 1; transform[0][1] = 2;
  transform[1][0] = 0; transform[1][1] = 2;
  transform[2][0] = 0; transform[2][1] = 1;

  typedef itk::Image<unsigned short, VImageDimension> MaskType;
  typedef itk::Image<unsigned short, 2> Mask2DType;
  typename Mask2DType::Pointer mask = Mask2DType::New();
  mitk::CastToItkImage(param.mask, mask);


  typename MaskType::Pointer mask3D = MaskType::New();
  mask3D->SetRegions(image->GetLargestPossibleRegion());
  mask3D->SetSpacing(image->GetSpacing());
  mask3D->SetOrigin(image->GetOrigin());
  mask3D->Allocate();

  itk::ImageRegionIteratorWithIndex<MaskType> iter(mask3D, mask3D->GetLargestPossibleRegion());
  while (!iter.IsAtEnd())
  {
    auto index = iter.GetIndex();
    iter.Set(0);
    if (index[param.axis] == param.slice)
    {
      Mask2DType::IndexType index2D;
      index2D[0] = index[transform[param.axis][0]];
      index2D[1] = index[transform[param.axis][1]];
      auto pixel = mask->GetPixel(index2D);
      iter.Set(pixel);
    }
    ++iter;
  }

  mitk::CastToMitkImage(mask3D, output);

}

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  // required params
  parser.addArgument("planar", "p", mitkCommandLineParser::Directory, "Input Polydata", "Path to the input VTK polydata", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("image", "i", mitkCommandLineParser::Directory, "Input Image", "Image which defines the dimensions of the Segmentation", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output file", "Output file. ", us::Any(), false, false, false, mitkCommandLineParser::Input);
  // Miniapp Infos
  parser.setCategory("Classification Tools");
  parser.setTitle("Planar Data to Nrrd Segmentation");
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

  try
  {
    mitk::BaseData::Pointer data = mitk::IOUtil::Load(parsedArgs["planar"].ToString())[0];
    mitk::PlanarFigure::Pointer planar = dynamic_cast<mitk::PlanarFigure*>(data.GetPointer());

    mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(parsedArgs["image"].ToString());

    mitk::PlanarFigureMaskGenerator::Pointer pfMaskGen = mitk::PlanarFigureMaskGenerator::New();
    pfMaskGen->SetPlanarFigure(planar);
    pfMaskGen->SetTimeStep(0);
    pfMaskGen->SetInputImage(image.GetPointer());

    mitk::Image::Pointer mask = pfMaskGen->GetMask();
    unsigned int axis = pfMaskGen->GetPlanarFigureAxis();
    unsigned int slice = pfMaskGen->GetPlanarFigureSlice();

    //itk::Image<unsigned short, 3>::IndexType index;
    mitk::Image::Pointer fullMask;
    MaskParameter param;
    param.slice = slice;
    param.axis = axis;
    param.mask = mask;
    AccessByItk_2(image, CreateNewMask, param, fullMask);

    std::string saveAs = parsedArgs["output"].ToString();
    MITK_INFO << "Save as: " << saveAs;
    mitk::IOUtil::Save(pfMaskGen->GetMask(), saveAs);
    mitk::IOUtil::Save(fullMask, saveAs);

    return 0;
  }
  catch (...)
  {
    return EXIT_FAILURE;
  }
}

#endif
