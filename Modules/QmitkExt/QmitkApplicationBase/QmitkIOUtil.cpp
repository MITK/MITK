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

#include "QmitkIOUtil.h"
#include <mitkIOUtil.h>
#include <mitkCoreObjectFactory.h>

// QT
#include <QFileDialog>
#include <QMessageBox>

//ITK
#include <itksys/SystemTools.hxx>

#include <Poco/Path.h>

void mitk::QmitkIOUtil::SaveBaseDataWithDialog(mitk::BaseData* data, std::string path)
{
    try
    {
        if (data != NULL)
        {
            //########### Check if we can save as standard image type via itkImageWriter
            mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(data);
            QString classname(data->GetNameOfClass());
            if ( image.IsNotNull() && (classname.compare("Image")==0 || classname.compare("SeedsImage")==0  ) )
            {
                SaveImageWithDialog(image, path);
                return; //succes. we can return
            }
            //########### End Check if we can save as standard image type via itkImageWriter

            //########### Check if we can save as standard pointset type via mitkPointSetWriter
            mitk::PointSet::Pointer pointset = dynamic_cast<mitk::PointSet*>(data);
            if(pointset.IsNotNull())
            {
                SavePointSetWithDialog(pointset, path);
                return; //succes. we can return
            }
            //########### End Check if we can save as standard pointset type via mitkPointSetWriter

            //########### Check if we can save as standard surface type via mitkVtkSurfaceWriter
            mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>(data);
            if(surface.IsNotNull())
            {
                SaveSurfaceWithDialog(surface, path);
                return; //succes. we can return
            }
            //########### End Check if we can save as standard surface type via mitkVtkSurfaceWriter

            //########### None standard data type was found, try to save with extensions.
            // now try the file writers provided by the CoreObjectFactory
            mitk::CoreObjectFactory::FileWriterList fileWriters = mitk::CoreObjectFactory::GetInstance()->GetFileWriters();
            bool writerFound = false;

            for (mitk::CoreObjectFactory::FileWriterList::iterator it = fileWriters.begin() ; it != fileWriters.end() ; ++it)
            {
                if ( (*it)->CanWriteBaseDataType(data) )
                {
                    writerFound = true;
                    SaveToFileWriter(*it, data, NULL, path.c_str());
//                    std::string tmpFileName = (*it)->GetFileName();
                    return;
                    // correct writer has been found->break
//                    if(!(tmpFileName.length()>0)){
//                        return;
//                    } else {
//                        writingSuccessful = true;
//                        break;
//                    }
                }
            }
            if(!writerFound)
            {
                // no appropriate writer has been found
                QMessageBox::critical(NULL,"ERROR","Could not find file writer for this data type");
                return;
            }

        }
    }catch(itk::ExceptionObject e)
    {
        QMessageBox::critical( NULL, "Exception during saving", e.GetDescription(),QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
    }
}

void mitk::QmitkIOUtil::SaveSurfaceWithDialog(mitk::Surface::Pointer surface, std::string fileName)
{
    //default selected suffix for surfaces
    QString selected_suffix("STL File (*.stl)");
    QString possible_suffixes("STL File (*.stl);; VTK File (*.vtk);; VTP File (*.vtp)");
    //default initial file name
    QString initialFilename("NewSurface");
    //default image extension
    initialFilename.append((mitk::IOUtil::DEFAULTSURFACEEXTENSION).c_str());

    //if any filename is supplied by the user, use it
    if( !fileName.empty() )
    {
        initialFilename = fileName.c_str();
    }

    QString qfileName = GetFileNameWithQDialog("Save Surface", initialFilename , possible_suffixes, &selected_suffix);

    //do nothing if the user presses cancel
    if ( qfileName.isEmpty() ) return;
    try{
        mitk::IOUtil::SaveSurface(surface,qfileName.toLocal8Bit().constData());
    }
    catch(mitk::Exception &e)
    {
        MITK_ERROR << "error saving file: " << e.what();

        std::string msg("Could not save surface.\n Error: ");
        msg.append(e.what());

        QMessageBox::critical( NULL, "Exception during saving", msg.c_str(),
                               QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
    }
}

void mitk::QmitkIOUtil::SaveImageWithDialog(mitk::Image::Pointer image, std::string fileName)
{
    //default selected suffix for images
    QString selected_suffix("Nearly Raw Raster Data (*.nrrd)");
    //default initial file name
    QString initialFilename("NewImage");
    //default image extension
    initialFilename.append((mitk::IOUtil::DEFAULTIMAGEEXTENSION).c_str());

    //if any filename is supplied by the user, use it
    if( !fileName.empty() )
    {
        initialFilename = fileName.c_str();
    }

    QString qfileName = GetFileNameWithQDialog("Save Image", initialFilename ,mitk::CoreObjectFactory::GetInstance()->GetSaveFileExtensions(),&selected_suffix);

    //do nothing if the user presses cancel
    if ( qfileName.isEmpty() ) return;

    try
    {
        mitk::IOUtil::SaveImage(image, qfileName.toLocal8Bit().constData());
    }
    catch(mitk::Exception &e)
    {
        MITK_ERROR << "error saving file: " << e.what();

        std::string msg("Could not save image.\n Error: ");
        msg.append(e.what());

        QMessageBox::critical( NULL, "Exception during saving", msg.c_str(),
                               QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
    }
}
void mitk::QmitkIOUtil::SavePointSetWithDialog(mitk::PointSet::Pointer pointset, std::string fileName)
{
    //default selected suffix for point sets
    QString selected_suffix("MITK Point-Sets (*.mps)");
    QString possible_suffixes("MITK Point-Sets (*.mps)");
    //default initial file name
    QString initialFilename("NewPointSet");
    //default image extension
    initialFilename.append((mitk::IOUtil::DEFAULTPOINTSETEXTENSION).c_str());

    //if any filename is supplied by the user, use it
    if( !fileName.empty() )
    {
        initialFilename = fileName.c_str();
    }

    QString qfileName = GetFileNameWithQDialog("Save PointSet", initialFilename , possible_suffixes, &selected_suffix);

    //do nothing if the user presses cancel
    if ( qfileName.isEmpty() ) return;
    try{
        mitk::IOUtil::SavePointSet(pointset,qfileName.toLocal8Bit().constData());
    }
    catch(mitk::Exception &e)
    {
        MITK_ERROR << "error saving file: " << e.what();

        std::string msg("Could not save pointset.\n Error: ");
        msg.append(e.what());

        QMessageBox::critical( NULL, "Exception during saving", msg.c_str(),
                               QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
    }
}

QString mitk::QmitkIOUtil::GetFileNameWithQDialog(QString caption, QString defaultFilename, QString filter, QString* selectedFilter)
{
    QString returnfileName = "";
    static QString lastDirectory = "";
    QString filename = lastDirectory + defaultFilename;
    returnfileName = QFileDialog::getSaveFileName(NULL,caption,filename,filter,selectedFilter);
    if (returnfileName != "")
    {
        std::string dir = itksys::SystemTools::GetFilenamePath( returnfileName.toStdString() );
        dir += Poco::Path::separator();
        lastDirectory = dir.c_str(); // remember path for next save dialog
    }
    std::string extension = itksys::SystemTools::GetFilenameLastExtension( returnfileName.toStdString() );
    if (extension == "") // if no extension has been entered manually into the filename
    {
      // get from combobox selected file extension
      extension = itksys::SystemTools::GetFilenameLastExtension( selectedFilter->toLocal8Bit().constData());
      extension = extension.substr(0, extension.size()-1); //remove the last char ")" from the extension
      returnfileName += QString::fromStdString(extension); //add it to the path which is returned
    }
    return returnfileName;
}

void mitk::QmitkIOUtil::SaveToFileWriter( mitk::FileWriterWithInformation::Pointer fileWriter, mitk::BaseData::Pointer data, const char* aFileName, const char* propFileName)
{
    // Check if desired format is supported
    if (! fileWriter->CanWriteBaseDataType(data) ) {
        QMessageBox::critical(NULL,"ERROR","Could not write file. Invalid data type for file writer.");
        return;
    }

    // Ensure a valid filename
    QString fileName;
    if (aFileName == NULL)
    {
        QString proposedName("");
        if(propFileName == NULL)
        {
            proposedName.append(fileWriter->GetDefaultFilename());
        }
        else
        {
            proposedName.append(propFileName).append(fileWriter->GetDefaultExtension());
        }
        fileName = GetFileNameWithQDialog("Save file", proposedName,QString::fromAscii(fileWriter->GetFileDialogPattern()));

        // Check if an extension exists already and if not, append the default extension
        if ( !fileName.contains( QRegExp("\\.\\w+$") ) )
        {
            fileName.append( fileWriter->GetDefaultExtension() );
        }
        else
        {
            std::string extension = itksys::SystemTools::GetFilenameLastExtension( fileName.toLocal8Bit().constData() );
            if (!fileWriter->IsExtensionValid(extension))
            {
                QString message;
                message.append("File extension not suitable for writing given data. Choose one extension of this list: ");
                message.append(fileWriter->GetPossibleFileExtensionsAsString().c_str());
                QMessageBox::critical(NULL,"ERROR",message);
                return;
            }
        }
    }
    else
        fileName = aFileName;


    // Write
    if (fileName.isEmpty() == false )
    {
        fileWriter->SetFileName( fileName.toLocal8Bit().constData() );
        fileWriter->DoWrite( data );
    }
}
