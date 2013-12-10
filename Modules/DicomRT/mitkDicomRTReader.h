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
#include <mitkDicomRTExports.h>
#include "mitkContourModel.h"
#include "mitkContourElement.h"
#include <mitkContourModelSet.h>

#include "dcmtk/config/osconfig.h"
#include "dcmtk/ofstd/ofconapp.h"

#include "dcmtk/ofstd/ofcond.h"

#include "dcmtk/dcmrt/drtdose.h"
#include "dcmtk/dcmrt/drtimage.h"
#include "dcmtk/dcmrt/drtplan.h"
#include "dcmtk/dcmrt/drttreat.h"
#include "dcmtk/dcmrt/drtionpl.h"
#include "dcmtk/dcmrt/drtiontr.h"

//######################################################
#include <mitkLookupTable.h>
#include <mitkLookupTableProperty.h>
#include <vtkLookupTable.h>
#include <vtkContourFilter.h>
#include <fstream>
#include <vtkColorTransferFunction.h>
#include <mitkImage.h>
#include <mitkPixelType.h>
#include <mitkDataNode.h>
#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkProperties.h>
#include <mitkRenderingModeProperty.h>
#include <mitkDataNodeFactory.h>
#include <itkGDCMSeriesFileNames.h>
#include <mitkDicomSeriesReader.h>
#include <DataStructures/mitkRTConstants.h>

#include <mitkCoreServices.h>
#include <mitkIShaderRepository.h>
//######################################################

#include "dcmtk/dcmrt/drtstrct.h"

#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkRibbonFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkSmartPointer.h>
#include <vtkCleanPolyData.h>

class vtkPolyData;
class DcmDataset;
class OFString;
class DRTContourSequence;
class DRTStructureSetIOD;

namespace mitk
{

  class mitkDicomRT_EXPORT DicomRTReader: public itk::Object
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

      void SetPolyData(vtkPolyData* roiPolyData);

      unsigned int Number;
      std::string  Name;
      std::string  Description;
      double       DisplayColor[3];
      //vllt direkt durch die contourmodels ersetzen ?!
      vtkPolyData* PolyData;
    };

  public:

    mitkClassMacro( DicomRTReader, itk::Object );
    itkNewMacro( Self );

    /**
     * @brief Used for getting a custom Isoline
     * @param value The gray-value which the isoline represents
     * @param dataObject The vtkDataObject from the DicomRT dose image
     * @return Returns a vtkPolyData including the contour/isoline
     *
     * This method is used for a custom Isoline e.g. from the DicomRT IsoSlider.
     * You define the value where the Isoline is by setting the value as a parameter.
     */
    vtkSmartPointer<vtkPolyData> GetIsoLine(double value, vtkDataObject* dataObject);

    /**
     * @brief Used for getting the StandardIsoLines
     * @param dataObject The vtkDataObject from the DicomRT dose image
     * @return Returns a vtkPolyData including the contours/isolines
     *
     * The IsoLines should be equal to the contours of the colorwash.
     */
    vtkSmartPointer<vtkPolyData> GetStandardIsoLines(vtkDataObject* dataObject);

    /**
     * @brief Get a default prescription dose
     * @param dataSet The DcmDataset of the DicomRTDose file
     * @return  The dose value
     *
     * If you dont define a prescription dose use this method, it calculates a
     * default value by reading the dose values from the dose file and multiplys
     * the highest value with 0.8.
     */
    double GetDefaultPrescriptionDose(DcmDataset* dataSet);

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
     * The returned mitk::ContourModelSet represent exactly one Roi/Structureset.
     * So the size of the returned deque is the number of Rois. The names of the
     * rois is stored in their mitk::Property.
     */
    ContourModelSetVector ReadStructureSet(DcmDataset* dataset);

    /**
     * @brief Reads a DcmDataset from a DicomRT plan file
     * @param dataset DcmDataset-object from DCMTK
     * @return The return doesnt make senese at the moment
     *
     * This method isnt ready for use at the moment. Dont use it!
     */
    int LoadRTPlan(DcmDataset* dataset);

    /**
     * @brief Reads a DcmDataset from a DicomRT dose file
     * @param dataset  DcmDataset-object from DCMTK
     * @param filename The path with the dose file used for getting the geometry
     * @return Returns a mitk::DataNode::Pointer in which a mitk::Image is stored
     *
     * The method reads the PixelData from the DicomRT dose file and scales them
     * with a factor for getting Gray-values instead of pixel-values.
     * The Gray-values are stored in a mitk::Image with an vtkColorTransferFunction.
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
    RoiEntry* FindRoiByNumber(int roiNumber);

    /**
     * @brief GetReferencedFrameOfReferenceSOPInstanceUID
     * @param structSetObject
     * @return
     */
    OFString GetReferencedFrameOfReferenceSOPInstanceUID(DRTStructureSetIOD &structSetObject);

    /**
     * @brief Compares two mitk::ContourModel::Pointer for testing the Structuresets.
     * @param first The first mitk::ContourModel::Pointer
     * @param second The second mitk::ContourModel::Pointer
     * @return Returns true if they are equal and false if they are different
     */
    bool Equals(mitk::ContourModel::Pointer first, mitk::ContourModel::Pointer second);

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
