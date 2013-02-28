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
#include <mitkTransferFunctionProperty.h>
#include <mitkTransferFunction.h>


int mitkImageVtkMapper2DTransferFunctionTest(int argc, char* argv[])
{
    // load all arguments into a datastorage, take last argument as reference rendering
    // setup a renderwindow of fixed size X*Y
    // render the datastorage
    // compare rendering to reference image
    MITK_TEST_BEGIN("mitkImageVtkMapper2DTransferFunctionTest")

    // enough parameters?
    if ( argc < 2 )
    {
        MITK_TEST_OUTPUT( << "Usage: " << std::string(*argv) << " [file1 file2 ...] outputfile" )
                MITK_TEST_OUTPUT( << "Will render a central axial slice of all given files into outputfile" )
                exit( EXIT_SUCCESS );
    }
    mitkRenderingTestHelper renderingHelper(640, 480, argc, argv);

    //define an arbitrary colortransferfunction
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
    colorTransferFunction->SetColorSpaceToRGB();
    colorTransferFunction->AddRGBPoint(0.0, 1, 0, 0); //black = red
    colorTransferFunction->AddRGBPoint(127.5, 0, 1, 0); //grey = green
    colorTransferFunction->AddRGBPoint(255.0, 0, 0, 1); //white = blue
    mitk::TransferFunction::Pointer transferFucntion = mitk::TransferFunction::New();
    transferFucntion->SetColorTransferFunction( colorTransferFunction );

    //set the property for the image
    renderingHelper.SetImageProperty("Image Rendering.Transfer Function", mitk::TransferFunctionProperty::New(transferFucntion));

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
    renderingHelper.PrepareRender();
    int retVal = vtkRegressionTestImage( renderingHelper.GetVtkRenderWindow() );

    //retVal meanings: (see VTK/Rendering/vtkTesting.h)
    //0 = test failed
    //1 = test passed
    //2 = test not run
    //3 = something with vtkInteraction
    MITK_TEST_CONDITION( retVal == 1, "VTK test result positive" );

    MITK_TEST_END();
}
