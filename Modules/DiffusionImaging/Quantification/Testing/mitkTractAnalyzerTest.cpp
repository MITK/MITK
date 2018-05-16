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
#include <mitkTractAnalyzer.h>
#include <itkImageFileReader.h>
#include <mitkNrrdTbssRoiImageReader.h>
#include <mitkIOUtil.h>

/**Documentation
 *  test for the class "mitkTractAnalyzer".
 */
int mitkTractAnalyzerTest(int, char* argv[])
{

  MITK_TEST_BEGIN("TractAnalyzer");

  // Load image
  typedef itk::Image<float, 3> FloatImageType;
  typedef itk::ImageFileReader<FloatImageType> ImageReaderType;

  ImageReaderType::Pointer reader = ImageReaderType::New();

  reader->SetFileName(argv[1]);
  reader->Update();
  FloatImageType::Pointer itkImage = reader->GetOutput();
  mitk::Image::Pointer mitkImage = mitk::Image::New();

  mitk::CastToMitkImage(itkImage, mitkImage);

  // load point set
  mitk::PointSet::Pointer pointSet = mitk::IOUtil::Load<mitk::PointSet>(argv[2]);

  mitk::TractAnalyzer analyzer;

  analyzer.SetInputImage(mitkImage);
  analyzer.SetThreshold(0.2);
  analyzer.SetPointSet(pointSet);
  analyzer.MakeRoi();

  mitk::TbssRoiImage::Pointer tbssRoi = analyzer.GetRoiImage();

  std::vector< itk::Index<3> > roi = tbssRoi->GetRoi();



  // Output roi for debug purposes
  std::cout << "ROI\n";
  for(unsigned int t=0; t<roi.size(); t++)
  {
     itk::Index<3> ix = roi.at(t);
     std::cout << ix[0] << ", " << ix[1] << ", " << ix[2] << "\n";
  }


  std::cout << std::endl;

   // check the cost of the roi
  double cost = analyzer.GetCostSum();

  std::cout << "Cost: " << cost << std::endl;

  bool equal = mitk::Equal(cost, 5162.854, 0.001);

  MITK_TEST_CONDITION(equal, "Checking cost of found ROI");


  MITK_TEST_END();
}
