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

//MITK
#include "mitkTestingMacros.h"
#include "mitkRenderingTestHelper.h"

//VTK
#include <vtkRegressionTestImage.h>


int mitkImageVtkMapper2DColorTest(int argc, char* argv[])
{
    // load all arguments into a datastorage, take last argument as reference rendering
    // setup a renderwindow of fixed size X*Y
    // render the datastorage
    // compare rendering to reference image
    MITK_TEST_BEGIN("mitkImageVtkMapper2DTest")

    // enough parameters?
    if ( argc < 2 )
    {
        MITK_TEST_OUTPUT( << "Usage: " << std::string(*argv) << " [file1 file2 ...] outputfile" )
                MITK_TEST_OUTPUT( << "Will render a central axial slice of all given files into outputfile" )
                exit( EXIT_SUCCESS );
    }

    mitkRenderingTestHelper renderingHelper(640, 480, argc, argv);
    //Set the opacity for all images
    renderingHelper.SetProperty("use color", mitk::BoolProperty::New(true));
    renderingHelper.SetProperty("color", mitk::ColorProperty::New(0.0f, 0.0f, 255.0f));
    //for now this test renders in sagittal view direction
    renderingHelper.SetViewDirection(mitk::SliceNavigationController::Sagittal);
    renderingHelper.Render();

    //use this to generate a reference screenshot or save the file:
    bool generateReferenceScreenshot = false;
    if(generateReferenceScreenshot)
    {
        renderingHelper.SaveAsPNG("/home/kilgus/Pictures/RenderingTestData/output.png");
    }

    //### Usage of vtkRegressionTestImage:
    //vtkRegressionTestImage( vtkRenderWindow )
    //Set a vtkRenderWindow containing the desired scene.
    //vtkRegressionTestImage automatically searches in argc and argv[]
    //for a path a valid image with -V. If the test failed with the
    //first image (foo.png) check if there are images of the form
    //foo_N.png (where N=1,2,3...) and compare against them.
    int retVal = vtkRegressionTestImage( renderingHelper.GetVtkRenderWindow() );

    //retVal meanings: (see VTK/Rendering/vtkTesting.h)
    //0 = test failed
    //1 = test passed
    //2 = test not run
    //3 = something with vtkInteraction
    MITK_TEST_CONDITION( retVal == 1, "VTK test result positive" );

    MITK_TEST_END();
}

