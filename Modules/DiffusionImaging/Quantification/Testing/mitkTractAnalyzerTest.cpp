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
#include <mitkPointSetReader.h>
#include <mitkNrrdTbssRoiImageReader.h>

/**Documentation
 *  test for the class "mitkTractAnalyzer".
 */
int mitkTractAnalyzerTest(int argc , char* argv[])
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

  mitk::PointSetReader::Pointer pointSetReader = mitk::PointSetReader::New();
  pointSetReader->SetFileName(argv[2]);
  pointSetReader->Update();

  mitk::PointSet::Pointer pointSet = pointSetReader->GetOutput();



  mitk::TractAnalyzer analyzer;

  analyzer.SetInputImage(mitkImage);
  analyzer.SetThreshold(0.2);
  analyzer.SetPointSet(pointSet);
  analyzer.MakeRoi();

  mitk::TbssRoiImage::Pointer tbssRoi = analyzer.GetRoiImage();



  // load reference roi
  mitk::NrrdTbssRoiImageReader::Pointer roiReader = mitk::NrrdTbssRoiImageReader::New();
  roiReader->SetFileName(argv[3]);
  roiReader->Update();

  mitk::TbssRoiImage* referenceRoi = roiReader->GetOutput(0);


  // compare point sets of tbssRoi and reference roi

  std::vector< itk::Index<3> > roi = tbssRoi->GetRoi();
  std::vector< itk::Index<3> > refRoi = referenceRoi->GetRoi();






  bool equalSize(roi.size() == refRoi.size());

  std::cout << "roi size: " << roi.size() << '\n';
  std::cout << "ref roi size: " << refRoi.size() << std::endl;

 // MITK_TEST_EQUAL(roi.size(), refRoi.size(), "Size of roi and control roi are the same.");

  if(equalSize)
  {
    bool samePath = true;
    for(int t=0; t<roi.size(); t++)
    {
      itk::Index<3> ix = roi.at(t);
      itk::Index<3> refIx = refRoi.at(t);

      if(ix != refIx)
      {
        samePath = false;
      }
    }

    MITK_TEST_CONDITION(samePath, "Calculated ROI matches reference ROI.");
  }


  MITK_TEST_END();
}
