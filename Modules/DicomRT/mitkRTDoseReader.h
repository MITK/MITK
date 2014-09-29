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


#ifndef mitkDicomRTReader_h
#define mitkDicomRTReader_h

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <mitkCommon.h>
#include <MitkDicomRTExports.h>

#include "dcmtk/dcmrt/drtdose.h"

#include <itkGDCMSeriesFileNames.h>
#include <mitkDicomSeriesReader.h>
#include <mitkImage.h>

namespace mitk
{

  class MitkDicomRT_EXPORT RTDoseReader: public itk::Object
  {

  public:

    mitkClassMacro( RTDoseReader, itk::Object );
    itkNewMacro( Self );

    template<typename TPixel, unsigned int VImageDimension>
    void MultiplayGridScaling( itk::Image< TPixel, VImageDimension>* image, Float32 gridscale);

    /**
     * @brief Get the maximum dose value from the dose file
     * @param dataSet The DcmDataset of the DicomRTDose file
     * @return  The dose value
     *
     * Checks all pixel values for the maximum value
     */
    double GetMaxDoseValue(DcmDataset* dataSet);

    /**
     * @brief Reads a DcmDataset from a DicomRT dose file
     * @param dataset  DcmDataset-object from DCMTK
     * @param filename The path with the dose file used for getting the geometry
     * @return Returns a mitkDataNode::Pointer in which a mitk::Image is stored
     *
     * The method reads the PixelData from the DicomRT dose file and scales
     * them with a factor for getting Gray-values instead of pixel-values.
     * The Gray-values are stored in a mitkImage with a vtkColorTransferFunc.
     * Relative values are used for coloring the image. The relative values are
     * relative to a PrescriptionDose definied in the RT-Plan. If there is no
     * RT-Plan file PrescriptionDose is set to 80% of the maximum dose.
     */
    mitk::DataNode::Pointer LoadRTDose(const char* filename);

    /**
    * Virtual destructor.
    */
    virtual ~RTDoseReader();

  protected:

    /**
    * Constructor.
    */
    RTDoseReader();

  };

}

#endif
