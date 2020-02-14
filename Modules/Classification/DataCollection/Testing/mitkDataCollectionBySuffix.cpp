/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#pragma warning (disable : 4996)

#include "mitkTestingMacros.h"
#include "mitkDataCollection.h"
#include "mitkCollectionReader.h"
#include "mitkIOUtil.h"

#include <vtkSmartPointer.h>

int mitkDataCollectionBySuffix(int argc, char* argv[])
{
  MITK_TEST_BEGIN("DataCollectionBySuffix");

  if(argc <= 1)
  {
    MITK_ERROR << "Provide XML File to parse.";
    return -1;
  }


  std::vector<std::string> suffixes;
  suffixes.push_back("_T2_reg.nrrd");
  suffixes.push_back("_T1_reg.nrrd");
  suffixes.push_back("_ce_reg.nrrd");
  suffixes.push_back("_gtv_reg.nrrd");
  suffixes.push_back("_reg.dwi");

  mitk::DataCollection::Pointer collection = mitk::CollectionReader::FolderToCollection(argv[1],suffixes,suffixes,true);

  MITK_INFO << "Collection No of time steps: " << collection->Size();
  mitk::DataCollection* col = dynamic_cast<mitk::DataCollection*> (collection->GetData(0).GetPointer());
  MITK_INFO << "Collection No of modalities: " << col->Size();



  MITK_TEST_END();
}
