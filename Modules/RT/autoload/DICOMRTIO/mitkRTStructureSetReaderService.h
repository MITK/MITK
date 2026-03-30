/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRTStructureSetReaderService_h
#define mitkRTStructureSetReaderService_h

#include <mitkAbstractFileReader.h>

#include <MitkDICOMRTIOExports.h>
#include <mitkContourModelSet.h>

#include <usModuleContext.h>

namespace mitk
{
  /**
   * \brief Reader service for DICOM files of modality RT Structure Set.
   *
   * Reads RT Structure Set DICOM files and converts each ROI (region of
   * interest) into a mitk::ContourModelSet.
   */
  class MITKDICOMRTIO_EXPORT RTStructureSetReaderService : public mitk::AbstractFileReader
  {

    /**
     * \brief Represents a single region of interest (ROI) from the structure set.
     */
    class RoiEntry
    {
    public:
      RoiEntry();
      RoiEntry(const RoiEntry& src);
      virtual ~RoiEntry();
      RoiEntry& operator=(const RoiEntry& src);

      void SetPolyData(ContourModelSet::Pointer roiPolyData);

      unsigned int Number;
      std::string  Name;
      std::string  Description;
      double       DisplayColor[3];
      mitk::ContourModelSet::Pointer ContourModelSet;
    };

  public:
    /** \brief Default constructor. Registers reader for the RT Structure Set MIME type. */
    RTStructureSetReaderService();

    /** \brief Copy constructor. */
    RTStructureSetReaderService(const RTStructureSetReaderService& other);

    ~RTStructureSetReaderService() override;

    /**
     * \brief Read an RT Structure Set from a DICOM file and return the ROIs as ContourModelSets.
     *
     * One ContourModelSet represents one ROI. A ContourModelSet contains
     * ContourModels which represent the individual structures.
     */
    using AbstractFileReader::Read;

  protected:
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    RTStructureSetReaderService* Clone() const override;

    /**
     * containing the ROIs meta information like name number and description
     */
    std::vector<RoiEntry> ROISequenceVector;

    /**
     * Returns the number of ROIs from the ROISequenceVector
     */
    size_t GetNumberOfROIs() const;

    /**
     * Returns the relevant ROI from the ROISequenceVector by its number
     */
    RoiEntry* FindRoiByNumber(unsigned int roiNum);

    us::ServiceRegistration<mitk::IFileReader> m_FileReaderServiceReg;
  };
}

#endif
