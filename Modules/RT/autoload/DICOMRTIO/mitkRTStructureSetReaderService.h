/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKRTSTRUCTURESETREADER_H
#define MITKRTSTRUCTURESETREADER_H

#include <mitkAbstractFileReader.h>

#include "MitkDICOMRTIOExports.h"
#include <mitkContourModelSet.h>

#include <usModuleContext.h>

namespace mitk
{
  class MITKDICOMRTIO_EXPORT RTStructureSetReaderService : public mitk::AbstractFileReader
  {

    /**
     * Represent a region of interest (ROI)
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
    RTStructureSetReaderService();
    RTStructureSetReaderService(const RTStructureSetReaderService& other);

    ~RTStructureSetReaderService() override;

    /**
    * @brief Reading a RT StructureSet from the DICOM file and returns the ROIs
    * (region of interest) as a ContourModelSet. One ContourModelSet represent
    * one ROI. A ContourModelSet contains ContourModels which represent the
    * single structures.
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

#endif // MITKRTSTRUCTURESETREADER_H
