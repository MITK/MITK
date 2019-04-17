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

#include "mitkAlgorithmHelper.h"

//itk
#include <itkImageDuplicator.h>

// Mitk
#include <mitkImageAccessByItk.h>
#include <mitkPointSetMappingHelper.h>

// MatchPoint
#include <mapImageRegistrationAlgorithmInterface.h>
#include <mapRegistrationAlgorithmInterface.h>
#include <mapPointSetRegistrationAlgorithmInterface.h>
#include <mapDummyImageRegistrationAlgorithm.h>
#include <mapAlgorithmIdentificationInterface.h>

namespace mitk
{

  MITKAlgorithmHelper::MITKAlgorithmHelper(map::algorithm::RegistrationAlgorithmBase *algorithm)
    : m_AlgorithmBase(algorithm), m_Error(CheckError::none)
  {
    m_AllowImageCasting = true;
  }

  bool MITKAlgorithmHelper::HasImageAlgorithmInterface(const map::algorithm::RegistrationAlgorithmBase* algorithm)
  {
    using InternalDefault2DImageType = itk::Image<map::core::discrete::InternalPixelType, 2>;
    using InternalDefault3DImageType = itk::Image<map::core::discrete::InternalPixelType, 3>;

    using Alg2DType = const ::map::algorithm::facet::ImageRegistrationAlgorithmInterface<InternalDefault2DImageType, InternalDefault2DImageType>; 
    if (dynamic_cast<Alg2DType*>(algorithm) != nullptr) return true;
    using Alg3DType = const ::map::algorithm::facet::ImageRegistrationAlgorithmInterface<InternalDefault3DImageType, InternalDefault3DImageType>;
    if (dynamic_cast<Alg3DType*>(algorithm) != nullptr) return true;
    using Alg2D3DType = const ::map::algorithm::facet::ImageRegistrationAlgorithmInterface<InternalDefault2DImageType, InternalDefault3DImageType>;
    if (dynamic_cast<Alg2D3DType*>(algorithm) != nullptr) return true;
    using Alg3D2DType = const ::map::algorithm::facet::ImageRegistrationAlgorithmInterface<InternalDefault3DImageType, InternalDefault2DImageType>;
    if (dynamic_cast<Alg3D2DType*>(algorithm) != nullptr) return true;

    return false;
  };

  bool MITKAlgorithmHelper::HasPointSetAlgorithmInterface(const map::algorithm::RegistrationAlgorithmBase* algorithm)
  {
    typedef ::map::core::continuous::Elements<3>::InternalPointSetType InternalDefaultPointSetType;
    typedef const ::map::algorithm::facet::PointSetRegistrationAlgorithmInterface<InternalDefaultPointSetType, InternalDefaultPointSetType>
      PointSetRegInterface;

    return dynamic_cast<PointSetRegInterface*>(algorithm) != nullptr;
  };

  map::core::RegistrationBase::Pointer
  MITKAlgorithmHelper::
  GetRegistration() const
  {
    map::core::RegistrationBase::Pointer spResult;

    unsigned int movingDim = m_AlgorithmBase->getMovingDimensions();
    unsigned int targetDim = m_AlgorithmBase->getTargetDimensions();

    if (movingDim != targetDim)
    {
      mapDefaultExceptionStaticMacro( <<
                                      "Error, algorithm instance has unequal dimensionality and is therefore not supported in the current version of MITKAlgorithmHelper.");
    }

    if (movingDim > 3)
    {
      mapDefaultExceptionStaticMacro( <<
                                      "Error, algorithm instance has a dimensionality larger than 3 and is therefore not supported in the current version of MITKAlgorithmHelper.");
    }

    typedef ::map::algorithm::facet::RegistrationAlgorithmInterface<2, 2> RegistrationAlg2D2DInterface;
    typedef ::map::algorithm::facet::RegistrationAlgorithmInterface<3, 3> RegistrationAlg3D3DInterface;

    RegistrationAlg2D2DInterface* pRegAlgorithm2D2D = dynamic_cast<RegistrationAlg2D2DInterface*>
        (m_AlgorithmBase.GetPointer());
    RegistrationAlg3D3DInterface* pRegAlgorithm3D3D = dynamic_cast<RegistrationAlg3D3DInterface*>
        (m_AlgorithmBase.GetPointer());

    if (pRegAlgorithm2D2D)
    {
      spResult = pRegAlgorithm2D2D->getRegistration();
    }

    if (pRegAlgorithm3D3D)
    {
      spResult = pRegAlgorithm3D3D->getRegistration();
    }

    return spResult;
  }

  mitk::MAPRegistrationWrapper::Pointer
  MITKAlgorithmHelper::
  GetMITKRegistrationWrapper() const
  {
    map::core::RegistrationBase::Pointer spInternalResult = GetRegistration();
    mitk::MAPRegistrationWrapper::Pointer spResult = mitk::MAPRegistrationWrapper::New();
    spResult->SetRegistration(spInternalResult);
    return spResult;
  };


  static const mitk::Image* GetDataAsImage(const mitk::BaseData* data)
  {
    return dynamic_cast<const mitk::Image*>(data);
  };

  static const mitk::PointSet* GetDataAsPointSet(const mitk::BaseData* data)
  {
    return dynamic_cast<const mitk::PointSet*>(data);
  };

  bool
  MITKAlgorithmHelper::
  CheckData(const mitk::BaseData* moving, const mitk::BaseData* target, CheckError::Type& error) const
  {
    if (! m_AlgorithmBase)
    {
      mapDefaultExceptionStaticMacro( << "Error, cannot check data. Helper has no algorithm defined.");
    }

    if (! moving)
    {
      mapDefaultExceptionStaticMacro( << "Error, cannot check data. Moving data pointer is nullptr.");
    }

    if (! target)
    {
      mapDefaultExceptionStaticMacro( << "Error, cannot check data. Target data pointer is nullptr.");
    }

    bool result = false;
    m_Error = CheckError::unsupportedDataType;

    unsigned int movingDim = m_AlgorithmBase->getMovingDimensions();
    unsigned int targetDim = m_AlgorithmBase->getTargetDimensions();

    if (movingDim != targetDim)
    {
      m_Error = CheckError::wrongDimension;
    }
    else
    {
      //First check if data are point sets or images
      if (GetDataAsPointSet(target) && GetDataAsPointSet(moving))
      {
        typedef ::map::core::continuous::Elements<3>::InternalPointSetType InternalDefaultPointSetType;
        typedef ::map::algorithm::facet::PointSetRegistrationAlgorithmInterface<InternalDefaultPointSetType, InternalDefaultPointSetType>
        PointSetRegInterface;

        PointSetRegInterface* pPointSetInterface = dynamic_cast<PointSetRegInterface*>
            (m_AlgorithmBase.GetPointer());

        if (!pPointSetInterface)
        {
          result = false;
          m_Error = CheckError::unsupportedDataType;
        }
      }
      else if (GetDataAsImage(moving) && GetDataAsImage(target))
      {
        if (movingDim == 2)
        {
          AccessTwoImagesFixedDimensionByItk(GetDataAsImage(moving), GetDataAsImage(target), DoCheckImages,
                                             2);
        }
        else if (movingDim == 3)
        {
          AccessTwoImagesFixedDimensionByItk(GetDataAsImage(moving), GetDataAsImage(target), DoCheckImages,
                                             3);
        }
        else
        {
          m_Error = CheckError::wrongDimension;
        }

        if (m_Error == CheckError::none || (m_AllowImageCasting && m_Error == CheckError::onlyByCasting))
        {
          result = true;
        }
      }

    }

    error = m_Error;
    return result;

  };

  void MITKAlgorithmHelper::SetAllowImageCasting(bool allowCasting)
  {
    this->m_AllowImageCasting = allowCasting;
  };

  bool MITKAlgorithmHelper::GetAllowImageCasting() const
  {
    return this->m_AllowImageCasting;
  };

  void MITKAlgorithmHelper::SetData(const mitk::BaseData* moving, const mitk::BaseData* target)
  {
    if (! m_AlgorithmBase)
    {
      mapDefaultExceptionStaticMacro( << "Error, cannot check data. Helper has no algorithm defined.");
    }

    if (! moving)
    {
      mapDefaultExceptionStaticMacro( << "Error, cannot check data. Moving data pointer is nullptr.");
    }

    if (! target)
    {
      mapDefaultExceptionStaticMacro( << "Error, cannot check data. Target data pointer is nullptr.");
    }

    unsigned int movingDim = m_AlgorithmBase->getMovingDimensions();
    unsigned int targetDim = m_AlgorithmBase->getTargetDimensions();

    if (movingDim != targetDim)
    {
      mapDefaultExceptionStaticMacro( <<
                                      "Error, cannot set data. Current version of MITKAlgorithmHelper only supports images/point sets with same dimensionality.");
    }

    if (GetDataAsPointSet(target) && GetDataAsPointSet(moving))
    {
      typedef ::map::core::continuous::Elements<3>::InternalPointSetType InternalDefaultPointSetType;
      typedef ::map::algorithm::facet::PointSetRegistrationAlgorithmInterface<InternalDefaultPointSetType, InternalDefaultPointSetType>
      PointSetRegInterface;

      PointSetRegInterface* pPointSetInterface = dynamic_cast<PointSetRegInterface*>
          (m_AlgorithmBase.GetPointer());

      pPointSetInterface->setMovingPointSet(mitk::PointSetMappingHelper::ConvertPointSetMITKtoMAP(
                                              GetDataAsPointSet(moving)->GetPointSet()));
      pPointSetInterface->setTargetPointSet(mitk::PointSetMappingHelper::ConvertPointSetMITKtoMAP(
                                              GetDataAsPointSet(target)->GetPointSet()));
    }
    else if (GetDataAsImage(moving) && GetDataAsImage(target))
    {
      if (movingDim == 2)
      {
        AccessTwoImagesFixedDimensionByItk(GetDataAsImage(moving), GetDataAsImage(target), DoSetImages, 2);
      }
      else if (movingDim == 3)
      {
        AccessTwoImagesFixedDimensionByItk(GetDataAsImage(moving), GetDataAsImage(target), DoSetImages, 3);
      }
    }
  };

  template<typename TInImageType, typename TOutImageType>
  typename TOutImageType::Pointer MITKAlgorithmHelper::CastImage(const TInImageType* input) const
  {
    typedef itk::CastImageFilter< TInImageType, TOutImageType > CastFilterType;
    typename CastFilterType::Pointer  spImageCaster =  CastFilterType::New();

    spImageCaster->SetInput(input);

    typename TOutImageType::Pointer spImage = spImageCaster->GetOutput();
    spImageCaster->Update();

    return spImage;
  }

  template<typename TPixelType1, unsigned int VImageDimension1,
           typename TPixelType2, unsigned int VImageDimension2>
  void MITKAlgorithmHelper::DoSetImages(const itk::Image<TPixelType1, VImageDimension1>* moving,
                                        const itk::Image<TPixelType2, VImageDimension2>* target)
  {
    typedef itk::Image<TPixelType1, VImageDimension1> MovingImageType;
    typedef itk::Image<TPixelType2, VImageDimension2> TargetImageType;
    typedef itk::Image<map::core::discrete::InternalPixelType, VImageDimension1>
    InternalDefaultMovingImageType;
    typedef itk::Image<map::core::discrete::InternalPixelType, VImageDimension2>
    InternalDefaultTargetImageType;

    typedef ::map::algorithm::facet::ImageRegistrationAlgorithmInterface<MovingImageType, TargetImageType>
    ImageRegInterface;
    typedef ::map::algorithm::facet::ImageRegistrationAlgorithmInterface<InternalDefaultMovingImageType, InternalDefaultTargetImageType>
    DefaultImageRegInterface;


    ImageRegInterface* pImageInterface = dynamic_cast<ImageRegInterface*>(m_AlgorithmBase.GetPointer());
    DefaultImageRegInterface* pDefaultImageInterface = dynamic_cast<DefaultImageRegInterface*>
        (m_AlgorithmBase.GetPointer());

    if (pImageInterface)
    {
      //just set directly and you are done

      /**@todo the duplication work arround is needed due to a insufficuence
       in the AccessTwoImagesFixedDimensionByItk macro. The macro always cast
       the passed image into non const (even if tha image was passed as const).
       This behavior enforces the unnecessary use of an writeaccessor, which as a consequence
       will lead to redundant access exceptions as long as the algorithm exists;
       e.g. in the typical scenario with the MatchPoint Plugins*/
      typedef itk::ImageDuplicator< MovingImageType > MovingDuplicatorType;
      typedef itk::ImageDuplicator< TargetImageType > TargetDuplicatorType;
      typename MovingDuplicatorType::Pointer mDuplicator = MovingDuplicatorType::New();
      mDuplicator->SetInputImage(moving);
      mDuplicator->Update();

      typename TargetDuplicatorType::Pointer tDuplicator = TargetDuplicatorType::New();
      tDuplicator->SetInputImage(target);
      tDuplicator->Update();

      typename MovingImageType::Pointer clonedMoving = mDuplicator->GetOutput();
      typename TargetImageType::Pointer clonedTarget = tDuplicator->GetOutput();

      pImageInterface->setTargetImage(clonedTarget);
      pImageInterface->setMovingImage(clonedMoving);
    }
    else if (pDefaultImageInterface)
    {
      //you may convert it to the default image type and use it then
      if (! m_AllowImageCasting)
      {
        mapDefaultExceptionStaticMacro( <<
                                        "Error, cannot set images. MITKAlgorithmHelper has to convert them into MatchPoint default images, but is not allowed. Please reconfigure helper.");
      }

      typename InternalDefaultTargetImageType::Pointer spCastedTarget =
        CastImage<TargetImageType, InternalDefaultTargetImageType>(target);
      typename InternalDefaultMovingImageType::Pointer spCastedMoving =
        CastImage<MovingImageType, InternalDefaultMovingImageType>(moving);
      pDefaultImageInterface->setTargetImage(spCastedTarget);
      pDefaultImageInterface->setMovingImage(spCastedMoving);
    }
    else
    {
      mapDefaultExceptionStaticMacro( << "Error, algorithm is not able to use the based images.");
    }
  }

  template<typename TPixelType1, unsigned int VImageDimension1,
           typename TPixelType2, unsigned int VImageDimension2>
  void MITKAlgorithmHelper::DoCheckImages(const itk::Image<TPixelType1, VImageDimension1>* /*moving*/,
                                          const itk::Image<TPixelType2, VImageDimension2>* /*target*/) const
  {
    typedef itk::Image<TPixelType1, VImageDimension1> MovingImageType;
    typedef itk::Image<TPixelType2, VImageDimension2> TargetImageType;
    typedef itk::Image<map::core::discrete::InternalPixelType, VImageDimension1>
    InternalDefaultMovingImageType;
    typedef itk::Image<map::core::discrete::InternalPixelType, VImageDimension2>
    InternalDefaultTargetImageType;

    typedef ::map::algorithm::facet::ImageRegistrationAlgorithmInterface<MovingImageType, TargetImageType>
    ImageRegInterface;
    typedef ::map::algorithm::facet::ImageRegistrationAlgorithmInterface<InternalDefaultMovingImageType, InternalDefaultTargetImageType>
    DefaultImageRegInterface;

    ImageRegInterface* pImageInterface = dynamic_cast<ImageRegInterface*>(m_AlgorithmBase.GetPointer());
    DefaultImageRegInterface* pDefaultImageInterface = dynamic_cast<DefaultImageRegInterface*>
        (m_AlgorithmBase.GetPointer());

    if (pImageInterface)
    {
      //just set directly and you are done
      m_Error = CheckError::none;
    }
    else if (pDefaultImageInterface)
    {
      //you may convert it to the default image type and use it then
      m_Error = CheckError::onlyByCasting;
    }
    else
    {
      m_Error = CheckError::unsupportedDataType;
    }
  }


  mapGenerateAlgorithmUIDPolicyMacro(DummyRegIDPolicy, "de.dkfz.dipp", "Identity", "1.0.0", "");

  mitk::MAPRegistrationWrapper::Pointer GenerateIdentityRegistration3D()
  {
      typedef map::algorithm::DummyImageRegistrationAlgorithm<map::core::discrete::Elements<3>::InternalImageType, map::core::discrete::Elements<3>::InternalImageType, DummyRegIDPolicy>
          DummyRegType;
      DummyRegType::Pointer regAlg = DummyRegType::New();
      mitk::MITKAlgorithmHelper helper(regAlg);

      map::core::discrete::Elements<3>::InternalImageType::Pointer dummyImg =
          map::core::discrete::Elements<3>::InternalImageType::New();
      dummyImg->Allocate();
      regAlg->setTargetImage(dummyImg);
      regAlg->setMovingImage(dummyImg);

      mitk::MAPRegistrationWrapper::Pointer dummyReg = mitk::MAPRegistrationWrapper::New();
      dummyReg->SetRegistration(regAlg->getRegistration());

      return dummyReg;
  }

}
