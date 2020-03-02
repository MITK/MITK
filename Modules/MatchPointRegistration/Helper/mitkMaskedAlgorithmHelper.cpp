/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMaskedAlgorithmHelper.h"

#include <itkImageMaskSpatialObject.h>

// Mitk
#include <mitkImageAccessByItk.h>

// MatchPoint
#include "mapMaskedRegistrationAlgorithmInterface.h"
#include <mapRegistrationAlgorithmInterface.h>

namespace mitk
{

  MaskedAlgorithmHelper::MaskedAlgorithmHelper(map::algorithm::RegistrationAlgorithmBase* algorithm) : m_AlgorithmBase(algorithm)
  {
  }

  bool MaskedAlgorithmHelper::HasMaskedRegistrationAlgorithmInterface(const map::algorithm::RegistrationAlgorithmBase* algorithm)
  {
    using MaskedInterface2D = const ::map::algorithm::facet::MaskedRegistrationAlgorithmInterface<2, 2>;
    using MaskedInterface3D = const ::map::algorithm::facet::MaskedRegistrationAlgorithmInterface<3, 3>;

    return dynamic_cast<MaskedInterface2D*>(algorithm) != nullptr && dynamic_cast<MaskedInterface3D*>(algorithm) != nullptr;
  };

  bool
    MaskedAlgorithmHelper::
    CheckSupport(const mitk::Image* movingMask, const mitk::Image* targetMask) const
  {
    if (! m_AlgorithmBase) mapDefaultExceptionStaticMacro(<< "Error, cannot check data. Helper has no algorithm defined.");

    unsigned int movingDim = m_AlgorithmBase->getMovingDimensions();
    unsigned int targetDim = m_AlgorithmBase->getTargetDimensions();

    bool result = movingDim == targetDim;

    if ( movingMask)
    {
      result = result && (movingMask->GetDimension() == movingDim);
    }

    if ( targetMask)
    {
      result = result && (targetMask->GetDimension() == targetDim);
    }

    if (movingDim == 2)
    {
      typedef ::map::algorithm::facet::MaskedRegistrationAlgorithmInterface<2, 2> MaskedInterface;
      const MaskedInterface* pMaskedReg = dynamic_cast<const MaskedInterface*>(m_AlgorithmBase.GetPointer());

      result = result && pMaskedReg;
    }
    else if (movingDim == 3)
    {
      typedef ::map::algorithm::facet::MaskedRegistrationAlgorithmInterface<3, 3> MaskedInterface;
      const MaskedInterface* pMaskedReg = dynamic_cast<const MaskedInterface*>(m_AlgorithmBase.GetPointer());

      result = result && pMaskedReg;
    }
    else
    {
      result = false;
    }

    return result;
  };

  bool MaskedAlgorithmHelper::SetMasks(const mitk::Image* movingMask, const mitk::Image* targetMask)
  {
    if (! m_AlgorithmBase) mapDefaultExceptionStaticMacro(<< "Error, cannot set data. Helper has no algorithm defined.");

    if (! CheckSupport(movingMask, targetMask)) return false;

    unsigned int movingDim = m_AlgorithmBase->getMovingDimensions();
    unsigned int targetDim = m_AlgorithmBase->getTargetDimensions();

    if (movingDim!=targetDim) return false;

    if (movingDim == 2)
    {
      return DoSetMasks<2,2>(movingMask, targetMask);
    }
    else if (movingDim == 3)
    {
      return DoSetMasks<3,3>(movingMask, targetMask);
    }
    return false;
  };

  template<unsigned int VMovingDimension, unsigned int VTargetDimension>
  bool MaskedAlgorithmHelper::DoSetMasks(const mitk::Image* movingMask, const mitk::Image* targetMask)
  {
    typedef itk::SpatialObject<VMovingDimension> MovingSpatialType;
    typedef itk::SpatialObject<VTargetDimension> TargetSpatialType;

    typedef ::map::algorithm::facet::MaskedRegistrationAlgorithmInterface<VMovingDimension, VTargetDimension> MaskedRegInterface;
    MaskedRegInterface* pAlg = dynamic_cast<MaskedRegInterface*>(m_AlgorithmBase.GetPointer());

    if (!pAlg) return false;

    if (movingMask)
    {
      AccessFixedDimensionByItk(movingMask, DoConvertMask, VMovingDimension);
      typename MovingSpatialType::Pointer movingSpatial = dynamic_cast<MovingSpatialType*>(m_convertResult.GetPointer());
      if (!movingSpatial) mapDefaultExceptionStaticMacro(<< "Error, cannot convert moving mask.");
      pAlg->setMovingMask(movingSpatial);
    }
    else
    {
      pAlg->setMovingMask(nullptr);
    }

    if (targetMask)
    {
      AccessFixedDimensionByItk(targetMask, DoConvertMask, VTargetDimension);
      typename TargetSpatialType::Pointer targetSpatial = dynamic_cast<TargetSpatialType*>(m_convertResult.GetPointer());
      if (! targetSpatial) mapDefaultExceptionStaticMacro(<< "Error, cannot convert moving mask.");
      pAlg->setTargetMask(targetSpatial);
    }
    else
    {
      pAlg->setTargetMask(nullptr);
    }

    return true;
  }

  template<unsigned int VImageDimension>
  typename itk::SpatialObject<VImageDimension>::Pointer
    MaskedAlgorithmHelper::ConvertMaskSO(const itk::Image<MaskPixelType, VImageDimension>* mask) const
  {
    typedef itk::ImageMaskSpatialObject<VImageDimension> SpatialType;

    typename SpatialType::Pointer spatial = SpatialType::New();
    spatial->SetImage(mask);

    return spatial.GetPointer();
  }

  template<typename TPixelType, unsigned int VImageDimension>
  void MaskedAlgorithmHelper::DoConvertMask(const itk::Image<TPixelType,VImageDimension>* mask)
  {
    using InImageType = itk::Image<TPixelType, VImageDimension>;
    using MaskImageType = itk::Image<MaskPixelType, VImageDimension>;

    typedef itk::CastImageFilter< InImageType, MaskImageType > CastFilterType;
    typename CastFilterType::Pointer  imageCaster = CastFilterType::New();

    imageCaster->SetInput(mask);

    auto castedMask = imageCaster->GetOutput();
    imageCaster->Update();
    m_convertResult = ConvertMaskSO<VImageDimension>(castedMask);
  }

  template<unsigned int VImageDimension>
  void MaskedAlgorithmHelper::DoConvertMask(const itk::Image<MaskPixelType, VImageDimension>* mask)
  {
    m_convertResult = ConvertMaskSO<VImageDimension>(mask);
  }
}
