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


#include "mitkIOUtil.h"
#include "mitkDataNodeFactory.h"
#include "mitkImageWriter.h"
#include "mitkPointSetWriter.h"
#include "mitkSurfaceVtkWriter.h"

#include <mitkGetModuleContext.h>
#include <mitkModuleContext.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkIDataNodeReader.h>
#include <mitkProgressBar.h>

//ITK
#include <itksys/SystemTools.hxx>

//VTK
#include <vtkPolyData.h>
#include <vtkTriangleFilter.h>
#include <vtkSmartPointer.h>


namespace mitk {

const std::string IOUtil::DEFAULTIMAGEEXTENSION = ".nrrd";
const std::string IOUtil::DEFAULTSURFACEEXTENSION = ".stl";
const std::string IOUtil::DEFAULTPOINTSETEXTENSION = ".mps";

int IOUtil::LoadFiles(const std::vector<std::string> &fileNames, DataStorage &ds)
{
    // Get the set of registered mitk::IDataNodeReader services
    ModuleContext* context = GetModuleContext();
    const std::list<ServiceReference> refs = context->GetServiceReferences<IDataNodeReader>();
    std::vector<IDataNodeReader*> services;
    services.reserve(refs.size());
    for (std::list<ServiceReference>::const_iterator i = refs.begin();
         i != refs.end(); ++i)
    {
        IDataNodeReader* s = context->GetService<IDataNodeReader>(*i);
        if (s != 0)
        {
            services.push_back(s);
        }
    }

    mitk::ProgressBar::GetInstance()->AddStepsToDo(2*fileNames.size());

    // Iterate over all file names and use the IDataNodeReader services
    // to load them.
    int nodesRead = 0;
    for (std::vector<std::string>::const_iterator i = fileNames.begin();
         i != fileNames.end(); ++i)
    {
        for (std::vector<IDataNodeReader*>::const_iterator readerIt = services.begin();
             readerIt != services.end(); ++readerIt)
        {
            try
            {
                int n = (*readerIt)->Read(*i, ds);
                nodesRead += n;
                if (n > 0) break;
            }
            catch (const std::exception& e)
            {
                MITK_WARN << e.what();
            }
        }
        mitk::ProgressBar::GetInstance()->Progress(2);
    }

    for (std::list<ServiceReference>::const_iterator i = refs.begin();
         i != refs.end(); ++i)
    {
        context->UngetService(*i);
    }

    return nodesRead;
}

DataStorage::Pointer IOUtil::LoadFiles(const std::vector<std::string>& fileNames)
{
    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
    LoadFiles(fileNames, *ds);
    return ds.GetPointer();
}

DataNode::Pointer IOUtil::LoadDataNode(const std::string path)
{
    mitk::DataNodeFactory::Pointer reader = mitk::DataNodeFactory::New();
    try
    {
        reader->SetFileName( path );
        reader->Update();

        if((reader->GetNumberOfOutputs()<1))
        {
            MITK_ERROR << "Could not find data '" << path << "'";
            mitkThrow() << "An exception occured during loading the file " << path << ". Exception says could not find data.";
        }

        mitk::DataNode::Pointer node = reader->GetOutput(0);

        if(node.IsNull())
        {
            MITK_ERROR << "Could not find path: '" << path << "'" << " datanode is NULL" ;
            mitkThrow() << "An exception occured during loading the file " << path << ". Exception says datanode is NULL.";
        }

        return reader->GetOutput( 0 );
     }
    catch ( itk::ExceptionObject & e )
    {
        MITK_ERROR << "Exception occured during load data of '" << path << "': Exception: " << e.what();
        mitkThrow() << "An exception occured during loading the file " << path << ". Exception says: " << e.what();
    }
}

Image::Pointer IOUtil::LoadImage(const std::string path)
{
    mitk::DataNode::Pointer node = LoadDataNode(path);
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
    if(image.IsNull())
    {
        MITK_ERROR << "Image is NULL '" << path << "'";
        mitkThrow() << "An exception occured during loading the image " << path << ". Exception says: Image is NULL.";
    }
    return image;
}

Surface::Pointer IOUtil::LoadSurface(const std::string path)
{
    mitk::DataNode::Pointer node = LoadDataNode(path);
    mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>(node->GetData());
    if(surface.IsNull())
    {
        MITK_ERROR << "Surface is NULL '" << path << "'";
        mitkThrow() << "An exception occured during loading the file " << path << ". Exception says: Surface is NULL.";
    }
    return surface;
}

PointSet::Pointer IOUtil::LoadPointSet(const std::string path)
{
    mitk::DataNode::Pointer node = LoadDataNode(path);
    mitk::PointSet::Pointer pointset = dynamic_cast<mitk::PointSet*>(node->GetData());
    if(pointset.IsNull())
    {
        MITK_ERROR << "PointSet is NULL '" << path << "'";
        mitkThrow() << "An exception occured during loading the file " << path << ". Exception says: Pointset is NULL.";
    }
    return pointset;
}

bool IOUtil::SaveImage(mitk::Image::Pointer image, const std::string path)
{
    std::string dir = itksys::SystemTools::GetFilenamePath( path );
    std::string baseFilename = itksys::SystemTools::GetFilenameWithoutLastExtension( path );
    std::string extension = itksys::SystemTools::GetFilenameLastExtension( path );
    std::string finalFileName = dir + "/" + baseFilename;

    mitk::ImageWriter::Pointer imageWriter = mitk::ImageWriter::New();
    //check if an extension is given, else use the defaul extension
    if( extension == "" )
    {
        MITK_WARN << extension << " extension is not set. Extension set to default: " << finalFileName
                  << DEFAULTIMAGEEXTENSION;
        extension = DEFAULTIMAGEEXTENSION;
    }

    // check if extension is suitable for writing image data
    if (!imageWriter->IsExtensionValid(extension))
    {
        MITK_WARN << extension << " extension is unknown. Extension set to default: " << finalFileName
                  << DEFAULTIMAGEEXTENSION;
        extension = DEFAULTIMAGEEXTENSION;
    }

    try
    {
        //write the data
        imageWriter->SetInput(image);
        imageWriter->SetFileName(finalFileName.c_str());
        imageWriter->SetExtension(extension.c_str());
        imageWriter->Write();
    }
    catch ( std::exception& e )
    {
        MITK_ERROR << " during attempt to write '" << finalFileName + extension << "' Exception says:";
        MITK_ERROR << e.what();
        mitkThrow() << "An exception occured during writing the file " << finalFileName << ". Exception says " << e.what();
    }
    return true;
}

bool IOUtil::SaveSurface(Surface::Pointer surface, const std::string path)
{
    std::string dir = itksys::SystemTools::GetFilenamePath( path );
    std::string baseFilename = itksys::SystemTools::GetFilenameWithoutLastExtension( path );
    std::string extension = itksys::SystemTools::GetFilenameLastExtension( path );
    std::string finalFileName = dir + "/" + baseFilename;

    if (extension == "") // if no extension has been set we use the default extension
    {
        MITK_WARN << extension << " extension is not set. Extension set to default: " << finalFileName
                  << DEFAULTSURFACEEXTENSION;
        extension = DEFAULTSURFACEEXTENSION;
    }
    try
    {
        finalFileName += extension;
        if(extension == ".stl" )
        {
            mitk::SurfaceVtkWriter<vtkSTLWriter>::Pointer surfaceWriter = mitk::SurfaceVtkWriter<vtkSTLWriter>::New();

            // check if surface actually consists of triangles; if not, the writer will not do anything; so, convert to triangles...
            vtkPolyData* polys = surface->GetVtkPolyData();
            if( polys->GetNumberOfStrips() > 0 )
            {
                vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
                triangleFilter->SetInput(polys);
                triangleFilter->Update();
                polys = triangleFilter->GetOutput();
                polys->Register(NULL);
                surface->SetVtkPolyData(polys);
            }
            surfaceWriter->SetInput( surface );
            surfaceWriter->SetFileName( finalFileName.c_str() );
            surfaceWriter->GetVtkWriter()->SetFileTypeToBinary();
            surfaceWriter->Write();
        }
        else if(extension == ".vtp")
        {
            mitk::SurfaceVtkWriter<vtkXMLPolyDataWriter>::Pointer surfaceWriter = mitk::SurfaceVtkWriter<vtkXMLPolyDataWriter>::New();
            surfaceWriter->SetInput( surface );
            surfaceWriter->SetFileName( finalFileName.c_str() );
            surfaceWriter->GetVtkWriter()->SetDataModeToBinary();
            surfaceWriter->Write();
        }
        else if(extension == ".vtk")
        {
            mitk::SurfaceVtkWriter<vtkPolyDataWriter>::Pointer surfaceWriter = mitk::SurfaceVtkWriter<vtkPolyDataWriter>::New();
            surfaceWriter->SetInput( surface );
            surfaceWriter->SetFileName( finalFileName.c_str() );
            surfaceWriter->Write();
        }
        else
        {
            // file extension not suitable for writing specified data type
            MITK_ERROR << "File extension is not suitable for writing'" << finalFileName;
            mitkThrow() << "An exception occured during writing the file " << finalFileName <<
                           ". File extension " << extension << " is not suitable for writing.";
        }
    }
    catch(std::exception& e)
    {
        MITK_ERROR << " during attempt to write '" << finalFileName << "' Exception says:";
        MITK_ERROR << e.what();
        mitkThrow() << "An exception occured during writing the file " << finalFileName << ". Exception says " << e.what();
    }
    return true;
}

bool IOUtil::SavePointSet(PointSet::Pointer pointset, const std::string path)
{
    mitk::PointSetWriter::Pointer pointSetWriter = mitk::PointSetWriter::New();

    std::string dir = itksys::SystemTools::GetFilenamePath( path );
    std::string baseFilename = itksys::SystemTools::GetFilenameWithoutLastExtension( path );
    std::string extension = itksys::SystemTools::GetFilenameLastExtension( path );
    std::string finalFileName = dir + "/" + baseFilename;

    if (extension == "") // if no extension has been entered manually into the filename
    {
        MITK_WARN << extension << " extension is not set. Extension set to default: " << finalFileName
                  << DEFAULTPOINTSETEXTENSION;
        extension = DEFAULTPOINTSETEXTENSION;
    }

    // check if extension is valid
    if (!pointSetWriter->IsExtensionValid(extension))
    {
        MITK_WARN << extension << " extension is unknown. Extension set to default: " << finalFileName
                  << DEFAULTPOINTSETEXTENSION;
        extension = DEFAULTPOINTSETEXTENSION;
    }
    try
    {
        pointSetWriter->SetInput( pointset );
        finalFileName += extension;
        pointSetWriter->SetFileName( finalFileName.c_str() );
        pointSetWriter->Update();
    }
    catch( std::exception& e )
    {
        MITK_ERROR << " during attempt to write '" << finalFileName << "' Exception says:";
        MITK_ERROR << e.what();
        mitkThrow() << "An exception occured during writing the file " << finalFileName << ". Exception says " << e.what();
    }
    return true;
}
}
