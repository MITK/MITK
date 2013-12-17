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

#ifndef mitkDICOMImageBlockDescriptor_h
#define mitkDICOMImageBlockDescriptor_h

#include "mitkDICOMEnums.h"
#include "mitkDICOMImageFrameInfo.h"
#include "mitkDICOMTag.h"

#include "mitkImage.h"
#include "mitkProperties.h"

#include "mitkGantryTiltInformation.h"

namespace mitk
{

class DICOMReader_EXPORT DICOMImageBlockDescriptor
{
  public:

    DICOMImageBlockDescriptor();
    ~DICOMImageBlockDescriptor();

    DICOMImageBlockDescriptor(const DICOMImageBlockDescriptor& other);
    DICOMImageBlockDescriptor& operator=(const DICOMImageBlockDescriptor& other);

    void SetImageFrameList(const DICOMImageFrameList& framelist);
    const DICOMImageFrameList& GetImageFrameList() const;

    void SetProperty(const std::string& key, BaseProperty* value);
    BaseProperty* GetProperty(const std::string& key) const;
    void SetFlag(const std::string& key, bool value);
    bool GetFlag(const std::string& key, bool defaultValue) const;
    void SetIntProperty(const std::string& key, int value);
    int GetIntProperty(const std::string& key, int defaultValue) const;

    void SetMitkImage(Image::Pointer image);
    Image::Pointer GetMitkImage() const;

    void SetSliceIsLoaded(unsigned int index, bool isLoaded);
    bool IsSliceLoaded(unsigned int index) const;
    bool AllSlicesAreLoaded() const;

    void SetPixelsInterpolated(bool pixelsAreInterpolated);
    bool GetPixelsInterpolated() const;

    void SetPixelSpacingInterpretation( PixelSpacingInterpretation interpretation );
    PixelSpacingInterpretation GetPixelSpacingInterpretation() const;

    void SetPixelSpacingInformation(const std::string& pixelSpacing, const std::string& imagerPixelSpacing);
    void GetDesiredMITKImagePixelSpacing( ScalarType& spacingX, ScalarType& spacingY) const;

    void SetTiltInformation(const GantryTiltInformation& info);
    const GantryTiltInformation GetTiltInformation() const;

  private:

    DICOMImageFrameList m_ImageFrameList;
    Image::Pointer m_MitkImage;
    BoolList m_SliceIsLoaded;
    bool m_PixelsInterpolated;
    PixelSpacingInterpretation m_PixelSpacingInterpretation;

    std::string m_PixelSpacing;
    std::string m_ImagerPixelSpacing;

    GantryTiltInformation m_TiltInformation;

    PropertyList::Pointer m_PropertyList;
};

}

#endif
