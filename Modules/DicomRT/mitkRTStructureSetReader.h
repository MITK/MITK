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

#ifndef MITKRTSTRUCTURESETREADER_H
#define MITKRTSTRUCTURESETREADER_H

#include <itkObject.h>
#include <itkObjectFactory.h>

#include <MitkDicomRTExports.h>
#include <mitkContourModelSet.h>
#include <mitkProperties.h>

#include "dcmtk/dcmrt/drtstrct.h"

#include <mitkDataNode.h>

namespace mitk
{
  class MitkDicomRT_EXPORT RTStructureSetReader: public itk::Object
  {
    typedef std::deque<mitk::DataNode::Pointer> ContourModelSetNodes;

    /**
     * Represent a region of interest (ROI)
     */
    class RoiEntry
    {
      public:
        RoiEntry();
        RoiEntry( const RoiEntry& src);
        virtual ~RoiEntry();
        RoiEntry &operator=(const RoiEntry &src);

        void SetPolyData(ContourModelSet::Pointer roiPolyData);

        unsigned int Number;
        std::string  Name;
        std::string  Description;
        double       DisplayColor[3];
        mitk::ContourModelSet::Pointer ContourModelSet;
    };

  public:
    mitkClassMacro( RTStructureSetReader, itk::Object )
    itkNewMacro( Self )

    /**
     * @brief Reading a RT StructureSet from the DICOM file and returns the ROIs
     * (region of interest) as a ContourModelSet. One ContourModelSet represent
     * one ROI. A ContourModelSet contains ContourModels which represent the
     * single structures.
     * @param filepath to the RT StructureSet (.dmc) file
     * @return Returns a std::deque filled with ContourModelSet::Pointer the
     * deque contains all ROIs from the DICOM file
     */
    ContourModelSetNodes ReadStructureSet(const char* filepath);

  protected:
    /**
     * containing the ROIs meta information like name number and description
     */
    std::vector<RoiEntry> ROISequenceVector;

    RTStructureSetReader();
    virtual ~RTStructureSetReader();

    /**
     * Returns the number of ROIs from the ROISequenceVector
     */
    size_t GetNumberOfROIs();

    /**
     * Returns the relevant ROI from the ROISequenceVector by its number
     */
    RoiEntry* FindRoiByNumber(unsigned int roiNum);
  };
}

#endif // MITKRTSTRUCTURESETREADER_H
