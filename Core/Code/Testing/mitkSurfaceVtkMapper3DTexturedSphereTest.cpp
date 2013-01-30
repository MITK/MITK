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

/**
 * @brief mitkSurfaceVtkMapper3DTexturedSphereTest This test puts a texture onto a sphere. It is a nice example how to use VTK methods to generate texture coordinates for MITK surfaces.
 * @param argv Just any image serving as texture.
 */

int mitkSurfaceVtkMapper3DTexturedSphereTest(int argc, char* argv[])
{
    // load all arguments into a datastorage, take last argument as reference rendering
    // setup a renderwindow of fixed size X*Y
    // render the datastorage
    // compare rendering to reference image
    MITK_TEST_BEGIN("mitkSurfaceVtkMapper3DTexturedSphereTest")

    // enough parameters?
    if ( argc < 2 )
    {
        MITK_TEST_OUTPUT( << "Usage: " << std::string(*argv) << " [file1 file2 ...] outputfile" )
                MITK_TEST_OUTPUT( << "Will render a central axial slice of all given files into outputfile" )
                exit( EXIT_SUCCESS );
    }

    mitkRenderingTestHelper renderingHelper(640, 480, argc, argv);
    //This is a test for a 3D surface, thus we need to set the mapper ID to 3D
    renderingHelper.SetMapperID(mitk::BaseRenderer::Standard3D);

    //######## Exmaple code begin ########
    //Generate a sphere in order to map texture on it
    vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
    sphere->SetThetaResolution(12);
    sphere->SetPhiResolution(12);
    sphere->SetRadius(50.0); //just to make it huge
    sphere->SetCenter(50,0,0); //just to center the sphere in the screen

    //taken from VTK example: http://www.vtk.org/Wiki/VTK/Examples/Python/Visualization/SphereTexture
    vtkSmartPointer<vtkTextureMapToSphere> mapToSphere = vtkSmartPointer<vtkTextureMapToSphere>::New();
    mapToSphere->SetInputConnection(sphere->GetOutputPort());
    mapToSphere->PreventSeamOn();

    //get the texture image from the helper's data storage
    mitk::Image::Pointer textureImage = static_cast< mitk::Image* > ( renderingHelper.GetDataStorage()->GetNode( mitk::NodePredicateDataType::New("Image"))->GetData() );

    //Generate MITK surface
    mitk::Surface::Pointer surfaceToPutTextureOn = mitk::Surface::New();
    surfaceToPutTextureOn->SetVtkPolyData(static_cast<vtkPolyData*>(mapToSphere->GetOutput()));
    //Generate a node
    mitk::DataNode::Pointer surfaceNode = mitk::DataNode::New();
    surfaceNode->SetData( surfaceToPutTextureOn );
    //Make a Property and add to the node
    mitk::SmartPointerProperty::Pointer textureProperty = mitk::SmartPointerProperty::New(textureImage);
    surfaceNode->SetProperty("Surface.Texture", textureProperty);
    //add to data storage
    renderingHelper.AddNodeToStorage(surfaceNode);
    //######## Exmaple code end ########

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
