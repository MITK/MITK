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

#ifndef __mitkDecisionForestIO__cpp
#define __mitkDecisionForestIO__cpp

#include "mitkRandomForestIO.h"
#include "itksys/SystemTools.hxx"
//#include "mitkHDF5IOMimeTypes.h"


#include "vigra/random_forest_hdf5_impex.hxx"

#include <iostream>
#include <fstream>

#include "mitkVigraRandomForestClassifier.h"
#include "mitkIOMimeTypes.h"

#define GetAttribute(name,type)\
  type name;\
  hdf5_file.readAttribute(".",name,name);


mitk::RandomForestFileIO::ConfidenceLevel mitk::RandomForestFileIO::GetReaderConfidenceLevel() const
{
  std::string ext = itksys::SystemTools::GetFilenameLastExtension(this->GetLocalFileName().c_str());
  bool is_loaded = vigra::rf_import_HDF5(m_rf, this->GetInputLocation());
  return ext == ".hdf5"  && is_loaded == true? IFileReader::Supported : IFileReader::Unsupported;
}

mitk::RandomForestFileIO::ConfidenceLevel mitk::RandomForestFileIO::GetWriterConfidenceLevel() const
{
  mitk::VigraRandomForestClassifier::ConstPointer input = dynamic_cast<const mitk::VigraRandomForestClassifier *>(this->GetInput());
  if (input.IsNull())
  {
    return IFileWriter::Unsupported;
  }else{
    return IFileWriter::Supported;
  }
}


mitk::RandomForestFileIO::RandomForestFileIO()
  : AbstractFileIO(mitk::VigraRandomForestClassifier::GetStaticNameOfClass())
{
  CustomMimeType customReaderMimeType(mitk::IOMimeTypes::DEFAULT_BASE_NAME() + ".hdf5");
  std::string category = "Vigra Random Forest File";
  customReaderMimeType.SetComment("Vigra Random Forest");
  customReaderMimeType.SetCategory(category);
  customReaderMimeType.AddExtension("hdf5");

//  this->AbstractFileIOReader::SetRanking(100);
//  this->AbstractFileIOWriter::SetRanking(100);
  this->AbstractFileWriter::SetMimeTypePrefix(mitk::IOMimeTypes::DEFAULT_BASE_NAME() + ".hdf5");
  this->AbstractFileWriter::SetMimeType(customReaderMimeType);
  this->SetWriterDescription("Vigra Random Forest");
  this->AbstractFileReader::SetMimeTypePrefix(mitk::IOMimeTypes::DEFAULT_BASE_NAME() + ".hdf5");
  this->AbstractFileReader::SetMimeType(customReaderMimeType);
  this->SetReaderDescription("Vigra Random Forest");

  //  this->SetReaderDescription(mitk::DecisionForestIOMimeTypes::DECISIONFOREST_MIMETYPE_DESCRIPTION());
  //  this->SetWriterDescription(mitk::DecisionForestIOMimeTypes::DECISIONFOREST_MIMETYPE_DESCRIPTION());
  this->RegisterService();
}

mitk::RandomForestFileIO::RandomForestFileIO(const mitk::RandomForestFileIO& other)
  : AbstractFileIO(other)
{
}

mitk::RandomForestFileIO::~RandomForestFileIO()
{}

std::vector<itk::SmartPointer<mitk::BaseData> >
mitk::RandomForestFileIO::
Read()
{

  mitk::VigraRandomForestClassifier::Pointer output = mitk::VigraRandomForestClassifier::New();
  std::vector<itk::SmartPointer<mitk::BaseData> > result;

  if ( this->GetInputLocation().empty())
  {
    MITK_ERROR << "Sorry, filename has not been set!";
    return result;
  }
  else
  {

    const std::string& locale = "C";
    const std::string& currLocale = setlocale( LC_ALL, NULL );

    if ( locale.compare(currLocale)!=0 )
    {
      try
      {
        setlocale(LC_ALL, locale.c_str());
      }
      catch(...)
      {
        MITK_INFO << "Could not set locale " << locale;
      }
    }




//    vigra::HDF5File hdf5_file;
//    vigra::rf_import_HDF5(rf,hdf5_file,this->GetInputLocation());
    output->SetRandomForest(m_rf);
    result.push_back(output.GetPointer());

//    if(!hdf5_file.existsAttribute(".","mitk")){
//      return result;
//    }else{

//      GetAttribute(mitk_isMitkDecisionTree,std::string);
//      if(mitk_isMitkDecisionTree.empty()) return result;

//      GetAttribute(mitk_Modalities,std::string);
//      std::vector<std::string> strs;
//      boost::split(strs, mitk_Modalities, boost::is_any_of("\t ,"));
//      MITK_INFO << "Import Modalities: " << mitk_Modalities;
//      output->SetModalities(strs);

//    }

    return result;
  }
}


void mitk::RandomForestFileIO::Write()
{
  mitk::BaseData::ConstPointer input = this->GetInput();
  if (input.IsNull())
  {
    MITK_ERROR <<"Sorry, input to NrrdDiffusionImageWriter is NULL!";
    return;
  }
  if ( this->GetOutputLocation().empty() )
  {
    MITK_ERROR << "Sorry, filename has not been set!";
    return ;
  }else{
    const std::string& locale = "C";
    const std::string& currLocale = setlocale( LC_ALL, NULL );

    if ( locale.compare(currLocale)!=0 )
    {
      try
      {
        setlocale(LC_ALL, locale.c_str());
      }
      catch(...)
      {
        MITK_INFO << "Could not set locale " << locale;
      }
    }

    mitk::VigraRandomForestClassifier::ConstPointer mitkDC = dynamic_cast<const mitk::VigraRandomForestClassifier *>(input.GetPointer());
    //mitkDC->GetRandomForest()
    vigra::rf_export_HDF5(mitkDC->GetRandomForest(), this->GetOutputLocation());

    vigra::HDF5File hdf5_file;
//    vigra::rf_import_HDF5(rf,hdf5_file,this->GetInputLocation());
//    output->SetRandomForest(rf);
//    result.push_back(output.GetPointer());

//    if(!hdf5_file.existsAttribute(".","mitk")){
//      return result;
//    }else{

//      GetAttribute(mitk_isMitkDecisionTree,std::string);
//      if(mitk_isMitkDecisionTree.empty()) return result;

//      GetAttribute(mitk_Modalities,std::string);
//      std::vector<std::string> strs;
//      boost::split(strs, mitk_Modalities, boost::is_any_of("\t ,"));
//      MITK_INFO << "Import Modalities: " << mitk_Modalities;
//      output->SetModalities(strs);

//    }

  }
}


mitk::AbstractFileIO* mitk::RandomForestFileIO::IOClone() const
{
  return new RandomForestFileIO(*this);
}

#endif
