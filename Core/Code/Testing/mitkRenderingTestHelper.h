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

#ifndef mitkRenderingTestHelper_h
#define mitkRenderingTestHelper_h

#include <vtkSmartPointer.h>
#include <mitkRenderWindow.h>
#include <mitkDataNodeFactory.h>
#include <mitkDataStorage.h>

class vtkRenderWindow;
class vtkRenderer;

class mitkRenderingTestHelper
{

public:
    /** @brief Generate a rendering test helper object including a render window of the size width * height (in pixel).
        @param argc Number of parameters. (here: Images) "Usage: [filename1 filenam2 -V referenceScreenshot (optional -T /directory/to/save/differenceImage)]
        @param argv Given parameters.
    **/
    mitkRenderingTestHelper(int width, int height, int argc, char *argv[]);

    ~mitkRenderingTestHelper();

    /** @brief Getter for the vtkRenderer.
    **/
    vtkRenderer* GetVtkRenderer();

    /** @brief Getter for the vtkRenderWindow which should be used to call vtkRegressionTestImage.
    **/
    vtkRenderWindow* GetVtkRenderWindow();

    /** @brief Method can be used to save a screenshot (e.g. reference screenshot as a .png file.
        @param fileName The filename of the new screenshot (including path).
    **/
    void SaveAsPNG(std::string fileName);

    /** @brief This method set the property of the member datastorage
        @param property Set a property for each image in the datastorage m_DataStorage.
    **/
    void SetProperty(const char *propertyKey, mitk::BaseProperty *property);

    /** @brief Set the view direction of the renderwindow (e.g. sagittal, coronal, axial)
    **/
    void SetViewDirection(mitk::SliceNavigationController::ViewDirection viewDirection);

    /** @brief Reorient the slice (e.g. rotation and translation like the swivel mode).
    **/
    void ReorientSlices(mitk::Point3D origin, mitk::Vector3D rotation);

    /** @brief Render everything into an mitkRenderWindow. Call SetViewDirection() and SetProperty() before this method.
    **/
    void Render();

    /** @brief Returns the datastorage, in order to modify the data inside a rendering test.
    **/
    mitk::DataStorage::Pointer GetDataStorage();
protected:

    /** @brief Prints the opengl information, e.g. version, vendor and extensions,
     *         This function can only be called after an opengl context is active.
     *         It only prints the context after the vtkRenderwindow is fully initialized.
     **/
    void PrintGLInfo();

    /** @brief This method tries to load the given file into a member datastorage, in order to render it.
        @param fileName The filename of the file to be loaded (including path).
    **/
    void AddToStorage(const std::string& filename);

    /** @brief This method tries to parse the given argv for files (e.g. images) and load them into a member datastorage, in order to render it.
        @param argc Number of parameters.
        @param argv Given parameters.
    **/
    void SetInputFileNames(int argc, char *argv[]);

    mitk::RenderWindow::Pointer m_RenderWindow; //<< Contains the mitkRenderWindow into which the test renders the data
    mitk::DataStorage::Pointer m_DataStorage; //<< Contains the mitkDataStorage which contains the data to be rendered

};

#endif

