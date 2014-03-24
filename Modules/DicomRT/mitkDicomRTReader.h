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
#include "mitkContourModel.h"
#include <mitkContourModelSet.h>

#include "dcmtk/dcmrt/drtstrct.h"
#include "dcmtk/dcmrt/drtdose.h"
#include "dcmtk/dcmrt/drtimage.h"

#include <mitkProperties.h>
#include <itkGDCMSeriesFileNames.h>
#include <mitkDicomSeriesReader.h>
#include <DataStructures/mitkRTConstants.h>

class vtkPolyData;
class DcmDataset;
class OFString;
class DRTContourSequence;
class DRTStructureSetIOD;

namespace mitk
{

  class MitkDicomRT_EXPORT DicomRTReader: public itk::Object
  {

    typedef std::deque<mitk::ContourModelSet::Pointer> ContourModelSetVector;

    /**
     * @brief Describes and holds some information about the beams.
     */
    class BeamEntry
    {
    public:
      BeamEntry()
      {
        Number=-1;
        SrcAxisDistance=0.0;
        GantryAngle=0.0;
        PatientSupportAngle=0.0;
        BeamLimitingDeviceAngle=0.0;
        LeafJawPositions[0][0]=0.0;
        LeafJawPositions[0][1]=0.0;
        LeafJawPositions[1][0]=0.0;
        LeafJawPositions[1][1]=0.0;
      }
      unsigned int Number;
      std::string Name;
      std::string Type;
      std::string Description;
      double SrcAxisDistance;
      double GantryAngle;
      double PatientSupportAngle;
      double BeamLimitingDeviceAngle;
      double LeafJawPositions[2][2];
    };

    /**
     * @brief Describes and holds some information about the Rois.
     */
    class RoiEntry
    {
    public:
      RoiEntry();
      virtual ~RoiEntry();
      RoiEntry(const RoiEntry& src);
      RoiEntry &operator=(const RoiEntry &src);

      void SetPolyData(ContourModelSet::Pointer roiPolyData);

      unsigned int Number;
      std::string  Name;
      std::string  Description;
      double       DisplayColor[3];
      mitk::ContourModelSet::Pointer ContourModelSet;
    };

  public:

    mitkClassMacro( DicomRTReader, itk::Object );
    itkNewMacro( Self );

    /**
     * @brief Get the maximum dose value from the dose file
     * @param dataSet The DcmDataset of the DicomRTDose file
     * @return  The dose value
     *
     * Checks all pixel values for the maximum value
     */
    double GetMaxDoseValue(DcmDataset* dataSet);

    /**
     * @brief Used for reading a DicomRT file
     * @param filename The path with your file which you want to read
     *
     * Calls the right method for reading a dose, structure or plan file.
     */
    ContourModelSetVector ReadDicomFile(char* filename);

    /**
     * @brief Reads a DcmDataset from a DicomRT structureset file
     * @param dataset DcmDataset-object from DCMTK
     * @return Returns a Deque with mitk::ContourModelSet
     *
     * The returned mitk::ContourModelSet represent exactly one
     * Roi/Structureset.
     * So the size of the returned deque is the number of Rois. The names of the
     * rois is stored in their mitk::Property.
     */
    ContourModelSetVector ReadStructureSet(DcmDataset* dataset);

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
    mitk::DataNode::Pointer LoadRTDose(DcmDataset* dataset, char* filename);

    /**
     * @brief Returns the number of Rois stored in the RoiSequenceVector
     * @return unsigned long size_t Number of Rois
     */
    size_t GetNumberOfRois();

    /**
     * @brief Find a Roi stored in the RoiSequenceVector by his number
     * @param roiNumber The number of the searched roi
     * @return Returns a mitk::DicomRTReader::RoiEntry object
     */
    RoiEntry* FindRoiByNumber(unsigned int roiNum);

    /**
     * @brief GetReferencedFrameOfReferenceSOPInstanceUID
     * @param structSetObject
     * @return
     */
    OFString GetRefFrameOfRefSOPInstanceUID
                            (DRTStructureSetIOD &structSetObject);

    /**
    * Virtual destructor.
    */
    virtual ~DicomRTReader();

  protected:

    /**
     * @brief Storing the Rois found in the Structureset file
     */
    std::vector<RoiEntry> RoiSequenceVector;

    /**
     * @brief Storing the Beams foud in the RT Plan file
     */
    std::vector<BeamEntry> BeamSequenceVector;

    /**
    * Constructor.
    */
    DicomRTReader();

  };

}

#endif
