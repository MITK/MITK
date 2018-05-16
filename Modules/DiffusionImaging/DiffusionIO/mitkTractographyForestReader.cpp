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

#include "mitkTractographyForestReader.h"
#include <itkMetaDataObject.h>
#include <mitkCustomMimeType.h>
#include "mitkDiffusionIOMimeTypes.h"

#undef DIFFERENCE
#define VIGRA_STATIC_LIB
#include <vigra/random_forest.hxx>
#include <vigra/random_forest_hdf5_impex.hxx>


mitk::TractographyForestReader::TractographyForestReader()
  : mitk::AbstractFileReader( mitk::DiffusionIOMimeTypes::TRACTOGRAPHYFOREST_MIMETYPE_NAME(), "Tractography Forest" )
{
  m_ServiceReg = this->RegisterService();
}

mitk::TractographyForestReader::TractographyForestReader(const TractographyForestReader &other)
  :mitk::AbstractFileReader(other)
{
}

mitk::TractographyForestReader * mitk::TractographyForestReader::Clone() const
{
  return new TractographyForestReader(*this);
}


std::vector<itk::SmartPointer<mitk::BaseData> > mitk::TractographyForestReader::Read()
{
  std::vector<itk::SmartPointer<mitk::BaseData> > result;
  std::shared_ptr< vigra::RandomForest<int> > forest;

  const std::string& locale = "C";
  const std::string& currLocale = setlocale( LC_ALL, nullptr );
  setlocale(LC_ALL, locale.c_str());

  std::string filename = this->GetInputLocation();

  try
  {
    MITK_INFO << "Trying to load random forest file.";

    forest = std::make_shared< vigra::RandomForest<int> >();
    vigra::rf_import_HDF5( *forest, filename);

    mitk::TractographyForest::Pointer mitkForest = mitk::TractographyForest::New(forest);
    result.push_back(mitkForest.GetPointer());
  }
  catch(...)
  {
    setlocale(LC_ALL, currLocale.c_str());
    MITK_INFO << "Reading random forest failed";
    throw;
  }

  setlocale(LC_ALL, currLocale.c_str());
  MITK_INFO << "Random forest read";

  return result;
}
