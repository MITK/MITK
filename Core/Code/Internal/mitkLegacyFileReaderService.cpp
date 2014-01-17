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

#include "mitkLegacyFileReaderService.h"

#include <mitkDicomSeriesReader.h>
#include <mitkProgressBar.h>
#include <mitkIOAdapter.h>

#include <usGetModuleContext.h>

mitk::LegacyFileReaderService::LegacyFileReaderService(const mitk::LegacyFileReaderService& other)
  : mitk::AbstractFileReader(other)
{
}

mitk::LegacyFileReaderService::LegacyFileReaderService(const std::string& mimeType,
                                                       const std::string& extension,
                                                       const std::string& description)
  : AbstractFileReader(mimeType, extension, description)
{
  if (mimeType.empty()) mitkThrow() << "LegacyFileReaderService cannot be initialized without a mime type for extension " << extension << ".";
  if (extension.empty()) mitkThrow() << "LegacyFileReaderService cannot be initialized without file extension for mime-type " << mimeType << ".";

  this->SetPriority(-100);
  m_ServiceReg = this->RegisterService();
}

mitk::LegacyFileReaderService::~LegacyFileReaderService()
{
  try
  {
    m_ServiceReg.Unregister();
  }
  catch (const std::exception&)
  {}
}

////////////////////// Reading /////////////////////////

std::vector<itk::SmartPointer<mitk::BaseData> > mitk::LegacyFileReaderService::Read(const std::string& path)
{
  std::vector<itk::SmartPointer<BaseData> > result;

  // The following code is adapted legacy code copied from DataNodeFactory::GenerateData()

  // IF filename is something.pic, and something.pic does not exist, try to read something.pic.gz
  // if there are both, something.pic and something.pic.gz, only the requested file is read
  // not only for images, but for all formats

  // part for DICOM
//  const char *numbers = "0123456789.";
//  std::string::size_type first_non_number;
//  first_non_number = itksys::SystemTools::GetFilenameName(m_FileName).find_first_not_of ( numbers );

  if (mitk::DicomSeriesReader::IsDicom(path) /*|| first_non_number == std::string::npos*/)
  {
    result = this->ReadFileSeriesTypeDCM(path);
  }
  else
  {
    // the mitkBaseDataIO class returns a pointer of a vector of BaseData objects
    result = LoadBaseDataFromFile( path );
  }

  return result;
}

std::vector<itk::SmartPointer<mitk::BaseData> > mitk::LegacyFileReaderService::Read(std::istream& stream)
{
  return mitk::AbstractFileReader::Read(stream);
}

std::vector<mitk::BaseData::Pointer> mitk::LegacyFileReaderService::ReadFileSeriesTypeDCM(const std::string& path)
{
  const char* previousCLocale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
  std::locale previousCppLocale( std::cin.getloc() );
  std::locale l( "C" );
  std::cin.imbue(l);

  std::vector<mitk::BaseData::Pointer> result;

  if (  DicomSeriesReader::IsPhilips3DDicom(path) )
  {
    MITK_INFO << "it is a Philips3D US Dicom file" << std::endl;
    DataNode::Pointer node = DataNode::New();
    mitk::DicomSeriesReader::StringContainer stringvec;
    stringvec.push_back(path);
    if (DicomSeriesReader::LoadDicomSeries(stringvec, *node))
    {
      //node->SetName(this->GetBaseFileName());
    }
    setlocale(LC_NUMERIC, previousCLocale);
    std::cin.imbue(previousCppLocale);
    result.push_back(node->GetData());
  }
  else
  {
    std::string dir = itksys::SystemTools::GetFilenamePath(path);
    DicomSeriesReader::FileNamesGrouping imageBlocks = DicomSeriesReader::GetSeries(dir, true, std::vector<std::string>()); // true = group gantry tilt images
    const unsigned int size = imageBlocks.size();

    ProgressBar::GetInstance()->AddStepsToDo(size);
    ProgressBar::GetInstance()->Progress();

    unsigned int outputIndex = 0u;
    const DicomSeriesReader::FileNamesGrouping::const_iterator n_end = imageBlocks.end();

    for (DicomSeriesReader::FileNamesGrouping::const_iterator n_it = imageBlocks.begin(); n_it != n_end; ++n_it)
    {
      const std::string &uid = n_it->first;
      DataNode::Pointer node = DataNode::New();

      const DicomSeriesReader::ImageBlockDescriptor& imageBlockDescriptor( n_it->second );

      MITK_INFO << "--------------------------------------------------------------------------------";
      MITK_INFO << "LegayFileReaderService: Loading DICOM series " << outputIndex << ": Series UID " << imageBlockDescriptor.GetSeriesInstanceUID() << std::endl;
      MITK_INFO << "  " << imageBlockDescriptor.GetFilenames().size() << " '" << imageBlockDescriptor.GetModality() << "' files (" << imageBlockDescriptor.GetSOPClassUIDAsString() << ") loaded into 1 mitk::Image";
      MITK_INFO << "  multi-frame: " << (imageBlockDescriptor.IsMultiFrameImage()?"Yes":"No");
      MITK_INFO << "  reader support: " << DicomSeriesReader::ReaderImplementationLevelToString(imageBlockDescriptor.GetReaderImplementationLevel());
      MITK_INFO << "  pixel spacing type: " << DicomSeriesReader::PixelSpacingInterpretationToString( imageBlockDescriptor.GetPixelSpacingType() );
      MITK_INFO << "  gantry tilt corrected: " << (imageBlockDescriptor.HasGantryTiltCorrected()?"Yes":"No");
      MITK_INFO << "  3D+t: " << (imageBlockDescriptor.HasMultipleTimePoints()?"Yes":"No");
      MITK_INFO << "--------------------------------------------------------------------------------";

      if (DicomSeriesReader::LoadDicomSeries(n_it->second.GetFilenames(), *node, true, true, true))
      {
        std::string nodeName(uid);
        std::string studyDescription;
        if ( node->GetStringProperty( "dicom.study.StudyDescription", studyDescription ) )
        {
          nodeName = studyDescription;
          std::string seriesDescription;
          if ( node->GetStringProperty( "dicom.series.SeriesDescription", seriesDescription ) )
          {
            nodeName += "/" + seriesDescription;
          }
        }

        node->SetName(nodeName);
        result.push_back(node->GetData());

        ++outputIndex;
      }
      else
      {
        MITK_ERROR << "DataNodeFactory: Skipping series " << outputIndex << " due to some unspecified error..." << std::endl;
      }

      ProgressBar::GetInstance()->Progress();
    }
  }

  setlocale(LC_NUMERIC, previousCLocale);
  std::cin.imbue(previousCppLocale);

  return result;
}

std::vector<mitk::BaseData::Pointer> mitk::LegacyFileReaderService::LoadBaseDataFromFile(const std::string& path)
{

  // factories are instantiated in mitk::CoreObjectFactory and other, potentially MITK external places

  std::vector<BaseData::Pointer> result;

  std::list<IOAdapterBase::Pointer> possibleIOAdapter;
  std::list<itk::LightObject::Pointer> allobjects = itk::ObjectFactoryBase::CreateAllInstance("mitkIOAdapter");

  for( std::list<itk::LightObject::Pointer>::iterator i = allobjects.begin();
       i != allobjects.end();
       ++i)
  {
    IOAdapterBase* io = dynamic_cast<IOAdapterBase*>(i->GetPointer());
    if(io)
    {
      possibleIOAdapter.push_back(io);
    }
    else
    {
      MITK_ERROR << "Error BaseDataIO factory did not return an IOAdapterBase: "
        << (*i)->GetNameOfClass()
        << std::endl;
    }
  }

  for( std::list<IOAdapterBase::Pointer>::iterator k = possibleIOAdapter.begin();
                                                   k != possibleIOAdapter.end();
                                                   ++k )
  {
    bool canReadFile = (*k)->CanReadFile(path, "", "");       // they could read the file

    if( canReadFile )
    {
      BaseProcess::Pointer ioObject = (*k)->CreateIOProcessObject(path, "", "");
      ioObject->Update();
      int numberOfContents = static_cast<int>(ioObject->GetNumberOfOutputs());

      if (numberOfContents > 0)
      {
        BaseData::Pointer baseData;
        for(int i=0; i<numberOfContents; ++i)
        {
          baseData = dynamic_cast<BaseData*>(ioObject->GetOutputs()[i].GetPointer());
          if (baseData) // this is what's wanted, right?
          {
            result.push_back( baseData );
          }
        }
      }

      break;
    }
  }

  return result;
}

us::ServiceRegistration<mitk::IMimeType> mitk::LegacyFileReaderService::RegisterMimeType(us::ModuleContext* /*context*/)
{
  // Do nothing. Mime types are registered explicitly in the LegacyIO module
  return us::ServiceRegistration<IMimeType>();
}

mitk::LegacyFileReaderService* mitk::LegacyFileReaderService::Clone() const
{
  return new LegacyFileReaderService(*this);
}
