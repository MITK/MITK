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

namespace mitk
{
  class MitkDicomRT_EXPORT RTStructureSetReader: public itk::Object
  {
    typedef std::deque<mitk::ContourModelSet::Pointer> ContourModelSetVector;

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
    mitkClassMacro( RTStructureSetReader, itk::Object );
    itkNewMacro( Self );

    ContourModelSetVector ReadStructureSet(const char* filepath);

  protected:
    std::vector<RoiEntry> ROISequenceVector;

    RTStructureSetReader();
    virtual ~RTStructureSetReader();

    size_t GetNumberOfROIs();
    RoiEntry* FindRoiByNumber(unsigned int roiNum);
  };
}

#endif // MITKRTSTRUCTURESETREADER_H
