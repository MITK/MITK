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


#ifndef mitkAlgorithmHelper_h
#define mitkAlgorithmHelper_h


//MatchPoint
#include "mapRegistrationAlgorithmBase.h"
#include "mapRegistrationBase.h"

//MITK
#include <mitkImage.h>
#include <mitkPointSet.h>

//MITK
#include "MitkMatchPointRegistrationExports.h"
#include "mitkMAPRegistrationWrapper.h"

namespace mitk
{
  /*!
    \brief MITKAlgorithmHelper
    \remark Current implementation is not thread-save. Just use one Helper class per registration task.
    \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.
  */
  class MITKMATCHPOINTREGISTRATION_EXPORT MITKAlgorithmHelper
  {
  public:

    MITKAlgorithmHelper(map::algorithm::RegistrationAlgorithmBase* algorithm);

    void SetData(const mitk::BaseData* moving, const mitk::BaseData* target);

    void SetAllowImageCasting(bool allowCasting);
    bool GetAllowImageCasting() const;

    static bool HasImageAlgorithmInterface(const map::algorithm::RegistrationAlgorithmBase* algorithm);
    static bool HasPointSetAlgorithmInterface(const map::algorithm::RegistrationAlgorithmBase* algorithm);


    struct CheckError
    {
      enum Type
      {
        none = 0,
        onlyByCasting = 1,
        wrongDimension = 2,
        unsupportedDataType = 3
      };
    };

    bool CheckData(const mitk::BaseData* moving, const mitk::BaseData* target,
                   CheckError::Type& error) const;

    map::core::RegistrationBase::Pointer GetRegistration() const;

    mitk::MAPRegistrationWrapper::Pointer GetMITKRegistrationWrapper() const;

    ~MITKAlgorithmHelper() {}

  private:

    MITKAlgorithmHelper& operator = (const MITKAlgorithmHelper&);
    MITKAlgorithmHelper(const MITKAlgorithmHelper&);

    /**Internal helper that casts itk images from one pixel type into an other
      (used by DoSetImages if the images have the right dimension but wrong type and AllowImageCasting is activated)*/
    template<typename TInImageType, typename TOutImageType>
    typename TOutImageType::Pointer CastImage(const TInImageType* input) const;

    /**Internal helper that is used by SetData if the data are images to set them properly.*/
    template<typename TPixelType1, unsigned int VImageDimension1,
             typename TPixelType2, unsigned int VImageDimension2>
    void DoSetImages(const itk::Image<TPixelType1, VImageDimension1>* moving,
                     const itk::Image<TPixelType2, VImageDimension2>* target);

    /**Internal helper that is used by SetData if the data are images to check if the image types are supported by the algorithm.*/
    template<typename TPixelType1, unsigned int VImageDimension1,
             typename TPixelType2, unsigned int VImageDimension2>
    void DoCheckImages(const itk::Image<TPixelType1, VImageDimension1>* moving,
                       const itk::Image<TPixelType2, VImageDimension2>* target) const;

    map::algorithm::RegistrationAlgorithmBase::Pointer m_AlgorithmBase;

    bool m_AllowImageCasting;

    mutable CheckError::Type m_Error;
  };

  /**Small helper function that generates Identity transforms in 3D.*/
  mitk::MAPRegistrationWrapper::Pointer MITKMATCHPOINTREGISTRATION_EXPORT GenerateIdentityRegistration3D();

}

#endif

