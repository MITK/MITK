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


#ifndef PIVOTCALIBRATION_H_HEADER_INCLUDED_
#define PIVOTCALIBRATION_H_HEADER_INCLUDED_

#include "MitkIGTExports.h"
#include <itkObject.h>
#include <itkObjectFactory.h>
#include <mitkCommon.h>
#include <mitkVector.h>
#include <mitkNavigationData.h>
#include <vector>


namespace mitk {
    /**Documentation
    * \brief Class for performing a pivot calibration out of a set of navigation datas
    * \ingroup IGT
    */
  class MITKIGT_EXPORT PivotCalibration : public itk::Object
    {
    public:
      mitkClassMacroItkParent(PivotCalibration, itk::Object);
      itkNewMacro(Self);
      void AddNavigationData(mitk::NavigationData::Pointer data);
      /** @brief Computes the pivot point and rotation/axis on the given
        *        navigation datas. You can get the results afterwards.
        * @return Returns true if the computation was successfull, false if not.
        */
      bool ComputePivotResult();

      itkGetMacro(ResultPivotPoint,mitk::Point3D);
      itkGetMacro(ResultRMSError,double);


    protected:
      PivotCalibration();
      ~PivotCalibration() override;

      std::vector<mitk::NavigationData::Pointer> m_NavigationDatas;

      bool ComputePivotPoint();
      bool ComputePivotAxis();

      mitk::Point3D m_ResultPivotPoint;
      double m_ResultRMSError;

    };
} // Ende Namespace
#endif
