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
void DicomSeriesReader::LoadDicom(const StringContainer &filenames, DataNode &node, bool sort, bool check_4d, UpdateCallBackMethod callback)
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

    /******** For Philips 3D+t images ********/
    if ( DicomSeriesReader::IsPhilips3DDicom(filenames.front().c_str())  )
    {
      ReadPhilips3DDicom(filenames.front().c_str(), image);
      node.SetData(image);
      setlocale(LC_NUMERIC, previousCLocale);
      std::cin.imbue(previousCppLocale);
      return;
    }

    /******** For 4D data split in multiple files ***************/
    std::list<StringContainer> decomposed_filenames;
    unsigned int volume_count = 1u;

    StringContainer sorted_filenames = sort
                                       ? DicomSeriesReader::SortSeriesSlices(filenames)
                                       : filenames;
    if (check_4d)
    {
      gdcm::Tag tag(0x0020,0x0032); //Image position (Patient)
      gdcm::Scanner scanner;

      scanner.AddTag(tag);
      scanner.Scan(sorted_filenames);

      const StringContainer::const_iterator f_end = sorted_filenames.end();
      const char *act_value = scanner.GetValue(sorted_filenames.front().c_str(), tag);

      decomposed_filenames.push_back(StringContainer());
      decomposed_filenames.back().push_back(sorted_filenames.front());

      for (StringContainer::const_iterator f_it = ++sorted_filenames.begin(); f_it != f_end; ++f_it)
      {
        const char *value = scanner.GetValue(f_it->c_str(), tag);

        if (!strcmp(act_value, value))
        {
          decomposed_filenames.push_back(StringContainer());
          ++volume_count;
        }

        decomposed_filenames.back().push_back(*f_it);
      }
    }
    else
    {
      decomposed_filenames.push_back(sorted_filenames);
    }

    if (volume_count > 1u)
    {
      // It is 4D! Read it and store into mitk image

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

#if (GDCM_MAJOR_VERSION == 2) && (GDCM_MINOR_VERSION < 1) && (GDCM_BUILD_VERSION < 15)
      // workaround for a GDCM 2 bug until version 2.0.15:
      // GDCM read spacing vector wrongly. Instead of "row spacing, column spacing", it misinterprets the DICOM tag as "column spacing, row spacing".
      // this is undone here, until we use a GDCM that has this issue fixed.
      // From the commit comments, GDCM 2.0.15 fixed the spacing interpretation with bug 2901181
      // http://sourceforge.net/tracker/index.php?func=detail&aid=2901181&group_id=137895&atid=739587


      Vector3D correctedImageSpacing = image->GetGeometry()->GetSpacing();
      std::swap( correctedImageSpacing[0], correctedImageSpacing[1] );
      image->GetGeometry()->SetSpacing( correctedImageSpacing );
#endif


      for (std::list<StringContainer>::iterator df_it = ++decomposed_filenames.begin(); df_it != df_end; ++df_it)
      {
        reader->SetFileNames(*df_it);
        reader->Update();
        image->SetImportVolume(reader->GetOutput()->GetBufferPointer(), act_volume++);
      }
      node.SetData(image);
      setlocale(LC_NUMERIC, previousCLocale);
      std::cin.imbue(previousCppLocale);
      return; // finished
    }
#endif
    /******** Normal Case, 3D (also for GDCM < 2 usable) ***************/

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

#if (GDCM_MAJOR_VERSION == 2) && (GDCM_MINOR_VERSION < 1) && (GDCM_BUILD_VERSION < 15)
      // workaround for a GDCM 2 bug until version 2.0.15:
      // GDCM read spacing vector wrongly. Instead of "row spacing, column spacing", it misinterprets the DICOM tag as "column spacing, row spacing".
      // this is undone here, until we use a GDCM that has this issue fixed.
      // From the commit comments, GDCM 2.0.15 fixed the spacing interpretation with bug 2901181
      // http://sourceforge.net/tracker/index.php?func=detail&aid=2901181&group_id=137895&atid=739587


      Vector3D correctedImageSpacing = image->GetGeometry()->GetSpacing();
      std::swap( correctedImageSpacing[0], correctedImageSpacing[1] );
      image->GetGeometry()->SetSpacing( correctedImageSpacing );
#endif


    MITK_DEBUG << "Volume dimension: [" << image->GetDimension(0) << ", " << image->GetDimension(1) << ", " << image->GetDimension(2) << "]";
    MITK_DEBUG << "Volume spacing: [" << image->GetGeometry()->GetSpacing()[0] << ", " << image->GetGeometry()->GetSpacing()[1] << ", " << image->GetGeometry()->GetSpacing()[2] << "]";

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
