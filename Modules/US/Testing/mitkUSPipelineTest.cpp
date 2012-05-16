/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-25 17:27:17 +0100 (Mo, 25 Feb 2008) $
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkUSVideoDevice.h"
#include "mitkTestingMacros.h"
#include "mitkUSImageToUSImageFilter.h"
#include "mitkPadImageFilter.h"

// START TESTFILER
// This is an specialization of the USImageToUSImageFIlter

  class TestUSFilter : public mitk::USImageToUSImageFilter
    {
    protected:
      TestUSFilter() : mitk::USImageToUSImageFilter(){};
      virtual ~TestUSFilter(){};

    public:
      mitkClassMacro(TestUSFilter, mitk::USImageToUSImageFilter);
      itkNewMacro(Self);

    virtual void GenerateOutputInformation()
      {     
      mitk::Image::Pointer inputImage  = (mitk::Image*) this->GetInput(0);
      mitk::Image::Pointer output      = (mitk::Image*) this->GetOutput(0);    
     if(inputImage.IsNull()) return;
     };
    

    void GenerateData()
      {
      MITK_INFO << "GenerateData called in Testfilter!";
      //mitk::Image::Pointer ni = const_cast<mitk::Image*>(this->GetInput(0));
      mitk::USImage::Pointer ni = this->GetInput(0);
      mitk::USImage::Pointer result = mitk::USImage::New();

      result->Initialize(ni);
      result->SetImportVolume(ni->GetData());

      mitk::USImageMetadata::Pointer meta = ni->GetMetadata();
      meta->SetDeviceComment("Test");
      result->SetMetadata(meta);
      SetNthOutput(0, result);
      MITK_INFO << "GenerateData completed in Testfilter!";
      };
    };

 
// END TESTFILTER

class mitkUSPipelineTestClass
{
 
public:

  static void TestPipelineUS(std::string videoFilePath)
  {   
   // Set up a pipeline
    mitk::USVideoDevice::Pointer videoDevice = mitk::USVideoDevice::New("C:\\Users\\maerz\\Videos\\Debut\\us.avi", "Manufacturer", "Model");
    TestUSFilter::Pointer filter = TestUSFilter::New();
    videoDevice->Update();
    filter->SetInput(videoDevice->GetOutput());
    filter->Update();
    MITK_TEST_CONDITION_REQUIRED(videoDevice.IsNotNull(), "videoDevice should not be null after instantiation");
   

    //mitk::USImage::Pointer result = dynamic_cast<mitk::USImage *> (filter->GetOutput(0));
    mitk::USImage::Pointer result = filter->GetOutput(0);
    MITK_TEST_CONDITION_REQUIRED(result.IsNotNull(), "resulting images should not be null");
    std::string model = result->GetMetadata()->GetDeviceModel();
    MITK_TEST_CONDITION_REQUIRED(model.compare("Model") == 0   , "Resulting images should have their metadata set correctly");
    
  }

};

/**
* This function is setting up a pipeline and checks the output for validity.
*/
int mitkUSPipelineTest(int  argc , char* argv[])
{
  MITK_TEST_BEGIN("mitkUSPipelineTest");

     mitkUSPipelineTestClass::TestPipelineUS(argv[1]);
    
    

  MITK_TEST_END();
}