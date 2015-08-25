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

#ifndef mitkCollectionDilation_H
#define mitkCollectionDilation_H

//MITK
#include "mitkCommon.h"
#include <mitkDataCollection.h>
#include <MitkTumorInvasionAnalysisExports.h>


namespace mitk
{
class MITKTUMORINVASIONANALYSIS_EXPORT CollectionDilation
{
public:

  typedef unsigned char BinaryType;
  typedef itk::Image<BinaryType, 3> BinaryImage;



  CollectionDilation(){}
  ~CollectionDilation(){}

  static void DilateBinaryByName(DataCollection* dataCollection, std::string name, unsigned int xy = 5, unsigned int z =0, std::string suffix ="DILATE");

  static void ErodeBinaryByName(DataCollection* dataCollection, std::string name, unsigned int xy = 5, unsigned int z =0, std::string suffix ="ERODE");

protected:

private:
  // DATA

  // FUNCTIONS
};
} // end namespace
#endif
