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

#ifndef mitkCollectionGrayDilation_H
#define mitkCollectionGrayDilation_H

//MITK
#include "mitkCommon.h"
#include <mitkDataCollection.h>
#include <MitkTumorInvasionAnalysisExports.h>


namespace mitk
{
class MITKTUMORINVASIONANALYSIS_EXPORT CollectionGrayOpening
{
public:

  //typedef unsigned char BinaryType;
  typedef double BinaryType;
  typedef itk::Image<BinaryType, 3> BinaryImage;



  CollectionGrayOpening(){}
  ~CollectionGrayOpening(){}

  static void PerformGrayOpening(mitk::DataCollection *dataCollection, std::string name, std::string suffix);

protected:

private:
  // DATA

  // FUNCTIONS
};
} // end namespace
#endif
