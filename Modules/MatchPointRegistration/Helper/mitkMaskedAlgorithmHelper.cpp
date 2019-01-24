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

      if (movingDim == 2)
      {
        typedef itk::Image<MaskPixelType,2> MaskImageType;
        mitk::PixelType maskPixelType = mitk::MakePixelType<MaskImageType>();

        result = result && (maskPixelType == movingMask->GetPixelType());
      }
      else if (movingDim == 3)
      {
        typedef itk::Image<MaskPixelType,3> MaskImageType;
        mitk::PixelType maskPixelType = mitk::MakePixelType<MaskImageType>();

        result = result && (maskPixelType == movingMask->GetPixelType());
      }
    }

    if ( targetMask)
    {
      result = result && (targetMask->GetDimension() == targetDim);

      if (movingDim == 2)
      {
        typedef itk::Image<MaskPixelType,2> MaskImageType;
        mitk::PixelType maskPixelType = mitk::MakePixelType<MaskImageType>();

        result = result && (maskPixelType == targetMask->GetPixelType());
      }
      else if (movingDim == 3)
      {
        typedef itk::Image<MaskPixelType,3> MaskImageType;
        mitk::PixelType maskPixelType = mitk::MakePixelType<MaskImageType>();

        result = result && (maskPixelType == targetMask->GetPixelType());
      }
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

  template<unsigned int VImageDimension1, unsigned int VImageDimension2>
  bool MaskedAlgorithmHelper::DoSetMasks(const mitk::Image* movingMask, const mitk::Image* targetMask)
  {
    typedef itk::SpatialObject<VImageDimension1> MovingSpatialType;
    typedef itk::SpatialObject<VImageDimension2> TargetSpatialType;

    typedef ::map::algorithm::facet::MaskedRegistrationAlgorithmInterface<VImageDimension1, VImageDimension2> MaskedRegInterface;
    MaskedRegInterface* pAlg = dynamic_cast<MaskedRegInterface*>(m_AlgorithmBase.GetPointer());

    if (!pAlg) return false;

    if (movingMask)
    {
      AccessFixedTypeByItk(movingMask, DoConvertMask, (MaskPixelType), (VImageDimension1));
      typename MovingSpatialType::Pointer movingSpatial = dynamic_cast<MovingSpatialType*>(m_convertResult.GetPointer());
      if (! movingSpatial) mapDefaultExceptionStaticMacro(<< "Error, cannot convert moving mask.");
      pAlg->setMovingMask(movingSpatial);
    }

    if (targetMask)
    {
      AccessFixedTypeByItk(targetMask, DoConvertMask, (MaskPixelType), (VImageDimension2));
      typename TargetSpatialType::Pointer targetSpatial = dynamic_cast<TargetSpatialType*>(m_convertResult.GetPointer());
      if (! targetSpatial) mapDefaultExceptionStaticMacro(<< "Error, cannot convert moving mask.");
      pAlg->setTargetMask(targetSpatial);
    }

    return true;
  }

  template<typename TPixelType, unsigned int VImageDimension>
  void MaskedAlgorithmHelper::DoConvertMask(const itk::Image<TPixelType,VImageDimension>* mask)
  {
    typedef itk::ImageMaskSpatialObject<VImageDimension> SpatialType;

    typename SpatialType::Pointer spatial = SpatialType::New();
    spatial->SetImage(mask);

    m_convertResult = spatial.GetPointer();
  }

}
