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

#include <mitkBaseData.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <metaCommand.h>
#include <mitkCommandLineParser.h>
#include <usAny.h>
#include <mitkIOUtil.h>
#include <mitkLexicalCast.h>
#include <itksys/SystemTools.hxx>
#include <itkDirectory.h>
#include <mitkFiberBundle.h>
#include <vtkTransformPolyDataFilter.h>
#include <fstream>
#include <chrono>
#include <boost/progress.hpp>
#include <itkTractsToFiberEndingsImageFilter.h>
#include <itkTractDensityImageFilter.h>
#include <itkImageRegionConstIterator.h>

typedef itksys::SystemTools ist;
typedef itk::Image<unsigned char, 3>    ItkUcharImgType;
typedef itk::Image<unsigned int, 3>    ItkUIntImgType;

/*!
\brief
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Calculate Overlap");
  parser.setCategory("Fiber Tracking Evaluation");
  parser.setDescription("");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("tractogram", "", mitkCommandLineParser::String, "", "file", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("overlap_image", "", mitkCommandLineParser::String, "", "file", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("ep_image", "", mitkCommandLineParser::String, "", "file", us::Any(), false, false, false, mitkCommandLineParser::Input);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string input_tractogram = us::any_cast<std::string>(parsedArgs["tractogram"]);
  std::string mask_file = us::any_cast<std::string>(parsedArgs["overlap_image"]);
  std::string mask_file2 = us::any_cast<std::string>(parsedArgs["ep_image"]);

  try
  {
    mitk::FiberBundle::Pointer fib = mitk::IOUtil::Load<mitk::FiberBundle>(input_tractogram);
    mitk::Image::Pointer mask = mitk::IOUtil::Load<mitk::Image>(mask_file);
    mitk::Image::Pointer mask2 = mitk::IOUtil::Load<mitk::Image>(mask_file2);

    ItkUcharImgType::Pointer itk_mask;
    mitk::CastToItkImage(mask, itk_mask);

    ItkUcharImgType::Pointer itk_mask2;
    mitk::CastToItkImage(mask2, itk_mask2);

    float ol = fib->GetOverlap(itk_mask);
    float ep = fib->GetNumEpFractionInMask(itk_mask2, true);
    MITK_INFO << "Overlap<<" << ol;
    MITK_INFO << "EP-Fraction<<" << ep;
  }
  catch (itk::ExceptionObject e)
  {
    std::cout << e;
    return EXIT_FAILURE;
  }
  catch (std::exception e)
  {
    std::cout << e.what();
    return EXIT_FAILURE;
  }
  catch (...)
  {
    std::cout << "ERROR!?!";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
