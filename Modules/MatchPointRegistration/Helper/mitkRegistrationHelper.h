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


#ifndef _mitkRegistrationHelper_h
#define _mitkRegistrationHelper_h

//ITK
#include "itkScalableAffineTransform.h"

//MatchPoint
#include "mapRegistrationAlgorithmBase.h"
#include "mapRegistration.h"

//MITK
#include <mitkImage.h>
#include <mitkDataNode.h>

//MITK
#include "MitkMatchPointRegistrationExports.h"
#include "mitkMAPRegistrationWrapper.h"

namespace mitk
{
/*!
  \brief MITKRegistrationHelper
  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.
*/
class MITKMATCHPOINTREGISTRATION_EXPORT MITKRegistrationHelper
{
public:

  typedef ::itk::ScalableAffineTransform< ::mitk::ScalarType,3 > Affine3DTransformType;
  typedef ::map::core::Registration<3,3> Registration3DType;
  typedef ::map::core::RegistrationBase RegistrationBaseType;

  /** Extracts the affine transformation, if possible, of the selected kernel.
   @param wrapper Pointer to the registration that is target of the extraction
   @param inverseKernel Indicates from which kernel the matrix should be extract. True: inverse kernel, False: direct kernel.
   @return Pointer to the extracted transform. If it is not possible to convert the kernel into an affine transform a null pointer is returned.
   @pre wrapper must point to a valid instance.
   @pre wrapper must be a 3D-3D registration.*/
  static Affine3DTransformType::Pointer getAffineMatrix(const mitk::MAPRegistrationWrapper* wrapper, bool inverseKernel);
  static Affine3DTransformType::Pointer getAffineMatrix(const RegistrationBaseType* registration, bool inverseKernel);

  static bool is3D(const mitk::MAPRegistrationWrapper* wrapper);
  static bool is3D(const RegistrationBaseType* regBase);

  /** Checks if the passed Node contains a MatchPoint registration
   @param Pointer to the node to be checked.*
   @return true: node contains a MAPRegistrationWrapper. false: "node" does not point to a valid instance or does not contain
    a registration wrapper.*/;
  static bool IsRegNode(const mitk::DataNode* node);

private:
    typedef ::map::core::Registration<3,3>::DirectMappingType RegistrationKernel3DBase;
    static Affine3DTransformType::Pointer getAffineMatrix(const RegistrationKernel3DBase& kernel);

    MITKRegistrationHelper();
    ~MITKRegistrationHelper();
    MITKRegistrationHelper& operator = (const MITKRegistrationHelper&);
    MITKRegistrationHelper(const MITKRegistrationHelper&);

};

}

#endif

