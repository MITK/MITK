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
#include <mitkNodePredicateDataType.h>

//VTK
#include <vtkRegressionTestImage.h>


int mitkImageVtkMapper2DSwivelTest(int argc, char* argv[])
{
    //load all arguments into a datastorage, take last argument as reference
    //setup a renderwindow of fixed size X*Y
    //render the datastorage
    //compare rendering to reference image
    MITK_TEST_BEGIN("mitkImageVtkMapper2DSwivelTest")

    // enough parameters?
    if ( argc < 2 )
    {
        MITK_TEST_OUTPUT( << "Usage: " << std::string(*argv) << " [file1 file2 ...] outputfile" )
                MITK_TEST_OUTPUT( << "Will render a central axial slice of all given files into outputfile" )
                exit( EXIT_SUCCESS );
    }

    mitkRenderingTestHelper renderingHelper(640, 480, argc, argv);
    //center point for rotation
    mitk::Point3D centerPoint;
    centerPoint.Fill(0.0f);
    //vector for rotating the slice
    mitk::Vector3D rotationVector;
    rotationVector.SetElement(0, 0.2);
    rotationVector.SetElement(1, 0.3);
    rotationVector.SetElement(2, 0.5);
    //sets a swivel direction for the image

    //new version of setting the center point:
    mitk::Image::Pointer image = static_cast<mitk::Image*>(renderingHelper.GetDataStorage()->GetNode(mitk::NodePredicateDataType::New("Image"))->GetData());

    //get the center point of the image
    centerPoint = image->GetGeometry()->GetCenter();

    //rotate the image arround its own center
    renderingHelper.ReorientSlices(centerPoint, rotationVector);
    renderingHelper.Render();

    //use this to generate a reference screenshot or save the file:
    bool generateReferenceScreenshot = false;
    if(generateReferenceScreenshot)
    {
        renderingHelper.SaveAsPNG("/media/hdd/thomasHdd/Pictures/RenderingTestData/pic3dSwivel640x480REF.png");
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

