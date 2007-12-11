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

#include <qapplication.h>

void QmitkRawImageFileOpener::init() 
{
  m_ReadEnable = false;
  m_SizeX = 0;
  m_SizeY = 0;
  m_SizeZ = 0;
  m_Pixel = 1;
  m_Dim = 2;
  m_Endianity = 1;
  m_Cancel = false;
}

mitk::Image::Pointer QmitkRawImageFileOpener::ShowAndTryToRead(const char * fileName)
{
  lineEditX->setText("1024");
  lineEditY->setText("1024");
  lineEditZ->setText("1");
  
  pixelBox->setCurrentItem(0);
  
  endianityBox->setCurrentItem(0);
  
  dimensionBox->setCurrentItem(0); 
  pixelBox->setFocus();
  this->show();
  
  while(!m_ReadEnable) qApp->processEvents();
 
  // check if Cancel button has been pressed
  if (m_Cancel == true) return NULL;
 
  mitk::RawImageFileReader::Pointer reader=mitk::RawImageFileReader::New();
  
  reader->SetFileName(fileName);
  reader->SetDimensionality(m_Dim);
  reader->SetDimensions(0,m_SizeX);
  reader->SetDimensions(1,m_SizeY);
  if (m_Dim == 3)
  reader->SetDimensions(2,m_SizeZ);
  
  
  switch (m_Pixel) 
  {
  case 0:
    reader->SetPixelType(mitk::RawImageFileReader::SSHORT);
    break;
  case 1:
    reader->SetPixelType(mitk::RawImageFileReader::USHORT);
    break;
  case 2:
    reader->SetPixelType(mitk::RawImageFileReader::SINT);
    break;
  case 3:
    reader->SetPixelType(mitk::RawImageFileReader::UINT);
    break;
  case 4:
    reader->SetPixelType(mitk::RawImageFileReader::DOUBLE);
    break;
  case 5:
    reader->SetPixelType(mitk::RawImageFileReader::FLOAT);
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

  try
  {
  reader->Update();
  }
  catch(...)
  {
    std::cerr << "Sorry! Could not open file " << fileName << std::endl;
   this->close();
   return NULL;
  }
  //fprintf( stderr, "Theoretically read:%s \n\n", fileName );
  this->close();
  mitk::Image::Pointer output = reader->GetOutput(); 
  return output;
}

void QmitkRawImageFileOpener::GetParameters()
{
  m_SizeX = lineEditX->text().toInt();
  m_SizeY = lineEditY->text().toInt();
  m_SizeZ = lineEditZ->text().toInt();
  
  m_Pixel  = pixelBox->currentItem();
  
  m_Endianity = endianityBox->currentItem();
  
  m_Dim = dimensionBox->currentItem()+2;  
  
  m_ReadEnable = true;
}

void QmitkRawImageFileOpener::CloseAndStopWaiting()
{
  this->close();
  m_ReadEnable = true;
  m_Cancel = true;
}

