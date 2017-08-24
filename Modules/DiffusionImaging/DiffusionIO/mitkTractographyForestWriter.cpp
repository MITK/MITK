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

#include "mitkTractographyForestWriter.h"
#include <mitkAbstractFileWriter.h>
#include <mitkCustomMimeType.h>
#include <mitkDiffusionIOMimeTypes.h>
#include <mitkTractographyForest.h>

//#undef DIFFERENCE
//#define VIGRA_STATIC_LIB
//#include <vigra/random_forest.hxx>
#include <vigra/random_forest_hdf5_impex.hxx>

mitk::TractographyForestWriter::TractographyForestWriter()
  : mitk::AbstractFileWriter(mitk::TractographyForest::GetStaticNameOfClass(), mitk::DiffusionIOMimeTypes::TRACTOGRAPHYFOREST_MIMETYPE_NAME(), "Tractography Forest")
{
  RegisterService();
}

mitk::TractographyForestWriter::TractographyForestWriter(const mitk::TractographyForestWriter & other)
  :mitk::AbstractFileWriter(other)
{}

mitk::TractographyForestWriter::~TractographyForestWriter()
{}

mitk::TractographyForestWriter * mitk::TractographyForestWriter::Clone() const
{
  return new mitk::TractographyForestWriter(*this);
}

void mitk::TractographyForestWriter::Write()
{

  std::ostream* out;
  std::ofstream outStream;

  if( this->GetOutputStream() )
  {
    out = this->GetOutputStream();
  }else{
    outStream.open( this->GetOutputLocation().c_str() );
    out = &outStream;
  }

  if ( !out->good() )
  {
    mitkThrow() << "Stream not good.";
  }


  const std::string& locale = "C";
  const std::string& currLocale = setlocale( LC_ALL, nullptr );
  setlocale(LC_ALL, locale.c_str());

  std::locale previousLocale(out->getloc());
  std::locale I("C");
  out->imbue(I);

  try
  {
    std::string filename = this->GetOutputLocation().c_str();
    mitk::TractographyForest::ConstPointer input = dynamic_cast<const mitk::TractographyForest*>(this->GetInput());

    MITK_INFO << "Saving forest to " << filename;
    vigra::rf_export_HDF5( *input->GetForest(), filename, "" );

    setlocale(LC_ALL, currLocale.c_str());
    MITK_INFO << "Forest saved successfully.";
  }
  catch(...)
  {
    setlocale(LC_ALL, currLocale.c_str());
    MITK_INFO << "Forest could not be saved.";
    throw;
  }
}
