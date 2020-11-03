/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QUATERNIONAVERAGING_H_HEADER_INCLUDED_
#define QUATERNIONAVERAGING_H_HEADER_INCLUDED_

#include "MitkIGTBaseExports.h"
#include <itkObject.h>
#include <itkObjectFactory.h>
#include <mitkCommon.h>
#include <mitkQuaternion.h>
#include <mitkVector.h>
#include <vector>


namespace mitk {
    /**Documentation
    * \brief This class offers methods to average quaternions.
    * \ingroup IGT Base
    */
  class MITKIGTBASE_EXPORT QuaternionAveraging : public itk::Object
    {
    public:

      mitkClassMacroItkParent(QuaternionAveraging, itk::Object);
      itkNewMacro(Self);

      enum Mode
      {
      SimpleMean, //simple averaging over all components of the quaternion, only valid for small differences.
      Slerp       //more advanced algorithm for averaging (spherical linear interpolation), may also be used for huge differences
      };

      static mitk::Quaternion CalcAverage(const std::vector<Quaternion>& quaternions, Mode mode = SimpleMean);

    protected:
      QuaternionAveraging();
      ~QuaternionAveraging() override;

    };
} // Ende Namespace
#endif
