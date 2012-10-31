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

void mitk::QmitkIOUtil::SaveBaseDataWithDialog(mitk::BaseData* data, std::string fileName, QWidget* parent)
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
                SaveImageWithDialog(image, fileName, parent);
                return; //succes. we can return
            }
            //########### End Check if we can save as standard image type via itkImageWriter

            //########### Check if we can save as standard pointset type via mitkPointSetWriter
            mitk::PointSet::Pointer pointset = dynamic_cast<mitk::PointSet*>(data);
            if(pointset.IsNotNull())
            {
                SavePointSetWithDialog(pointset, fileName, parent);
                return; //succes. we can return
            }
            //########### End Check if we can save as standard pointset type via mitkPointSetWriter

            //########### Check if we can save as standard surface type via mitkVtkSurfaceWriter
            mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>(data);
            if(surface.IsNotNull())
            {
                SaveSurfaceWithDialog(surface, fileName, parent);
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
                    SaveToFileWriter(*it, data, fileName.c_str());
                    //TODO: testen ob das so bleiben kann:
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
                QMessageBox::critical(parent,"ERROR","Could not find file writer for this data type");
                return;
            }

        }
    }catch(itk::ExceptionObject e)
    {
        QMessageBox::critical( parent, "Exception during saving", e.GetDescription(),QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
    }
}

void mitk::QmitkIOUtil::SaveSurfaceWithDialog(mitk::Surface::Pointer surface, std::string fileName, QWidget* parent)
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

    QString qfileName = GetFileNameWithQDialog("Save Surface", initialFilename , possible_suffixes, &selected_suffix, parent);

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

void mitk::QmitkIOUtil::SaveImageWithDialog(mitk::Image::Pointer image, std::string fileName, QWidget *parent)
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

    QString qfileName = GetFileNameWithQDialog("Save Image", initialFilename ,mitk::CoreObjectFactory::GetInstance()->GetSaveFileExtensions(),&selected_suffix,parent);

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
void mitk::QmitkIOUtil::SavePointSetWithDialog(mitk::PointSet::Pointer pointset, std::string fileName, QWidget *parent)
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

    QString qfileName = GetFileNameWithQDialog("Save PointSet", initialFilename , possible_suffixes, &selected_suffix, parent);

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

QString mitk::QmitkIOUtil::GetFileNameWithQDialog(QString caption, QString defaultFilename, QString filter, QString* selectedFilter, QWidget* parent)
{
    QString returnfileName = "";
    static QString lastDirectory = "";
    QString filename = lastDirectory + defaultFilename;
    returnfileName = QFileDialog::getSaveFileName(parent,caption,filename,filter,selectedFilter, QFileDialog::DontConfirmOverwrite);
    if (!returnfileName.isEmpty())
    {
        std::string dir = itksys::SystemTools::GetFilenamePath( returnfileName.toStdString() );
        dir += Poco::Path::separator();
        lastDirectory = dir.c_str(); // remember path for next save dialog
        std::string extension = itksys::SystemTools::GetFilenameLastExtension( returnfileName.toStdString() );
        if(extension.empty()) // if no extension has been entered manually into the filename
        {
            // get from combobox selected file extension
            extension = itksys::SystemTools::GetFilenameLastExtension( selectedFilter->toLocal8Bit().constData());
            extension = extension.substr(0, extension.size()-1); //remove the last char ")" from the extension
            returnfileName += QString::fromStdString(extension); //add it to the path which is returned
        }
    }

    //check if the file exists
    if( itksys::SystemTools::FileExists( returnfileName.toLocal8Bit().constData() ))
    {
        int answer = QMessageBox::question( parent, "Warning",
                                            QString("File %1 already exists. Overwrite?").arg( returnfileName ),
                                            QMessageBox::Yes,
                                            QMessageBox::No );
        if( answer == QMessageBox::No )
        {
            //if the user doesn't want to overwrite the file, return an emtpy string.
            return QString("");
        }
    }
    return returnfileName;
}

void mitk::QmitkIOUtil::SaveToFileWriter( mitk::FileWriterWithInformation::Pointer fileWriter, mitk::BaseData::Pointer data,   const std::string fileName )
{
    if (! fileWriter->CanWriteBaseDataType(data) )
    {
        QMessageBox::critical(NULL,"ERROR","Could not write file. Invalid data type for file writer.");
        return;
    }

    // Ensure a valid filename
    QString qFileName(QString::fromStdString(fileName));
    QString qProposedFileName(qFileName);
    if(qFileName.isEmpty())
    {
        qProposedFileName.append(fileWriter->GetDefaultFilename());
    }
    qProposedFileName.append(fileWriter->GetDefaultExtension());
    qFileName = GetFileNameWithQDialog("Save file", qProposedFileName,
                                       QString::fromAscii(fileWriter->GetFileDialogPattern()));
    //do nothing if the user presses cancel
    if ( qFileName.isEmpty() ) return;

    // Check if an extension exists already and if not, append the default extension
    if ( !qFileName.contains( QRegExp("\\.\\w+$") ) )
    {
        qFileName.append( fileWriter->GetDefaultExtension() );
    }
    else
    {
        std::string extension = itksys::SystemTools::GetFilenameLastExtension( qFileName.toLocal8Bit().constData() );
        if (!fileWriter->IsExtensionValid(extension))
        {
            QString message;
            message.append("File extension not suitable for writing given data. Choose one extension of this list: ");
            message.append(fileWriter->GetPossibleFileExtensionsAsString().c_str());
            QMessageBox::critical(NULL,"ERROR",message);
            return;
        }
    }
    //Write only if a valid name was set
    if(qFileName.isEmpty() == false)
    {
        fileWriter->SetFileName( qFileName.toLocal8Bit().constData() );
        fileWriter->DoWrite( data );
    }
}
