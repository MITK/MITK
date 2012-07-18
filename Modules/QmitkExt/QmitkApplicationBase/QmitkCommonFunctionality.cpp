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

#include <qregexp.h>
#include <qmessagebox.h>
#include <qfileinfo.h>
#include <qstring.h>
#include <QDateTime>
#include <string>
#include "mitkPointSetWriter.h"
#include "mitkConfig.h"
#include "mitkCoreObjectFactory.h"
#include "QmitkCommonFunctionality.h"
#include <mitkImageAccessByItk.h>
#include <mitkPicFileReader.h>

#include <mitkIpPic.h>

#include <vtkWindowToImageFilter.h>
#include <vtkRenderWindow.h>
#include <vtkPNGWriter.h>
#include <vtkCellArray.h>
#include <vtkTriangleFilter.h>
#include <QApplication>
#include <QMessageBox>

#include <Poco/Path.h>

void CommonFunctionality::SaveToFileWriter( mitk::FileWriterWithInformation::Pointer fileWriter, mitk::BaseData::Pointer data, const char* aFileName, const char* propFileName)
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
    fileName = GetSaveFileNameStartingInLastDirectory("Save file", proposedName,QString::fromAscii(fileWriter->GetFileDialogPattern()));

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


/**
* Saves the given mitk::BaseData to a file. The user is prompted to
* enter a file name. Currently only mitk::Image, mitk::Surface, mitk::PointSet and
* mitk::VesselGraphData are supported. This function is deprecated
* until the save-problem is solved by means of a Save-Factory or any
* other "nice" mechanism
*/
void CommonFunctionality::SaveBaseData( mitk::BaseData* data, const char * aFileName )
{
  //save initial time
  QDateTime initialTime = QDateTime::currentDateTime();
  std::string fileNameUsed; //file name that was actually used by the writer (e.g. after file open dialog)
  bool writingSuccessful = false;

  try{
    if (data != NULL)
    {
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(data);
      QString classname(data->GetNameOfClass());
      if ( image.IsNotNull() && (classname.compare("Image")==0 || classname.compare("SeedsImage")==0  ) )
      {
        fileNameUsed = CommonFunctionality::SaveImage(image, aFileName, true);
        if(!(fileNameUsed.length()>0)){
          return;
        } else {
          writingSuccessful = true;
        }
      }

      if(!writingSuccessful)
      {
        mitk::PointSet::Pointer pointset = dynamic_cast<mitk::PointSet*>(data);
        if(pointset.IsNotNull())
        {
          std::string fileName;
          if(aFileName == NULL)
            fileName = "PointSet";
          else
            fileName = aFileName;

          
          
          fileName = itksys::SystemTools::GetFilenameWithoutExtension(fileName);
                    
          QString initialFileName = QString::fromStdString(fileName);

          QString selected_suffix("MITK Point-Sets (*.mps)");
          QString possible_suffixes("MITK Point-Sets (*.mps)");
          

          /*QString qfileName = QFileDialog::getSaveFileName( NULL, "Save image", initialFilename ,mitk::CoreObjectFactory::GetInstance()->GetSaveFileExtensions(),&selected_suffix);
          */
          QString qfileName = GetSaveFileNameStartingInLastDirectory("Save file", initialFileName, possible_suffixes, &selected_suffix);
          
          
          MITK_INFO<<qfileName.toLocal8Bit().constData();

          mitk::PointSetWriter::Pointer writer = mitk::PointSetWriter::New();
          std::string extension = itksys::SystemTools::GetFilenameLastExtension( qfileName.toLocal8Bit().constData() );
          if (extension == "") // if no extension has been entered manually into the filename
          {
            // get from combobox selected file extension
            extension = itksys::SystemTools::GetFilenameLastExtension( selected_suffix.toLocal8Bit().constData());
            extension = extension.substr(0, extension.size()-1);
            qfileName += QString::fromStdString(extension);
          }

          MITK_INFO<<"extension: " << extension;
          // check if extension is valid
          if (!writer->IsExtensionValid(extension))
          {
            QString message;
            message.append("File extension not suitable for writing point set data. Choose one extension of this list: ");
            message.append(writer->GetPossibleFileExtensionsAsString().c_str());
            QMessageBox::critical(NULL,"ERROR",message);
            return;
          }

          if (qfileName.isEmpty() == false )
          {
            writer->SetInput( pointset );
            writer->SetFileName( qfileName.toLocal8Bit().constData() );
            writer->Update();
            fileNameUsed = writer->GetFileName();
            writingSuccessful = true;
          } else {
            return;
          }
        }

        if(!writingSuccessful)
        {
          mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>(data);
          if(surface.IsNotNull())
          {
            fileNameUsed = CommonFunctionality::SaveSurface(surface, aFileName);
            if(!(fileNameUsed.length()>0)){
              return;
            } else {
              writingSuccessful = true;
            }
          }

          if(!writingSuccessful)
          {
            // now try the file writers provided by the CoreObjectFactory
            mitk::CoreObjectFactory::FileWriterList fileWriters = mitk::CoreObjectFactory::GetInstance()->GetFileWriters();
            bool writerFound = false;

            for (mitk::CoreObjectFactory::FileWriterList::iterator it = fileWriters.begin() ; it != fileWriters.end() ; ++it) 
            {
              if ( (*it)->CanWriteBaseDataType(data) ) {
                writerFound = true;
                SaveToFileWriter(*it, data, NULL, aFileName); 
                fileNameUsed = (*it)->GetFileName();
                // correct writer has been found->break
                if(!(fileNameUsed.length()>0)){
                  return;
                } else {
                  writingSuccessful = true;
                  break;
                }
              } 
            }
            if(!writerFound)
            {
              // no appropriate writer has been found 
              QMessageBox::critical(NULL,"ERROR","Could not find file writer for this data type"); 
              return;
            }
          }
        }

      }
    } else {
      QMessageBox::critical(NULL,"ERROR","Cannot write data (invalid/empty)"); 
      return;
    }
  } catch(itk::ExceptionObject e)
  {
    QMessageBox::critical( NULL, "SaveDialog", e.GetDescription(),QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
  }

  //writing is assumed to have been successful

  //check if file exists, file size >0 and last modified after this function was called
  try{
    QFileInfo* fileInfo = new QFileInfo(QString(fileNameUsed.c_str()));
    if(!fileInfo->exists())
    {
      QMessageBox::warning(NULL,"WARNING","File was not created or was split into multiple files");
    } else if(fileInfo->size()==0) {
      QMessageBox::warning(NULL,"WARNING","File is empty");
    } else if(fileInfo->lastModified()<initialTime) {
      QMessageBox::warning(NULL,"WARNING","Save not successful. File was not updated (only old version available)");
    }
    delete fileInfo;
  } catch(...) {
    QMessageBox::critical(NULL,"ERROR","Save not successful. Possibly no writing permission.");
  }
}
mitk::DataNode::Pointer CommonFunctionality::FileOpen( const QString& fileName)
{
  return FileOpen( fileName.toLocal8Bit().constData() );
}

mitk::DataNode::Pointer CommonFunctionality::FileOpen( const char * fileName )
{
  mitk::DataNodeFactory::Pointer factory = mitk::DataNodeFactory::New();

  try
  {
    factory->SetFileName( fileName );
    factory->Update();
    return factory->GetOutput( 0 );
  }
  catch ( itk::ExceptionObject & ex )
  {
    itkGenericOutputMacro( << "Exception during file open: " << ex );
    return NULL;
  }
}

mitk::DataNode::Pointer CommonFunctionality::FileOpenImageSequence(const QString& aFileName)
{
  return FileOpenImageSequence( aFileName.toLocal8Bit().constData() );
}

mitk::DataNode::Pointer CommonFunctionality::FileOpenImageSequence(const char* aFileName)
{
  if(aFileName==NULL) return NULL;

  mitk::DataNodeFactory::Pointer factory = mitk::DataNodeFactory::New();

  QString fileName = aFileName;
  if (!fileName.contains("dcm") && !fileName.contains("DCM"))
  {
    int fnstart = fileName.lastIndexOf( QRegExp("[/\\\\]"), fileName.length() );
    if(fnstart<0) fnstart=0;
    int start = fileName.indexOf( QRegExp("[0-9]"), fnstart );
    if(start<0)
    {
      return FileOpen(fileName.toLocal8Bit().constData());;
    }

    char prefix[1024], pattern[1024];

    strncpy(prefix, fileName.toLocal8Bit().constData(), start);
    prefix[start]=0;

    int stop=fileName.indexOf( QRegExp("[^0-9]"), start );
    sprintf(pattern, "%%s%%0%uu%s",stop-start,fileName.toLocal8Bit().constData()+stop);


    factory->SetFilePattern( pattern );
    factory->SetFilePrefix( prefix );
  }
  else
  {
    //    factory->SetFileName( fileName );
    factory->SetFilePattern( fileName.toLocal8Bit().constData() );
    factory->SetFilePrefix( fileName.toLocal8Bit().constData() );
  }
  factory->Update();
  return factory->GetOutput( 0 );

}

mitk::DataNode::Pointer CommonFunctionality::FileOpenImageSequence()
{
  QString fileName = QFileDialog::getOpenFileName(NULL,mitk::CoreObjectFactory::GetInstance()->GetFileExtensions());

  if ( !fileName.isNull() )
  {
    return FileOpenImageSequence(fileName);
  }
  else
  {
    return NULL;
  }
}

mitk::DataNode::Pointer CommonFunctionality::FileOpen()
{
  return CommonFunctionality::FileOpenSpecific( mitk::CoreObjectFactory::GetInstance()->GetFileExtensions() );
}

mitk::DataNode::Pointer CommonFunctionality::FileOpenSpecific( const QString& fileExtensions)
{
  return FileOpenSpecific( fileExtensions.toLocal8Bit().constData() );
}

mitk::DataNode::Pointer CommonFunctionality::FileOpenSpecific( const char *fileExtensions )
{
  QString fileName = QFileDialog::getOpenFileName( NULL, fileExtensions );
  if ( !fileName.isNull() )
  {
    mitk::DataNode::Pointer result = FileOpen(fileName.toLocal8Bit().constData());
    if ( result.IsNull() )
    {
      return FileOpenImageSequence(fileName);
    }
    else
    {
      return result;
    }
  }
  else
  {
    return NULL;
  }
}

mitk::DataNode::Pointer CommonFunctionality::OpenVolumeOrSliceStack()
{
  mitk::DataNode::Pointer newNode = NULL;

  QString fileName = QFileDialog::getOpenFileName(NULL,mitk::CoreObjectFactory::GetInstance()->GetFileExtensions() );
  if ( !fileName.isNull() )
  {
    newNode = CommonFunctionality::FileOpen(fileName);
    if (newNode.IsNotNull())
    {
      mitk::Image::Pointer imageData = dynamic_cast<mitk::Image*> (newNode->GetData()) ;
      if (imageData.IsNull()) return NULL;

      if (imageData->GetDimension(2) == 1)
      {
        //        std::string dir = itksys::SystemTools::GetFilenamePath( std::string(fileName.ascii()) )
        newNode = CommonFunctionality::FileOpenImageSequence(fileName);
        imageData = dynamic_cast<mitk::Image*> (newNode->GetData());
      }
      return newNode;
    }
  }
  {
    return NULL;
  }
}

#include "mitkSurfaceVtkWriter.h"
#include <vtkSTLWriter.h>
#include <vtkPolyDataWriter.h>
#include <vtkXMLPolyDataWriter.h>

std::string CommonFunctionality::SaveSurface(mitk::Surface* surface, const char* aFileName)
{
  std::string fileName;
  if(aFileName == NULL)
    fileName = "Surface";
  else
    fileName = aFileName;

  std::string selectedItemsName = itksys::SystemTools::GetFilenameWithoutExtension(fileName);
  //selectedItemsName += ".stl"
  QString selected_suffix("STL File (*.stl)");
  QString possible_suffixes("STL File (*.stl);; VTK File (*.vtk);; VTP File (*.vtp)");
  QString qfileName = GetSaveFileNameStartingInLastDirectory("Save surface object", QString::fromStdString(selectedItemsName), possible_suffixes,&selected_suffix);

  if (qfileName.isEmpty())
    return "";

  std::string extension = itksys::SystemTools::GetFilenameLastExtension( qfileName.toStdString() );
  if (extension == "") // if no extension has been entered manually into the filename
  {
    // get from combobox selected file extension
    extension = itksys::SystemTools::GetFilenameLastExtension( selected_suffix.toLocal8Bit().constData());
    extension = extension.substr(0, extension.size()-1);
    qfileName += QString::fromStdString(extension);
  }

  if(extension == ".stl" )
  {
    mitk::SurfaceVtkWriter<vtkSTLWriter>::Pointer writer=mitk::SurfaceVtkWriter<vtkSTLWriter>::New();

    // check if surface actually consists of triangles; if not, the writer will not do anything; so, convert to triangles...
    vtkPolyData* polys = surface->GetVtkPolyData();
    if( polys->GetNumberOfStrips() > 0 )
    {
      vtkTriangleFilter* triangleFilter = vtkTriangleFilter::New();
      triangleFilter->SetInput(polys);
      triangleFilter->Update();
      polys = triangleFilter->GetOutput();
      polys->Register(NULL);
      triangleFilter->Delete();
      surface->SetVtkPolyData(polys);
    }

    writer->SetInput( surface );
    writer->SetFileName(qfileName.toLocal8Bit().constData());
    writer->GetVtkWriter()->SetFileTypeToBinary();
    writer->Write();
  }
  else if(extension == ".vtp")
  {
    mitk::SurfaceVtkWriter<vtkXMLPolyDataWriter>::Pointer writer=mitk::SurfaceVtkWriter<vtkXMLPolyDataWriter>::New();
    writer->SetInput( surface );
    writer->SetFileName(qfileName.toLocal8Bit().constData());
    writer->GetVtkWriter()->SetDataModeToBinary();
    writer->Write();
  }
  else if (extension == ".vtk")
  {
    mitk::SurfaceVtkWriter<vtkPolyDataWriter>::Pointer writer=mitk::SurfaceVtkWriter<vtkPolyDataWriter>::New();
    writer->SetInput( surface );
    writer->SetFileName(qfileName.toLocal8Bit().constData());
    writer->Write();
  }
  else
  {
    // file extension not suitable for writing specified data type
    QMessageBox::critical(NULL,"ERROR","File extension not suitable for writing Surface data. Choose .vtk, .stl or .vtp");
    return "";
  }
  return qfileName.toLocal8Bit().constData();
}

#include "mitkImageWriter.h"
#include <itksys/SystemTools.hxx>

std::string CommonFunctionality::SaveImage(mitk::Image* image, const char* aFileName, bool askForDifferentFilename)
{
  QString selected_suffix("Nearly Raw Raster Data (*.nrrd)");
  std::string defaultExtension = ".nrrd";

  std::string fileName;
  if(aFileName == NULL || askForDifferentFilename)
  {
    QString initialFilename(aFileName);
    if (initialFilename.isEmpty()) initialFilename = "NewImage.pic";

    QString qfileName = GetSaveFileNameStartingInLastDirectory("Save image", initialFilename ,mitk::CoreObjectFactory::GetInstance()->GetSaveFileExtensions(),&selected_suffix);
    MITK_INFO<<qfileName.toLocal8Bit().constData();
    if (qfileName.isEmpty() )
      return "";
    fileName = qfileName.toLocal8Bit().constData();
  }
  else
    fileName = aFileName;

  try
  {
    std::string dir = itksys::SystemTools::GetFilenamePath( fileName );
    std::string baseFilename = itksys::SystemTools::GetFilenameWithoutLastExtension( fileName );
    std::string extension = itksys::SystemTools::GetFilenameLastExtension( fileName );

    if( extension == "" )
      extension = defaultExtension;

    if (extension == ".gz")
    {
      QMessageBox::critical( NULL, "SaveDialog", "Warning: You can not save an image in the compressed \n"
        ".pic.gz format. You must save as a normal .pic file.\n"
        "Please press Save again and choose a filename with a .pic ending.",
        QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
      return "";
    }
    // check if extension is suitable for writing image data
    mitk::ImageWriter::Pointer imageWriter = mitk::ImageWriter::New();    
    if (!imageWriter->IsExtensionValid(extension))
    {
      // muellerm, 12-05-02, using default file extension
      // if no valid extension was given, see bug 11799

      MITK_WARN << extension << " extension is unknown. Writing image to file " << fileName
                   << defaultExtension;
      extension = defaultExtension;
      baseFilename = itksys::SystemTools::GetFilenameName( fileName );
      //MITK_INFO << baseFilename;

      /*
      QString message;
      message.append("File extension not suitable for writing image data. Choose one extension of this list: ");
      message.append(imageWriter->GetPossibleFileExtensionsAsString().c_str());
      QMessageBox::critical(NULL,"ERROR",message);
      return "";
      */
    }

    dir += "/";
    dir += baseFilename;

    if( itksys::SystemTools::FileExists( (dir + extension).c_str() ) )
    {
      int answer = QMessageBox::question( QApplication::topLevelWidgets().at(0), "Warning",
                                          QString("File %1 already exists. Overwrite?").arg( QString::fromStdString(dir + extension) ),
                                          QMessageBox::Yes,
                                          QMessageBox::No );
      if( answer == QMessageBox::No )
        return "";
    }


    // Check if Image data/ Geometry information is lost
    if (image->GetDimension() == 2)
    {
       //if image is 2D the transformation matrix is cropped to 2D. Check if information is lost here.
       bool informationIsLost = false;
       while(1)
       {
          MITK_INFO << "spac: " << image->GetGeometry()->GetSpacing();
          if (image->GetGeometry()->GetSpacing()[2] != 1)
          {
             informationIsLost = true;
             break;
          }
          MITK_INFO << "orig: " << image->GetGeometry()->GetOrigin();
          if (image->GetGeometry()->GetOrigin()[2] != 0)
          {
             informationIsLost = true;
             break;
          }
          mitk::Vector3D col0, col1, col2;
          col0.Set_vnl_vector(image->GetGeometry()->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(0));
          col1.Set_vnl_vector(image->GetGeometry()->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(1));
          col2.Set_vnl_vector(image->GetGeometry()->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(2));

          MITK_INFO << "col0: " << col0;
          MITK_INFO << "col1 " <<  col1;
          MITK_INFO << "col2: " << col2;
          if ((col0[2] != 0) || (col1[2] != 0) || (col2[0] != 0) || (col2[1] != 0) || (col2[2] != 1))
          {
             informationIsLost = true;
             break;
          }
          break;
       };

       if (informationIsLost)
       {
          // information will be lost, if continuing to save this file as 2D
          // tell it the user and offer to save it as 3D

          // todo: if format is png, jpg, etc.. forget it, no geometry information at all

          QMessageBox msgBox;
          msgBox.setText("You are trying to save a 2D image that has 3D geometry informations.");
          msgBox.setInformativeText("You will lose the 3D geometry information this way. Do you rather want to save it as a 3D image?");
          msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
          msgBox.setDefaultButton(QMessageBox::Yes);
          int ret = msgBox.exec();

          if (ret == QMessageBox::YesRole)
          {
             // Save as 3D
             // todo: convert image to 3D image
             mitk::Image::Pointer imageAs3D;
             AccessFixedDimensionByItk_2(image,Convert2DImageTo3D,2,image,imageAs3D);
             image = imageAs3D;
          }
          else if (ret == QMessageBox::NoRole)
          {
             // Continue Saving as 2D

          }
          else 
          {
             // Abort, don't save anything
             return 0; 
          }
       }
    }

    // write image
    imageWriter->SetInput(image);
    imageWriter->SetFileName(dir.c_str());
    imageWriter->SetExtension(extension.c_str());
    imageWriter->Write();
    fileName = dir + extension;
  }
  catch ( itk::ExceptionObject &err)
  {
    itkGenericOutputMacro( << "Exception during write: " << err );
    QString exceptionString;
    exceptionString.append("Error during write image: ");
    exceptionString.append(err.GetDescription());
    QMessageBox::critical(NULL,"ERROR",exceptionString);
    return "";
  }
  catch ( ... )
  {
    itkGenericOutputMacro( << "Unknown type of exception during write" );
    QMessageBox::critical(NULL,"ERROR","Error during write image. Possibly no writing permission.");
    fileName = "";
  }
  return fileName;
}


std::string CommonFunctionality::SaveScreenshot( vtkRenderWindow* renderWindow , const char* filename )
{
  //
  // perform some error checking
  //
  if ( ! renderWindow )
  {
    itkGenericOutputMacro( << "render window is NULL!" );
    return std::string("");
  }
  if ( ! renderWindow )
  {
    itkGenericOutputMacro( << "Unsupported type of render window! The only supported type is currently QmitkRenderWindow." );
    return std::string("");
  }

  //
  // create the screenshot before the filechooser is opened,
  // so there the file chooser will not be part of the screenshot
  //
  //QPixmap buffer = QPixmap::grabWindow( qtRenderWindow->winId() );

  // new Version: 
  //// take screenshot of render window without the coloured frame of 2 pixels by cropping the raw image data
  vtkWindowToImageFilter* wti = vtkWindowToImageFilter::New();
  wti->SetInput( renderWindow );
  wti->Update();
  vtkImageData* imageData = wti->GetOutput();
  int framesize = 5;
  int* windowSize = renderWindow->GetSize();
  int numberOfScalarComponents = imageData->GetNumberOfScalarComponents();
  vtkImageData* processedImageData = vtkImageData::New();
  processedImageData->SetNumberOfScalarComponents(numberOfScalarComponents);
  processedImageData->SetExtent(0,windowSize[0]-2*framesize-1,0,windowSize[1]-2*framesize-1,0,0);
  processedImageData->SetScalarTypeToUnsignedChar();
  for (int i=framesize; i<windowSize[0]-framesize; i++)
  {
    for (int j=framesize; j<windowSize[1]-framesize; j++)
    {
      for (int k=0; k<numberOfScalarComponents; k++)
      {
        processedImageData->SetScalarComponentFromDouble(i-framesize,j-framesize,0,k,imageData->GetScalarComponentAsDouble(i,j,0,k));
      }
    }
  }

  // write new image as *.png to file
  vtkPNGWriter* pngWriter = vtkPNGWriter::New();

  //
  // if the provided filename is empty ask the user 
  // for the name of the file in which the screenshot
  // should be saved
  //
  std::string concreteFilename = "";
  if( filename == NULL )
  {
    //
    // show a file selector with the supported file formats
    //
    QString qfileName = GetSaveFileNameStartingInLastDirectory("Save screenshot", QString::fromStdString(""), QString::fromStdString(".png") );
    if ( qfileName.isEmpty() )
      return "";
    concreteFilename = qfileName.toLocal8Bit().constData();
  }
  else
    concreteFilename = filename;

  // make sure the filename ends with .png
  const std::string outFileSuffix("png");
  std::string::size_type pos = concreteFilename.rfind('.');

  if ( pos == std::string::npos )
    concreteFilename = concreteFilename + '.' + outFileSuffix;
  else 
  {
    std::string extname = concreteFilename.substr(pos+1);
    if ( extname.empty() ) concreteFilename += outFileSuffix; // name ended with '.'
    if ( !(extname == outFileSuffix) )
      concreteFilename.replace( pos+1, std::string::npos, "png" );
  }

  //
  // wait for 500 ms to let the file chooser close itself
  //  
  // int msecs = 500;
  // clock_t ticks = ( clock_t )( ( ( ( float ) msecs ) / 1000.0f ) * ( ( float ) CLOCKS_PER_SEC ) );
  // clock_t goal = ticks + std::clock();
  // while ( goal > std::clock() );
  //

  //
  // save the screenshot under the given filename
  //

  pngWriter->SetInput(processedImageData);
  //pngWriter->SetInput( wti->GetOutput() );
  pngWriter->SetFileName( concreteFilename.c_str() );

  pngWriter->Write();

  if ( pngWriter->GetErrorCode() != 0 )
    QMessageBox::information(NULL, "Save Screenshot...", "The file could not be saved. Please check filename, format and access rights...");

  wti->Delete();
  pngWriter->Delete();
  return concreteFilename;  
}

QString CommonFunctionality::GetSaveFileNameStartingInLastDirectory(QString caption, QString defaultFilename, QString filter, QString* selectedFilter)
{
QString returnValue = "";
static QString lastDirectory = "";
QString filename = lastDirectory + defaultFilename;
returnValue = QFileDialog::getSaveFileName(NULL,caption,filename,filter,selectedFilter);
if (returnValue != "")
  {
  std::string dir = itksys::SystemTools::GetFilenamePath( returnValue.toStdString() );
  dir += Poco::Path::separator();
  lastDirectory = dir.c_str(); // remember path for next save dialog
  }
return returnValue;
}
