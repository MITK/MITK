/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCollectionGrayDilation_H
#define mitkCollectionGrayDilation_H

// MITK
#include "mitkCommon.h"
#include <MitkTumorInvasionAnalysisExports.h>
#include <mitkDataCollection.h>

namespace mitk
{
  class MITKTUMORINVASIONANALYSIS_EXPORT CollectionGrayOpening
  {
  public:
    // typedef unsigned char BinaryType;
    typedef itk::Image<mitk::ScalarType, 3> ImageType;

    CollectionGrayOpening() {}
    ~CollectionGrayOpening() {}
    /**
     * @brief PerformGrayOpening - Opening operation on a specific modality type wihtin the DataCollection. Creates a
     * new item.
     * @param dataCollection
     * @param name
     * @param suffix
     */
    static void PerformGrayOpening(mitk::DataCollection *dataCollection, std::string name, std::string suffix);

  protected:
  private:
    // DATA

    // FUNCTIONS
  };
} // end namespace
#endif
