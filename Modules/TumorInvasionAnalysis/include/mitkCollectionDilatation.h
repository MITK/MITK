/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCollectionDilation_H
#define mitkCollectionDilation_H

// MITK
#include "mitkCommon.h"
#include <MitkTumorInvasionAnalysisExports.h>
#include <mitkDataCollection.h>

namespace mitk
{
  class MITKTUMORINVASIONANALYSIS_EXPORT CollectionDilation
  {
  public:
    typedef unsigned char BinaryType;
    typedef itk::Image<BinaryType, 3> BinaryImage;

    CollectionDilation() {}
    ~CollectionDilation() {}
    /**
     * @brief DilateBinaryByName - Dilate all occurances of a modality within a mitk::DataCollection
     * @param dataCollection
     * @param name - name of modality
     * @param xy - dilatation in xy direction
     * @param z - dilatation in z direction
     * @param suffix - suffix that is appended to newly created image
     */
    static void DilateBinaryByName(DataCollection *dataCollection,
                                   std::string name,
                                   unsigned int xy = 5,
                                   unsigned int z = 0,
                                   std::string suffix = "DILATE");

    /**
     * @brief ErodeBinaryByName - Erode all occurances of a modality within a mitk::DataCollection
     * @param dataCollection
     * @param name - name of modality
     * @param xy - erosion in xy direction
     * @param z - erosion in z direction
     * @param suffix - suffix that is appended to newly created image
     */
    static void ErodeBinaryByName(DataCollection *dataCollection,
                                  std::string name,
                                  unsigned int xy = 5,
                                  unsigned int z = 0,
                                  std::string suffix = "ERODE");

  protected:
  private:
    // DATA
    // FUNCTIONS
  };
} // end namespace
#endif
