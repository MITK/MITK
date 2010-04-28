/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKDICOMSERIESREADER_TXX_
#define MITKDICOMSERIESREADER_TXX_

#include <mitkDicomSeriesReader.h>

#include <itkImageSeriesReader.h>

namespace mitk
{

template <typename PixelType>
void DicomSeriesReader::LoadDicom(const StringContainer &filenames, DataNode &node)
{
  typedef itk::Image<PixelType, 3> ImageType;
  typedef itk::ImageSeriesReader<ImageType> ReaderType;

  DcmIoType::Pointer io = DcmIoType::New();
  typename ReaderType::Pointer reader = ReaderType::New();

  reader->SetFileNames(filenames);
  reader->SetImageIO(io);
  reader->ReverseOrderOff();
  reader->Update();

  mitk::Image::Pointer image = mitk::Image::New();

  image->InitializeByItk(reader->GetOutput());
  image->SetImportVolume(reader->GetOutput()->GetBufferPointer());
  node.SetData(image);
}

}

#endif
