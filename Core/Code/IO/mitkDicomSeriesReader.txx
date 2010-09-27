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

#if GDCM_MAJOR_VERSION >= 2
  #include <gdcmSorter.h>
  #include <gdcmScanner.h>
#endif

namespace mitk
{

template <typename PixelType>
void DicomSeriesReader::LoadDicom(const StringContainer &filenames, DataNode &node, UpdateCallBackMethod callback)
{
  const char* previousCLocale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
  std::locale previousCppLocale( std::cin.getloc() );
  std::locale l( "C" );
  std::cin.imbue(l);

  try
  {
    mitk::Image::Pointer image = mitk::Image::New();
    CallbackCommand *command = callback ? new CallbackCommand(callback) : 0;

#if GDCM_MAJOR_VERSION >= 2

    /******** Workaround for 4D data ***************/
    gdcm::Sorter sorter;

    sorter.SetSortFunction(DicomSeriesReader::GdcmSortFunction);
    sorter.Sort(filenames);

    gdcm::Tag acq_time(0x0008,0x0032);
    gdcm::Scanner scanner;

    scanner.AddTag(acq_time);
    scanner.Scan(sorter.GetFilenames());

    std::list<StringContainer> decomposed_filenames;
    const StringContainer::const_iterator f_end = sorter.GetFilenames().end();
    const char *act_value = scanner.GetValue(sorter.GetFilenames().front().c_str(), acq_time);
    unsigned int volume_count = 1u;

    decomposed_filenames.push_back(StringContainer());
    decomposed_filenames.back().push_back(sorter.GetFilenames().front());

    for (StringContainer::const_iterator f_it = ++sorter.GetFilenames().begin(); f_it != f_end; ++f_it)
    {
      const char *value = scanner.GetValue(f_it->c_str(), acq_time);

      if (strcmp(act_value, value))
      {
        act_value = value;
        decomposed_filenames.push_back(StringContainer());
        ++volume_count;
      }

      decomposed_filenames.back().push_back(*f_it);
    }
    /******** End: Workaround for 4D data **********/

    if (volume_count > 1)
    {
      typedef itk::Image<PixelType, 4> ImageType;
      typedef itk::ImageSeriesReader<ImageType> ReaderType;

      DcmIoType::Pointer io = DcmIoType::New();
      typename ReaderType::Pointer reader = ReaderType::New();

      reader->SetImageIO(io);
      reader->ReverseOrderOff();

      if (command)
      {
        reader->AddObserver(itk::ProgressEvent(), command);
      }

      const std::list<StringContainer>::const_iterator df_end = decomposed_filenames.end();
      unsigned int act_volume = 1u;

      reader->SetFileNames(decomposed_filenames.front());
      reader->Update();
      image->InitializeByItk(reader->GetOutput(), 1, volume_count);
      image->SetImportVolume(reader->GetOutput()->GetBufferPointer(), 0u);

      MITK_DEBUG << "Volume dimension: [" << image->GetDimension(0) << ", " << image->GetDimension(1) << ", " << image->GetDimension(2) << ", " << image->GetDimension(3) << "]";
      MITK_DEBUG << "Volume spacing: [" << image->GetGeometry()->GetSpacing()[0] << ", " << image->GetGeometry()->GetSpacing()[1] << ", " << image->GetGeometry()->GetSpacing()[2] << "]";

      for (std::list<StringContainer>::iterator df_it = ++decomposed_filenames.begin(); df_it != df_end; ++df_it)
      {
        reader->SetFileNames(*df_it);
        reader->Update();
        image->SetImportVolume(reader->GetOutput()->GetBufferPointer(), act_volume++);
      }
    }
    else
    {
      typedef itk::Image<PixelType, 3> ImageType;
      typedef itk::ImageSeriesReader<ImageType> ReaderType;

      DcmIoType::Pointer io = DcmIoType::New();
      typename ReaderType::Pointer reader = ReaderType::New();

      reader->SetImageIO(io);
      reader->ReverseOrderOff();

      if (command)
      {
        reader->AddObserver(itk::ProgressEvent(), command);
      }

      reader->SetFileNames(filenames);
      reader->Update();
      image->InitializeByItk(reader->GetOutput());
      image->SetImportVolume(reader->GetOutput()->GetBufferPointer());

      MITK_DEBUG << "Volume dimension: [" << image->GetDimension(0) << ", " << image->GetDimension(1) << ", " << image->GetDimension(2) << "]";
      MITK_DEBUG << "Volume spacing: [" << image->GetGeometry()->GetSpacing()[0] << ", " << image->GetGeometry()->GetSpacing()[1] << ", " << image->GetGeometry()->GetSpacing()[2] << "]";
    }

#else
    typedef itk::Image<PixelType, 3> ImageType;
    typedef itk::ImageSeriesReader<ImageType> ReaderType;

    DcmIoType::Pointer io = DcmIoType::New();
    typename ReaderType::Pointer reader = ReaderType::New();

    reader->SetImageIO(io);
    reader->ReverseOrderOff();

    if (command)
    {
      reader->AddObserver(itk::ProgressEvent(), command);
    }

    reader->SetFileNames(filenames);
    reader->Update();
    image->InitializeByItk(reader->GetOutput());
    image->SetImportVolume(reader->GetOutput()->GetBufferPointer());

    MITK_DEBUG << "Volume dimension: [" << image->GetDimension(0) << ", " << image->GetDimension(1) << ", " << image->GetDimension(2) << "]";
    MITK_DEBUG << "Volume spacing: [" << image->GetGeometry()->GetSpacing()[0] << ", " << image->GetGeometry()->GetSpacing()[1] << ", " << image->GetGeometry()->GetSpacing()[2] << "]";

#endif

    node.SetData(image);
    
    setlocale(LC_NUMERIC, previousCLocale);
    std::cin.imbue(previousCppLocale);

  }
  catch (std::exception& e)
  {
    setlocale(LC_NUMERIC, previousCLocale);
    std::cin.imbue(previousCppLocale);

    throw e;
  }
}

}

#endif
