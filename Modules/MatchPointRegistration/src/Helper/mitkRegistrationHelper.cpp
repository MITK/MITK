/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "mitkRegistrationHelper.h"
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateOr.h>
#include <mitkPointSet.h>
#include <mitkLabelSetImage.h>

//MatchPoint
#include "mapRegistrationKernel.h"

namespace mitk
{
  mitk::TNodePredicateDataType<mitk::MAPRegistrationWrapper>::ConstPointer InternalRegNodePredicate = mitk::TNodePredicateDataType<mitk::MAPRegistrationWrapper>::New().GetPointer();
  mitk::TNodePredicateDataType<mitk::Image>::ConstPointer InternalImageNodePredicate = mitk::TNodePredicateDataType<mitk::Image>::New().GetPointer();
  mitk::TNodePredicateDataType<mitk::PointSet>::ConstPointer InternalPointSetNodePredicate = mitk::TNodePredicateDataType<mitk::PointSet>::New().GetPointer();


  MITKRegistrationHelper::Affine3DTransformType::Pointer
  MITKRegistrationHelper::
  getAffineMatrix(const mitk::MAPRegistrationWrapper* wrapper, bool inverseKernel)
  {
    Affine3DTransformType::Pointer result = nullptr;

    if (wrapper)
      {
        result = getAffineMatrix(wrapper->GetRegistration(), inverseKernel);
      }
    return result;
  }

  MITKRegistrationHelper::Affine3DTransformType::Pointer
  MITKRegistrationHelper::
  getAffineMatrix(const RegistrationBaseType* registration, bool inverseKernel)
  {
    Affine3DTransformType::Pointer result = nullptr;

    if (registration && is3D(registration))
      {
        const Registration3DType* pReg = dynamic_cast<const Registration3DType*>(registration);
        if (pReg)
          {
            if (inverseKernel)
              {
                result = getAffineMatrix(pReg->getInverseMapping());
              }
            else
              {
                result = getAffineMatrix(pReg->getDirectMapping());
              }
          }
      }
    return result;
  }


  MITKRegistrationHelper::Affine3DTransformType::Pointer MITKRegistrationHelper::getAffineMatrix(const RegistrationKernel3DBase& kernel)
  {
    Affine3DTransformType::Pointer result = nullptr;

    typedef ::map::core::RegistrationKernel<3,3> KernelType;

    const KernelType* pModelKernel = dynamic_cast<const KernelType*>(&kernel);

    if (pModelKernel)
      {

        KernelType::TransformType::MatrixType matrix;
        KernelType::TransformType::OutputVectorType offset;

        if(pModelKernel->getAffineMatrixDecomposition(matrix,offset))
          {
            result = Affine3DTransformType::New();
            Affine3DTransformType::MatrixType resultMatrix;
            Affine3DTransformType::OffsetType resultOffset;

            /**@TODO If MatchPoint and MITK get same scalar
              values the casting of matrix and offset values is
              obsolete and should be removed.*/
            //The conversion of matrix and offset is needed
            //because mitk uses float and MatchPoint currently
            //double as scalar value.
            for (unsigned int i=0; i<matrix.GetVnlMatrix().size(); ++i)
              {
                resultMatrix.GetVnlMatrix().begin()[i] = static_cast<mitk::ScalarType>(matrix.GetVnlMatrix().begin()[i]);
              }
            resultOffset.CastFrom(offset); //needed because mitk uses float and MatchPoint currently double as scalar value

            result->SetMatrix(resultMatrix);
            result->SetOffset(resultOffset);
          }
      }
    return result;
  }

  bool MITKRegistrationHelper::is3D(const mitk::MAPRegistrationWrapper* wrapper)
  {
    bool result = false;

    if (wrapper)
      {
        result = wrapper->GetMovingDimensions()==3 && wrapper->GetTargetDimensions()==3;
      }
    return result;
  }

  bool MITKRegistrationHelper::is3D(const RegistrationBaseType* reBase)
  {
    bool result = false;

    if (reBase)
      {
        result = reBase->getMovingDimensions()==3 && reBase->getTargetDimensions()==3;
      }
    return result;
  }

  bool MITKRegistrationHelper::IsRegNode(const mitk::DataNode* node)
  {
    if (!node) return false;

    return InternalRegNodePredicate->CheckNode(node);
  }

  NodePredicateBase::ConstPointer MITKRegistrationHelper::RegNodePredicate()
  {
    return InternalRegNodePredicate.GetPointer();
  }

  NodePredicateBase::ConstPointer MITKRegistrationHelper::ImageNodePredicate()
  {
    return InternalImageNodePredicate.GetPointer();
  }

  NodePredicateBase::ConstPointer MITKRegistrationHelper::PointSetNodePredicate()
  {
    return InternalPointSetNodePredicate.GetPointer();
  }

  NodePredicateBase::ConstPointer MITKRegistrationHelper::MaskNodePredicate()
  {
    auto isLabelSetImage = mitk::TNodePredicateDataType<mitk::LabelSetImage>::New();
    auto hasBinaryProperty = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    auto isLegacyMask = mitk::NodePredicateAnd::New(ImageNodePredicate(), hasBinaryProperty);

    auto isLabelSetOrLegacyMask = mitk::NodePredicateOr::New(isLabelSetImage, isLegacyMask);

    return isLabelSetOrLegacyMask.GetPointer();
  }

}
