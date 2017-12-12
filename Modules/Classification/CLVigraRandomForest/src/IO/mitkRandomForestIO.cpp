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
  return ext == ".forest"  && is_loaded == true? IFileReader::Supported : IFileReader::Unsupported;
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
  CustomMimeType customReaderMimeType(mitk::IOMimeTypes::DEFAULT_BASE_NAME() + ".forest");
  std::string category = "Vigra Random Forest File";
  customReaderMimeType.SetComment("Vigra Random Forest");
  customReaderMimeType.SetCategory(category);
  customReaderMimeType.AddExtension("forest");

  //  this->AbstractFileIOWriter::SetRanking(100);
  this->AbstractFileWriter::SetMimeTypePrefix(mitk::IOMimeTypes::DEFAULT_BASE_NAME() + ".forest");
  this->AbstractFileWriter::SetMimeType(customReaderMimeType);
  this->SetWriterDescription("Vigra Random Forest");
  this->AbstractFileReader::SetMimeTypePrefix(mitk::IOMimeTypes::DEFAULT_BASE_NAME() + ".forest");
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
    const std::string& currLocale = setlocale( LC_ALL, nullptr );

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

    output->SetRandomForest(m_rf);
    result.push_back(output.GetPointer());
    vigra::HDF5File hdf5_file(this->GetInputLocation() , vigra::HDF5File::Open);


    hdf5_file.cd_mk("/_mitkOptions");

    // ---------------------------------------------------------
    // Read tree weights
    if(hdf5_file.existsDataset("treeWeights"))
    {
      auto treeWeight = output->GetTreeWeights();
      treeWeight.resize(m_rf.tree_count(),1);
      vigra::MultiArrayView<2, double> W(vigra::Shape2(treeWeight.rows(),treeWeight.cols()),treeWeight.data());
      hdf5_file.read("treeWeights",W);
      output->SetTreeWeights(treeWeight);
    }
    // ---------------------------------------------------------

    // ---------------------------------------------------------
    // Read itemList
    if(hdf5_file.existsDataset("itemList")){
      std::string items_string;
      hdf5_file.read("itemList",items_string);
      auto itemlist = output->GetItemList();

      std::string current_item = "";
      for(auto character : items_string)
      {
        if(character == ';'){
          // skip seperator and push back item
          itemlist.push_back(current_item);
          current_item.clear();
        }else{
          current_item = current_item + character;
        }
      }
      output->SetItemList(itemlist);
    }
    // ---------------------------------------------------------

    hdf5_file.close();

    return result;
  }
}

void mitk::RandomForestFileIO::Write()
{
  mitk::BaseData::ConstPointer input = this->GetInput();
  if (input.IsNull())
  {
    MITK_ERROR <<"Sorry, input to NrrdDiffusionImageWriter is nullptr!";
    return;
  }
  if ( this->GetOutputLocation().empty() )
  {
    MITK_ERROR << "Sorry, filename has not been set!";
    return ;
  }else{
    const std::string& locale = "C";
    const std::string& currLocale = setlocale( LC_ALL, nullptr );

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

    vigra::HDF5File hdf5_file(this->GetOutputLocation() , vigra::HDF5File::Open);

    hdf5_file.cd_mk("/_mitkOptions");

    // Write tree weights
    // ---------------------------------------------------------
    auto treeWeight = mitkDC->GetTreeWeights();
    vigra::MultiArrayView<2, double> W(vigra::Shape2(treeWeight.rows(),treeWeight.cols()),treeWeight.data());
    hdf5_file.write("treeWeights",W);
    // ---------------------------------------------------------

    // Write itemList
    // ---------------------------------------------------------
    auto items = mitkDC->GetItemList();
    std::string item_stringlist;
    for(auto entry : items)
      item_stringlist = item_stringlist + entry + ";";

    hdf5_file.write("itemList",item_stringlist);
    // ---------------------------------------------------------

    hdf5_file.close();
  }
}

mitk::AbstractFileIO* mitk::RandomForestFileIO::IOClone() const
{
  return new RandomForestFileIO(*this);
}

#endif
