/*=========================================================================
 
Program:  Medical Imaging & Interaction Toolkit
Language:  C++
Date:   $Date$
Version:  $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/
#include <mitkRawImageFileReader.h>
#include <mitkImage.h>
#include <mitkPixelType.h>

#include <qapplication.h>

void QmitkRawImageFileSequenceOpener::init() 
{
  m_ReadEnable = false;
  m_SizeX = 0;
  m_SizeY = 0;
  m_Pixel = 1;
  m_Endianity = 1;
  m_Dim = 2;
  m_Cancel = false;
}

mitk::Image::Pointer QmitkRawImageFileSequenceOpener::ShowAndTryToRead(QStringList  fileNames)
{
    
  lineEditX->setText("1024");
  lineEditY->setText("1024");
    
  pixelBox->setCurrentItem(0);
  
  endianityBox->setCurrentItem(0);
  
  pixelBox->setFocus();
  this->show();
  
  while(!m_ReadEnable) qApp->processEvents();
 
  // check if Cancel button has been pressed
  if (m_Cancel == true) return NULL;
 
  mitk::RawImageFileReader::Pointer reader=mitk::RawImageFileReader::New();
    
  reader->SetDimensionality(m_Dim);
  reader->SetDimensions(0,m_SizeX);
  reader->SetDimensions(1,m_SizeY);
    
  mitk::PixelType pixelType;
  switch (m_Pixel) 
  {
  case 0:
    reader->SetPixelType(mitk::RawImageFileReader::SCHAR);
    pixelType.Initialize(typeid(signed char),1);
    break;
  case 1:
    reader->SetPixelType(mitk::RawImageFileReader::UCHAR);
    pixelType.Initialize(typeid(unsigned char),1);
    break;
  case 2:
    reader->SetPixelType(mitk::RawImageFileReader::SSHORT);
    pixelType.Initialize(typeid(signed short),1);
    break;
  case 3:
    reader->SetPixelType(mitk::RawImageFileReader::USHORT);
    pixelType.Initialize(typeid(unsigned short),1);
    break;
  case 4:
    reader->SetPixelType(mitk::RawImageFileReader::SINT);
    pixelType.Initialize(typeid(signed int),1);
    break;
  case 5:
    reader->SetPixelType(mitk::RawImageFileReader::UINT);
    pixelType.Initialize(typeid(unsigned int),1);
    break;
  case 6:
    reader->SetPixelType(mitk::RawImageFileReader::DOUBLE);
    pixelType.Initialize(typeid(double),1);
    break;
  case 7:
    reader->SetPixelType(mitk::RawImageFileReader::FLOAT);
    pixelType.Initialize(typeid(float),1);
    break; 
  }
  
  switch  (m_Endianity)
  {
  case 0:
   reader->SetEndianity(mitk::RawImageFileReader::LITTLE);
   break;
  case 1:
   reader->SetEndianity(mitk::RawImageFileReader::BIG);
   break;
  }

  /*
  qDebug("Raw file parameters: %d", m_SizeX);
  qDebug("Value X= %d", m_SizeX);
  qDebug("Value Y= %d", m_SizeY);
  qDebug("Value Z= %d", m_SizeZ);
  qDebug("Pixel type= %d", m_Pixel);
  qDebug("Endianity= %d", m_Endianity);
  qDebug("Dimension= %d", m_Dim);
  */
  
  //////////////////////////
  // loop that reads each slice one-by-one and adds it to the image
  mitk::Image::Pointer output =mitk::Image::New() ;
  unsigned int size[3];
  size[0] = m_SizeX;
  size[1] = m_SizeY;
  size[2] = fileNames.count();
  output->Initialize(pixelType, 3, size,1);
  int slice = 0;
  for ( QStringList::Iterator it = fileNames.begin(); it != fileNames.end(); ++it )
  {
    reader->SetFileName((*it).ascii());
    try
    {
    reader->Update();
    }
    catch(...)
    {
     std::cerr << "Sorry! Could not open file "  << std::endl;
     this->close();
     return NULL;
    }
    output->SetSlice(reader->GetOutput()->GetData(), slice, 0, 0);
    qDebug("SliceNumber= %d", slice);
    slice++;
  }
  this->close();
  return output;
}

void QmitkRawImageFileSequenceOpener::GetParameters()
{
  m_SizeX = lineEditX->text().toInt();
  m_SizeY = lineEditY->text().toInt();
    
  m_Pixel  = pixelBox->currentItem();
  
  m_Endianity = endianityBox->currentItem();
  
  m_ReadEnable = true;
}

void QmitkRawImageFileSequenceOpener::CloseAndStopWaiting()
{
  this->close();
  m_ReadEnable = true;
  m_Cancel = true;
}

