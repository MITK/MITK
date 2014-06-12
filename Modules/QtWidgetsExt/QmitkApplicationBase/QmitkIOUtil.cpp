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
#include <mitkImageWriter.h>

// QT
#include <QFileDialog>
#include <QMessageBox>

//ITK
#include <itksys/SystemTools.hxx>

void QmitkIOUtil::SaveBaseDataWithDialog(mitk::BaseData* data, std::string fileName, QWidget* parent)
{
  try
  {
    if (data != NULL)
    {
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

      mitk::CoreObjectFactory::FileWriterList fileWriters; // = mitk::CoreObjectFactory::GetInstance()->GetFileWriters();

      mitk::CoreObjectFactory::FileWriterList fileWriterCandidates;
      QString fileDialogPattern;
      for (mitk::CoreObjectFactory::FileWriterList::iterator it = fileWriters.begin() ; it != fileWriters.end() ; ++it)
      {
        if ( (*it)->CanWriteBaseDataType(data) )
        {
          fileWriterCandidates.push_back(*it);
          fileDialogPattern += QString::fromStdString((*it)->GetFileDialogPattern()) + ";;";
        }
      }

      if (!fileWriterCandidates.empty())
      {
        // remove last ';;' from pattern
        fileDialogPattern.remove( fileDialogPattern.size()-2, 2);

        // Ensure a valid filename
        QString qFileName(QString::fromStdString(fileName));
        QString qProposedFileName( qFileName );
        QString selectedFilter;
        if(qFileName.isEmpty())
        {
          qProposedFileName.append(fileWriterCandidates.front()->GetDefaultFilename());
        }
        qProposedFileName.append(fileWriterCandidates.front()->GetDefaultExtension());
        qFileName = GetFileNameWithQDialog("Save file", qProposedFileName,
                                           fileDialogPattern, &selectedFilter);
        //do nothing if the user presses cancel
        if ( qFileName.isEmpty() )
          return;

        std::string ext = itksys::SystemTools::GetFilenameLastExtension(qFileName.toStdString());
        QString extension = QString::fromStdString(ext);

        //QString extension = "."+QFileInfo(qFileName).completeSuffix();
        for (mitk::CoreObjectFactory::FileWriterList::iterator it = fileWriterCandidates.begin() ;
             it != fileWriterCandidates.end() ; ++it)
        {
          if ((*it)->IsExtensionValid(extension.toStdString()))
          {
            (*it)->SetFileName( qPrintable(qFileName) );
            (*it)->DoWrite( data );
            return;
          }
        }

        // if the image extension consists of two parts (e.g. *.nii.gz) we need to check again
        // with the two last extensions. This is to allow points within the image name.
        QString qFileNameCopy(qFileName);
        qFileNameCopy.remove(QString::fromStdString(ext));
        std::string ext2 = itksys::SystemTools::GetFilenameLastExtension(qFileNameCopy.toStdString());
        ext2.append(ext);
        extension = QString::fromStdString(ext2);
        for (mitk::CoreObjectFactory::FileWriterList::iterator it = fileWriterCandidates.begin() ;
          it != fileWriterCandidates.end() ; ++it)
        {
          if ((*it)->IsExtensionValid(extension.toStdString()))
          {
            (*it)->SetFileName( qPrintable(qFileName) );
            (*it)->DoWrite( data );
            return;
          }
        }
        // returns earlier when successful
      }

      // no appropriate writer has been found
      QMessageBox::critical(parent,"ERROR","Could not find file writer for this data type");
      return;
    }
  }catch(itk::ExceptionObject e)
  {
      QMessageBox::critical( parent, "Exception during saving", e.GetDescription(),QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
  }
}

void QmitkIOUtil::SaveSurfaceWithDialog(mitk::Surface::Pointer surface, std::string fileName, QWidget* parent)
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

void QmitkIOUtil::SaveImageWithDialog(mitk::Image::Pointer image, std::string fileName, QWidget *parent)
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

  QString qfileName = GetFileNameWithQDialog("Save Image", initialFilename,
                                             mitk::ImageWriter::New()->GetFileDialogPattern(),
                                             &selected_suffix,parent);

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

void QmitkIOUtil::SavePointSetWithDialog(mitk::PointSet::Pointer pointset, std::string fileName, QWidget *parent)
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

QString QmitkIOUtil::GetFileNameWithQDialog(QString caption, QString defaultFilename, QString filter, QString* selectedFilter, QWidget* parent)
{
  QString returnfileName = "";
  static QString lastDirectory = "";
  QString filename = lastDirectory +"/"+ defaultFilename;
  QString selectedFilterInternal = selectedFilter ? *selectedFilter : QString::null;
  returnfileName = QFileDialog::getSaveFileName(parent,caption,filename,filter,&selectedFilterInternal, QFileDialog::DontConfirmOverwrite);
  if (selectedFilter)
  {
    *selectedFilter = selectedFilterInternal;
  }
  if (!returnfileName.isEmpty())
  {
    QFileInfo fileInfo(returnfileName);
    lastDirectory = fileInfo.absolutePath(); // remember path for next save dialog
    if(fileInfo.completeSuffix().isEmpty()) // if no extension has been entered manually into the filename
    {
      // get from combobox selected file extension
      QString selectedExtensions = selectedFilterInternal.split(" (", QString::SkipEmptyParts).back().trimmed();
      selectedExtensions = selectedExtensions.split(")", QString::SkipEmptyParts).front().trimmed();
      QString selectedExtension = selectedExtensions.split(" ", QString::SkipEmptyParts).front();
      if (selectedExtension.size() < 3) // at least something like "*.?"
      {
        QMessageBox::critical(parent, "File extension error", "Unable to deduce a valid file extension.");
        return QString::null;
      }
      selectedExtension = selectedExtension.mid(1);
      returnfileName += selectedExtension; //add it to the path which is returned
    }

    //check if the file exists
    if(fileInfo.exists())
    {
      int answer = QMessageBox::question( parent, "Warning",
                                          QString("File %1 already exists. Overwrite?").arg( returnfileName ),
                                          QMessageBox::Yes,
                                          QMessageBox::No );
      if( answer == QMessageBox::No )
      {
        returnfileName.clear();
      }
    }
  }

  return returnfileName;
}

bool QmitkIOUtil::SaveToFileWriter( mitk::FileWriterWithInformation::Pointer fileWriter, mitk::BaseData::Pointer data,   const std::string fileName )
{
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
  if ( qFileName.isEmpty() )
    return false;

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
      return false;
    }
  }
  //Write only if a valid name was set
  if(qFileName.isEmpty() == false)
  {
    fileWriter->SetFileName( qFileName.toLocal8Bit().constData() );
    fileWriter->DoWrite( data );
  }

  return true;
}

