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
#include <mitkSmartPointerProperty.h>
#include <mitkSurface.h>

//VTK
#include <vtkRegressionTestImage.h>
#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <mitkIOUtil.h>
#include <vtkSphereSource.h>
#include <vtkTextureMapToSphere.h>
#include <vtkCamera.h>


int mitkSurfaceVtkMapper3DTest(int argc, char* argv[])
{
    // load all arguments into a datastorage, take last argument as reference rendering
    // setup a renderwindow of fixed size X*Y
    // render the datastorage
    // compare rendering to reference image
    MITK_TEST_BEGIN("mitkSurfaceVtkMapper3DTest")

    // enough parameters?
    if ( argc < 2 )
    {
        MITK_TEST_OUTPUT( << "Usage: " << std::string(*argv) << " [file1 file2 ...] outputfile" )
                MITK_TEST_OUTPUT( << "Will render a central axial slice of all given files into outputfile" )
                exit( EXIT_SUCCESS );
    }

    mitkRenderingTestHelper renderingHelper(640, 480, argc, argv);

    vtkSmartPointer<vtkFloatArray> textureCoordinates =
            vtkSmartPointer<vtkFloatArray>::New();
    textureCoordinates->SetNumberOfComponents(2);
    textureCoordinates->SetName("TextureCoordinates");

    mitk::Image::Pointer textureImage = static_cast< mitk::Image* > ( renderingHelper.GetDataStorage()->GetNode( mitk::NodePredicateDataType::New("Image"))->GetData() );

    //generate texture coordinates assuming that surface and texture can be mapped 1 to 1
    unsigned int* dims = textureImage->GetDimensions();
    for(unsigned int j = 0; j < dims[1]; ++j)
    {
        for(unsigned int i = 0; i < dims[0]; ++i)
        {
            int pixelID = i + j*dims[0];
            float xNorm = (((float)i)/dims[0]);
            float yNorm = ((float)j)/dims[1];
            textureCoordinates->InsertTuple2(pixelID, xNorm, yNorm);
        }
    }

    mitk::Surface::Pointer surfaceToPutTextureOn = static_cast< mitk::Surface* > ( renderingHelper.GetDataStorage()->GetNode( mitk::NodePredicateDataType::New("Surface"))->GetData() );

    surfaceToPutTextureOn->GetVtkPolyData()->GetPointData()->SetTCoords(textureCoordinates);
    mitk::SmartPointerProperty::Pointer textureProperty = mitk::SmartPointerProperty::New(textureImage);
    renderingHelper.GetDataStorage()->GetNode(mitk::NodePredicateDataType::New("Surface"))->SetProperty("Surface.Texture", textureProperty);

    //remove the image from the data storage in order to not disturb the world geometry
    //(only the surface geometry should be used for rendering)
    renderingHelper.GetDataStorage()->Remove( renderingHelper.GetDataStorage()->GetNode( mitk::NodePredicateDataType::New("Image")) );

    renderingHelper.SetMapperID(mitk::BaseRenderer::Standard3D);
    renderingHelper.Render();

    //Find a nice camera position to view the surface from the front.
    //This has to be done after calling renderingHelper.Render(),
    //because it would overwrite the camera position with global reinit.
    //It is not necessary, but else the surface is ugly rendered from the side.
    mitk::Point3D surfaceCenter= surfaceToPutTextureOn->GetGeometry()->GetCenter();
    vtkCamera* camera3d = renderingHelper.GetVtkRenderer()->GetActiveCamera();
    //1m distance to camera should be a nice default value for most cameras
    camera3d->SetPosition(0,0,-1000);
    camera3d->SetViewUp(0,-1,0);
    camera3d->SetFocalPoint(0,0,surfaceCenter[2]);
    camera3d->SetViewAngle(40);
    camera3d->SetClippingRange(1, 10000);
    renderingHelper.GetVtkRenderer()->ResetCamera();


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
