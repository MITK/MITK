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
#include "mitkDICOMTagCache.h"

#include "mitkImage.h"
#include "mitkProperties.h"
#include "mitkWeakPointer.h"
#include "mitkIPropertyProvider.h"

#include "mitkGantryTiltInformation.h"

#include <unordered_map>

namespace mitk
{

  struct DICOMCachedValueInfo
  {
    unsigned int TimePoint;
    unsigned int SliceInTimePoint;
    std::string Value;
  };

  class DICOMCachedValueLookupTable : public GenericLookupTable< DICOMCachedValueInfo >
  {
  public:
    typedef DICOMCachedValueLookupTable Self;
    typedef GenericLookupTable< DICOMCachedValueInfo >   Superclass;
    const char *GetNameOfClass() const override
    {
      return "DICOMCachedValueLookupTable";
    }

    DICOMCachedValueLookupTable() {}
    Superclass& operator=(const Superclass& other) override { return Superclass::operator=(other); }
    ~DICOMCachedValueLookupTable() override {}
  };

  /**
     \ingroup DICOMReaderModule
     \brief Output descriptor for DICOMFileReader.

     As a result of analysis by a mitk::DICOMFileReader, this class
     describes the properties of a single mitk::Images that could
     be loaded by the file reader.

     The descriptor contains the following information:
     - the mitk::Image itself. This will be nullptr after analysis and only be present after actual loading.
     - a list of frames (mostly: filenames) that went into composition of the mitk::Image.
     - an assessment of the reader's ability to load this set of files (ReaderImplementationLevel)
     - this can be used for reader selection when one reader is able to load an image with correct colors and the other is able to produce only gray values, for example
     - description of aspects of the image. Mostly a key-value list implemented by means of mitk::PropertyList.
     - for specific keys and possible values, see documentation of specific readers.

     \note an mitk::Image may both consist of multiple files (the "old" DICOM way) or a mitk::Image may be described by a single DICOM file or even only parts of a DICOM file (the newer multi-frame DICOM classes). To reflect this DICOMImageFrameList describes a list of frames from different or a single file.

     Described aspects of an image are:
     - whether pixel spacing is meant to be in-patient or on-detector (mitk::PixelSpacingInterpretation)
     - details about a possible gantry tilt (intended for use by file readers, may be hidden later)
     */
  class MITKDICOMREADER_EXPORT DICOMImageBlockDescriptor: public IPropertyProvider
  {
  public:

    DICOMImageBlockDescriptor();
    virtual ~DICOMImageBlockDescriptor();

    DICOMImageBlockDescriptor(const DICOMImageBlockDescriptor& other);
    DICOMImageBlockDescriptor& operator=(const DICOMImageBlockDescriptor& other);

    static DICOMTagList GetTagsOfInterest();

    /// List of frames that constitute the mitk::Image (DICOMImageFrame%s)
    void SetImageFrameList(const DICOMImageFrameList& framelist);
    /// List of frames that constitute the mitk::Image (DICOMImageFrame%s)
    const DICOMImageFrameList& GetImageFrameList() const;

    /// The 3D mitk::Image that is loaded from the DICOM files of a DICOMImageFrameList
    void SetMitkImage(Image::Pointer image);
    /// the 3D mitk::Image that is loaded from the DICOM files of a DICOMImageFrameList
    Image::Pointer GetMitkImage() const;

    /// Reader's capability to appropriately load this set of frames
    ReaderImplementationLevel GetReaderImplementationLevel() const;
    /// Reader's capability to appropriately load this set of frames
    void SetReaderImplementationLevel(const ReaderImplementationLevel& level);

    /// Key-value store describing aspects of the image to be loaded
    void SetProperty(const std::string& key, BaseProperty* value);
    /// Key-value store describing aspects of the image to be loaded
    BaseProperty* GetProperty(const std::string& key) const;

    /// Convenience function around GetProperty()
    std::string GetPropertyAsString(const std::string&) const;

    /// Convenience function around SetProperty()
    void SetFlag(const std::string& key, bool value);
    /// Convenience function around GetProperty()
    bool GetFlag(const std::string& key, bool defaultValue) const;

    /// Convenience function around SetProperty()
    void SetIntProperty(const std::string& key, int value);
    /// Convenience function around GetProperty()
    int GetIntProperty(const std::string& key, int defaultValue) const;

    virtual BaseProperty::ConstPointer GetConstProperty(const std::string &propertyKey,
      const std::string &contextName = "",
      bool fallBackOnDefaultContext = true) const override;

    virtual std::vector<std::string> GetPropertyKeys(const std::string &contextName = "",
      bool includeDefaultContext = false) const override;

    virtual std::vector<std::string> GetPropertyContextNames() const override;

  private:

    // For future implementation: load slice-by-slice, mark this using these methods
    void SetSliceIsLoaded(unsigned int index, bool isLoaded);
    // For future implementation: load slice-by-slice, mark this using these methods
    bool IsSliceLoaded(unsigned int index) const;
    // For future implementation: load slice-by-slice, mark this using these methods
    bool AllSlicesAreLoaded() const;

  public:

    /// Describe how the mitk::Image's pixel spacing should be interpreted
    PixelSpacingInterpretation GetPixelSpacingInterpretation() const;

    /// Describe the correct x/y pixel spacing of the mitk::Image (which some readers might need to adjust after loading)
    void GetDesiredMITKImagePixelSpacing(ScalarType& spacingXinMM, ScalarType& spacingYinMM) const;

    /// Describe the gantry tilt of the acquisition
    void SetTiltInformation(const GantryTiltInformation& info);
    /// Describe the gantry tilt of the acquisition
    const GantryTiltInformation GetTiltInformation() const;

    /// SOP Class UID of this set of frames
    void SetSOPClassUID(const std::string& uid);
    /// SOP Class UID of this set of frames
    std::string GetSOPClassUID() const;
    /// SOP Class as human readable name (e.g. "CT Image Storage")
    std::string GetSOPClassUIDAsName() const;

    /**Convinience method that returns the property timesteps*/
    int GetNumberOfTimeSteps() const;
    /**return the number of frames that constitute one timestep.*/
    int GetNumberOfFramesPerTimeStep() const;

    void SetTagCache(DICOMTagCache* privateCache);

    /** Type specifies additional tags of interest. Key is the tag path of interest.
    * The value is an optional user defined name for the property that should be used to store the tag value(s).
    * Empty value is default and will imply to use the found DICOMTagPath as property name.*/
    typedef std::map<DICOMTagPath, std::string> AdditionalTagsMapType;
    /**
    * \brief Set a list of DICOMTagPaths that specifiy all DICOM-Tags that will be copied into the property of the mitk::Image.
    *
    * This method can be used to specify a list of DICOM-tags that shall be available after the loading.
    * The value in the tagMap is an optional user defined name for the property key that should be used
    * when storing the property). Empty value is default and will imply to use the found DICOMTagPath
    * as property key.
    * By default the content of the DICOM tags will be stored in a StringLookupTable on the mitk::Image.
    * This behaviour can be changed by setting a different TagLookupTableToPropertyFunctor via
    * SetTagLookupTableToPropertyFunctor().
    */
    void SetAdditionalTagsOfInterest(const AdditionalTagsMapType& tagMap);

    typedef std::function<mitk::BaseProperty::Pointer(const DICOMCachedValueLookupTable&) > TagLookupTableToPropertyFunctor;

    /**
    * \brief Set a functor that defines how the slice-specific tag-values are stored in a Property.
    *
    * This method sets a functor that is given a StringLookupTable that contains the values of one DICOM tag
    * mapped to the slice index.
    * The functor is supposed to store these values in an mitk Property.
    *
    * By default, the StringLookupTable is stored in a StringLookupTableProperty except if all values are
    * identical. In this case, the unique value is stored only once in a StringProperty.
    */
    void SetTagLookupTableToPropertyFunctor(TagLookupTableToPropertyFunctor);


    /// Print information about this image block to given stream
    void Print(std::ostream& os, bool filenameDetails) const;

  private:

    // read values from tag cache
    std::string GetPixelSpacing() const;
    std::string GetImagerPixelSpacing() const;

    Image::Pointer FixupSpacing(Image* mitkImage);
    Image::Pointer DescribeImageWithProperties(Image* mitkImage);
    void UpdateImageDescribingProperties() const;

    static mitk::BaseProperty::Pointer GetPropertyForDICOMValues(const DICOMCachedValueLookupTable& cacheLookupTable);

    double stringtodouble(const std::string& str) const;
    DICOMImageFrameList m_ImageFrameList;

    Image::Pointer m_MitkImage;
    BoolList m_SliceIsLoaded;
    ReaderImplementationLevel m_ReaderImplementationLevel;

    GantryTiltInformation m_TiltInformation;

    PropertyList::Pointer m_PropertyList;

    mitk::WeakPointer<DICOMTagCache> m_TagCache;

    mutable bool m_PropertiesOutOfDate;

    AdditionalTagsMapType m_AdditionalTagMap;
    std::set<std::string> m_FoundAdditionalTags;

    TagLookupTableToPropertyFunctor m_PropertyFunctor;
  };

}

#endif
