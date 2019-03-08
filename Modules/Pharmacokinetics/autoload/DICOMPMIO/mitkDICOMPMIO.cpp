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

#ifndef __mitkDICOMPharmacokineticsIO__cpp
#define __mitkDICOMPharmacokineticsIO__cpp

#include "mitkDICOMPMIO.h"
#include "mitkDICOMQIIOMimeTypes.h"
#include "mitkDICOMPMConstants.h"
#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkDICOMIOHelper.h>
#include <mitkDICOMProperty.h>
#include <mitkIDICOMTagsOfInterest.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkLocaleSwitch.h>
#include <mitkPropertyNameHelper.h>
#include <dcmqi/ParaMapConverter.h>
#include "mitkParamapPresetsParser.h"


// us
#include <usGetModuleContext.h>
#include <usModuleContext.h>

// model fit parameters
#include "mitkModelFitConstants.h"


namespace mitk
{
  DICOMPMIO::DICOMPMIO()
    : AbstractFileIO(Image::GetStaticNameOfClass(),
                     mitk::MitkDICOMQIIOMimeTypes::DICOMPM_MIMETYPE_NAME(),
                     "DICOM PM")

  {
    AbstractFileWriter::SetRanking(10);
    AbstractFileReader::SetRanking(10);
    this->RegisterService();
    this->AddDICOMTagsToService();
  }

  void DICOMPMIO::AddDICOMTagsToService()
  {
    IDICOMTagsOfInterest *toiService = GetDicomTagsOfInterestService();
    if (toiService != nullptr)
    {
	  toiService->AddTagOfInterest(DICOMPMConstants::RWVM_SEQUENCE_PATH());
	  toiService->AddTagOfInterest(DICOMPMConstants::UNITS_SEQUENCE_PATH());
	  toiService->AddTagOfInterest(DICOMPMConstants::UNITS_CODE_VALUE_PATH());
	  toiService->AddTagOfInterest(DICOMPMConstants::UNITS_CODE_SCHEME_PATH());
	  toiService->AddTagOfInterest(DICOMPMConstants::UNITS_CODE_MEANING_PATH());
	  toiService->AddTagOfInterest(DICOMPMConstants::QUANTITY_DEFINITION_SEQUENCE_PATH());
	  toiService->AddTagOfInterest(DICOMPMConstants::QUANTITY_DEFINITION_VALUE_TYPE_PATH());
	  toiService->AddTagOfInterest(DICOMPMConstants::QUANTITY_DEFINITION_CONCEPT_CODE_SEQUENCE_PATH());
	  toiService->AddTagOfInterest(DICOMPMConstants::QUANTITY_DEFINITION_CONCEPT_CODE_VALUE_PATH());
	  toiService->AddTagOfInterest(DICOMPMConstants::QUANTITY_DEFINITION_CONCEPT_CODE_SCHEME_PATH());
	  toiService->AddTagOfInterest(DICOMPMConstants::QUANTITY_DEFINITION_CONCEPT_CODE_MEANING_PATH());
    }
  }

  IFileIO::ConfidenceLevel DICOMPMIO::GetWriterConfidenceLevel() const
  {
    if (AbstractFileIO::GetWriterConfidenceLevel() == Unsupported)
      return Unsupported;
	
	const Image *PMinput = static_cast<const Image *>(this->GetInput());
	if (PMinput)
		return Supported;
	else
		return Unsupported;
  }

  void DICOMPMIO::Write()
  {
    ValidateOutputLocation();
    mitk::LocaleSwitch localeSwitch("C");
    LocalFile localFile(this);
    const std::string path = localFile.GetFileName();

	auto PMinput = dynamic_cast<const Image *>(this->GetInput());
	if (PMinput == nullptr)
		mitkThrow() << "Cannot write non-image data";

    // Get DICOM information from referenced image
    vector<DcmDataset *> dcmDatasets;


    DcmFileFormat *readFileFormat = new DcmFileFormat();
    try
    {
      // Generate dcmdataset witk DICOM tags from property list; ATM the source are the filepaths from the
      // property list
      mitk::StringLookupTableProperty::Pointer filesProp =
        dynamic_cast<mitk::StringLookupTableProperty *>(PMinput->GetProperty("referenceFiles").GetPointer());

      if (filesProp.IsNull())
      {
        mitkThrow() << "No property with dicom file path.";
        return;
      }


	  // returns a list of all referenced files
      StringLookupTable filesLut = filesProp->GetValue();
      const StringLookupTable::LookupTableType &lookUpTableMap = filesLut.GetLookupTable();
      for (auto it : lookUpTableMap)
      {
        const char *fileName = (it.second).c_str();
        if (readFileFormat->loadFile(fileName, EXS_Unknown).good())
          dcmDatasets.push_back(readFileFormat->getAndRemoveDataset());
      }
    }
    catch (const std::exception &e)
    {
      MITK_ERROR << "An error occurred while getting the dicom information: " << e.what() << endl;
      return;
    }

	mitk::Image *mitkPMImage = const_cast<mitk::Image *>(PMinput);
	// Cast input PMinput to itk image 
	ImageToItk<PMitkInputImageType>::Pointer PMimageToItkFilter = ImageToItk<PMitkInputImageType>::New();
	PMimageToItkFilter->SetInput(mitkPMImage);
	PMimageToItkFilter->Update();
		
	// Cast from original itk type to dcmqi input itk image type
	typedef itk::CastImageFilter<PMitkInputImageType, PMitkInternalImageType> castItkImageFilterType;
	castItkImageFilterType::Pointer castFilter = castItkImageFilterType::New();
	castFilter->SetInput(PMimageToItkFilter->GetOutput());
	castFilter->Update();
	PMitkInternalImageType::Pointer itkParamapImage = castFilter->GetOutput();
	itkParamapImage->DisconnectPipeline();
	
	// Create PM meta information
	const std::string &tmpMetaInfoFile = this->CreateMetaDataJsonFilePM();

	// Convert itk PM images to dicom image
	MITK_INFO << "Writing PM image: " << path << std::endl;
	try
	{
		dcmqi::ParaMapConverter *PMconverter = new dcmqi::ParaMapConverter();
		DcmDataset *PMresult = PMconverter->itkimage2paramap(itkParamapImage, dcmDatasets, tmpMetaInfoFile);
		// Write dicom file
		DcmFileFormat dcmFileFormat(PMresult);
		std::string filePath = path.substr(0, path.find_last_of("."));
		filePath = filePath + ".dcm";
		dcmFileFormat.saveFile(filePath.c_str(), EXS_LittleEndianExplicit);
		// Clean up
		if (PMconverter != nullptr)
			delete PMconverter;
		if (PMresult != nullptr)
			delete PMresult;
	}
	catch (const std::exception &e)
	{
		MITK_ERROR << "An error occurred during writing the DICOM Paramap: " << e.what() << endl;
		return;
	}

	//-------------------------------------------------------------//

    // End of image writing; clean up
    if (readFileFormat)
      delete readFileFormat;

    for (auto obj : dcmDatasets)
      delete obj;
    dcmDatasets.clear();
	
  }
  
  const std::string mitk::DICOMPMIO::CreateMetaDataJsonFilePM() const
  {  
	  const mitk::Image *PMimage = dynamic_cast<const mitk::Image *>(this->GetInput());
	  dcmqi::JSONParametricMapMetaInformationHandler PMhandler;
	  
	  // Get Metadata from modelFitConstants
	  std::string parameterName;
	  PMimage->GetPropertyList()->GetStringProperty(ModelFitConstants::PARAMETER_NAME_PROPERTY_NAME().c_str(), parameterName);
	  std::string modelName;
	  PMimage->GetPropertyList()->GetStringProperty(ModelFitConstants::MODEL_NAME_PROPERTY_NAME().c_str(), modelName);


      mitk::ParamapPresetsParser* pmPresets = mitk::ParamapPresetsParser::New();
      // Here the mitkParamapPresets.xml file containing the Coding Schmeme Designator and Code Value are parsed and the relevant values extracted
      pmPresets->LoadPreset();

	  auto pmType_parameterName = pmPresets->GetType(parameterName);
	  auto pmType_modelName = pmPresets->GetType(modelName);

	  // Here some other 
	  // mandatory
	  // TODO: where to get these from?
	  // TODO: AnatomicRegionSequence from Segmentation?
	  PMhandler.setAnatomicRegionSequence("T-9200B", "SRT", "Prostate");
	  PMhandler.setDerivedPixelContrast("TCS");
	  PMhandler.setFrameLaterality("U");
	  // optional
	  PMhandler.setQuantityValueCode(pmType_parameterName.codeValue, pmType_parameterName.codeScheme, parameterName);
	  PMhandler.setMeasurementMethodCode(pmType_modelName.codeValue, pmType_modelName.codeScheme, modelName);
	  PMhandler.setSeriesNumber("1");
	  PMhandler.setInstanceNumber("1");
	  PMhandler.setDerivationCode("110816", "DCM", "Time Course of Signal");
	  PMhandler.setMeasurementUnitsCode("/min", "UCUM", "/m");
	  PMhandler.setRealWorldValueSlope(1);

	  return PMhandler.getJSONOutputAsString();
  }


  // IKO: not yet implemented, TO DO: write DICOMPMReader
  std::vector<BaseData::Pointer> DICOMPMIO::Read()
  {
	  mitk::LocaleSwitch localeSwitch("C");
	  std::vector<BaseData::Pointer> result;
	  return result;
  }

  // IKO: not yet implemented TO DO: write DICOMPMReaderConfidenceLevel
  IFileIO::ConfidenceLevel DICOMPMIO::GetReaderConfidenceLevel() const
  {
	  return Unsupported;
  }



  DICOMPMIO *DICOMPMIO::IOClone() const { return new DICOMPMIO(*this); }
} // namespace






#endif //__mitkDICOMPharmacokineticsIO__cpp
