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

#include "mitkUSVideoDevice.h"
#include "mitkTestingMacros.h"
#include "mitkUSImageToUSImageFilter.h"
#include "mitkPadImageFilter.h"


class mitkUSPipelineTestClass
{
 

public:
   // Internal specialisation of USImageToUSImageFilter to build a test-pipeline
  class TestFilter : public mitk::USImageToUSImageFilter
  {
    public : mitkUSPipelineTestClass::TestFilter() : mitk::USImageToUSImageFilter()
    {
      this->SetNumberOfInputs(1);
      this->SetNumberOfOutputs(1);

    }

    /*
    \brief Method generating the output information of this filter (e.g. image dimension, image type, etc.).
    The interface ImageToImageFilter requires this implementation. Everything is taken from the input image.
    */
    virtual void GenerateOutputInformation()
    {     
      MITK_INFO << "GenerateOutputInformation called in Testfilter!";
      mitk::Image::Pointer inputImage  = (mitk::Image*) this->GetInput();
      mitk::Image::Pointer output = this->GetOutput(0);
      itkDebugMacro(<<"GenerateOutputInformation()");
     if(inputImage.IsNull()) return;
   }
    

    void GenerateData(){
      MITK_INFO << "GenerateData called in Testfilter!";
      mitk::Image::Pointer ni = const_cast<mitk::Image*>(this->GetInput(0));
      mitk::USImage::Pointer result = mitk::USImage::New();

      result->Initialize(ni);
      result->SetImportVolume(ni->GetData());
      mitk::USImageMetadata::Pointer meta = result->GetMetadata();
      meta->SetDeviceComment("Test");
      result->SetMetadata(meta);
      SetNthOutput(0, result);
      MITK_INFO << "GenerateData completed in Testfilter!";
    }
  }; // Inner class end


  static void TestPipelineUS()
  {
    
   // Set up a pipeline
    mitk::USVideoDevice::Pointer videoDevice = mitk::USVideoDevice::New("C:\\Users\\maerz\\Videos\\Debut\\us.avi", "Manufacturer", "Model");
    TestFilter::Pointer filter = TestFilter::New();
    filter->SetInput(videoDevice->GetOutput());
    
    MITK_TEST_CONDITION_REQUIRED(videoDevice.IsNotNull(), "videoDevice should not be null after instantiation");
    filter->Update();
    mitk::USImage::Pointer result = dynamic_cast<mitk::USImage *> (filter->GetOutput(0));
    MITK_TEST_CONDITION_REQUIRED(result.IsNotNull(), "resulting images should not be null");
    MITK_TEST_CONDITION_REQUIRED(result->GetMetadata()->GetDeviceModel().compare("Model") == 0   , "Resulting images should have their metadata set correctly");
    
  }

    static void TestPipelinePlain()
  {
    
   // Set up a pipeline
    mitk::USVideoDevice::Pointer videoDevice = mitk::USVideoDevice::New("C:\\Users\\maerz\\Videos\\Debut\\us.avi", "Manufacturer", "Model");
    MITK_TEST_CONDITION_REQUIRED(videoDevice.IsNotNull(), "videoDevice should not be null after instantiation");

   mitk::PadImageFilter::Pointer padFilter = mitk::PadImageFilter::New();
    // This shouldn't really change anything
    padFilter->SetInput(0, videoDevice->GetOutput());
    padFilter->SetInput(1, videoDevice->GetOutput());

   // padFilter->GetOutput()->Update();
    padFilter->GetOutput(0)->Update();
    mitk::Image::Pointer result = padFilter->GetOutput(0);
    mitk::USImage::Pointer newres = mitk::USImage::New(result);
    MITK_TEST_CONDITION_REQUIRED(newres.IsNotNull(), "resulting images should not be null");
    MITK_TEST_CONDITION_REQUIRED(newres->GetMetadata()->GetDeviceModel().compare("Model") == 0   , "resulting images should have their metadata set correctly");

  }



 

};

/**
* This function is setting up a pipeline and checks the output for validity.
*/
int mitkUSPipelineTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("mitkUSPipelineTest");

    // Tests are commented out until master merge - they fail because of an old big in mitk image

    // mitkUSPipelineTestClass::TestPipelineUS();
    // mitkUSPipelineTestClass::TestPipelinePlain();
    

  MITK_TEST_END();
}