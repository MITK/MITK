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
#ifndef mitkForest_cpp
#define mitkForest_cpp

#include "time.h"
#include <sstream>

#include <mitkDataCollection.h>
#include <mitkCollectionReader.h>
#include <mitkCollectionWriter.h>
#include <mitkCollectionStatistic.h>
#include <vtkSmartPointer.h>
#include <mitkIOUtil.h>
#include <sstream>

#include <mitkLRDensityEstimation.h>

int main(int argc, char* argv[])
{
  MITK_INFO << "Argc " << argc;

  //////////////////////////////////////////////////////////////////////////////
  // Read Images
  //////////////////////////////////////////////////////////////////////////////
  mitk::DataCollection::Pointer col = mitk::DataCollection::New();

  MITK_INFO << "Arg 2 " << argv[2];
  mitk::Image::Pointer sur=mitk::IOUtil::Load<mitk::Image>(argv[2]);
  col->AddData(sur.GetPointer(),"sur");
  MITK_INFO << "Arg 3 " << argv[3];
  mitk::Image::Pointer mask=mitk::IOUtil::Load<mitk::Image>(argv[3]);
  col->AddData(mask.GetPointer(),"mask");

  std::vector<std::string> modalities;
  for (int i = 4; i < argc; ++i)
  {
    MITK_INFO << "Img " << argv[i];
    std::stringstream ss;
    ss << i;
    modalities.push_back(ss.str());
    mitk::Image::Pointer img = mitk::IOUtil::Load<mitk::Image>(argv[i]);
    col->AddData(img.GetPointer(),ss.str());
  }

  mitk::LRDensityEstimation est;
  est.SetCollection(col);
  est.SetTrainMask("sur");
  est.SetTestMask("mask");
  est.SetModalities(modalities);
  est.SetWeightName("weight");
  est.Update();

  mitk::Image::Pointer w= col->GetMitkImage("weight");
  mitk::IOUtil::Save(w,argv[1]);

  return 0;
}

#endif
