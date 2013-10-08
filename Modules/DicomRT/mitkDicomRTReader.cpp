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


#include "mitkDicomRTReader.h"

namespace mitk
{

  DicomRTReader::DicomRTReader(){}

  DicomRTReader::~DicomRTReader(){}

  DicomRTReader::RoiEntry::RoiEntry()
  {
    Number=0;
    DisplayColor[0]=1.0;
    DisplayColor[1]=0.0;
    DisplayColor[2]=0.0;
    PolyData=NULL;
  }

  DicomRTReader::RoiEntry::~RoiEntry()
  {
    SetPolyData(NULL);
  }

  DicomRTReader::RoiEntry::RoiEntry(const RoiEntry& src)
  {
    Number=src.Number;
    Name=src.Name;
    Description=src.Description;
    DisplayColor[0]=src.DisplayColor[0];
    DisplayColor[1]=src.DisplayColor[1];
    DisplayColor[2]=src.DisplayColor[2];
    PolyData=NULL;
    SetPolyData(src.PolyData);
  }

  DicomRTReader::RoiEntry& DicomRTReader::RoiEntry::operator=(const RoiEntry &src)
  {
    Number=src.Number;
    Name=src.Name;
    Description=src.Description;
    DisplayColor[0]=src.DisplayColor[0];
    DisplayColor[1]=src.DisplayColor[1];
    DisplayColor[2]=src.DisplayColor[2];
    SetPolyData(src.PolyData);
    return (*this);
  }

  void DicomRTReader::RoiEntry::SetPolyData(vtkPolyData* roiPolyData)
  {
    if (roiPolyData == this->PolyData)
    {
      // not changed
      return;
    }
    if (this->PolyData != NULL)
    {
      this->PolyData->UnRegister(NULL);
    }

    this->PolyData = roiPolyData;

    if (this->PolyData != NULL)
    {
      this->PolyData->Register(NULL);
    }
  }

  std::deque<mitk::ContourModelSet::Pointer> DicomRTReader::ReadDicomFile(char* filename)
  {
    std::cout << "\n\n" << "Filename:" << filename << "\n\n";
    DcmFileFormat file;
    OFCondition outp;
    outp = file.loadFile(filename, EXS_Unknown);
    if(outp.good())
    {
      DcmDataset *dataset = file.getDataset();
      OFString sopClass;
      if(dataset->findAndGetOFString(DCM_SOPClassUID, sopClass).good() && !sopClass.empty())
      {
        if(sopClass == UID_RTDoseStorage)
        {
          mitk::LookupTable::Pointer x = this->DicomRTReader::LoadRTDose(dataset);
          ContourModelSetVector y;
          return y;
        }
        else if(sopClass == UID_RTStructureSetStorage)
        {
          ContourModelSetVector x = this->DicomRTReader::ReadStructureSet(dataset);
          return x;
        }
        else if(sopClass == UID_RTPlanStorage)
        {
          int x = this->DicomRTReader::LoadRTPlan(dataset);
          ContourModelSetVector y;
          return y;
        }
        else
        {
          std::cout << "Error reading the RTStructureSetStorage\n\n";
          ContourModelSetVector y;
          return y;
        }
      }
      else
      {
        std::cout << "Error reading the SOPClassID\n\n";
        ContourModelSetVector y;
        return y;
      }
    }
    else
    {
      std::cout << "Cant read the input file\n\n";
      ContourModelSetVector y;
      return y;
    }
  }

  size_t DicomRTReader::GetNumberOfRois()
  {
    return this->RoiSequenceVector.size();
  }

  DicomRTReader::RoiEntry* DicomRTReader::FindRoiByNumber(int roiNumber)
  {
    for(int i=0; i<this->RoiSequenceVector.size(); i++)
    {
      if(this->RoiSequenceVector[i].Number == roiNumber)
      {
        return &this->RoiSequenceVector[i];
      }
    }
    return NULL;
  }

  std::deque<mitk::ContourModelSet::Pointer> DicomRTReader::ReadStructureSet(DcmDataset* dataset)
  {
    //For storing contourmodelsets that belongs to the same object
    //e.g. An eye consists of several contourmodels (contourmodel consists of several 3D-Points)
    //and together they are a contourmodelset
    ContourModelSetVector contourModelSetVector;

    DRTStructureSetIOD structureSetObject;
    OFCondition outp = structureSetObject.read(*dataset);
    if(!outp.good())
    {
      std::cout << "Error reading the file\n\n";
      std::deque<mitk::ContourModelSet::Pointer> x;
      return x;
    }
    DRTStructureSetROISequence &roiSequence = structureSetObject.getStructureSetROISequence();
    if(!roiSequence.gotoFirstItem().good())
    {
      std::cout << "Error reading the structure sequence\n\n";
      std::deque<mitk::ContourModelSet::Pointer> x;
      return x;
    }
    do{
      DRTStructureSetROISequence::Item &currentSequence = roiSequence.getCurrentItem();
      if(!currentSequence.isValid())
      {
        continue;
      }
      OFString roiName;
      OFString roiDescription;
      Sint32 roiNumber;
      RoiEntry roi;

      currentSequence.getROIName(roiName);
      currentSequence.getROIDescription(roiDescription);
      currentSequence.getROINumber(roiNumber);

      roi.Name = roiName.c_str();
      roi.Description = roiDescription.c_str();
      roi.Number = roiNumber;

      this->RoiSequenceVector.push_back(roi);
    }
    while(roiSequence.gotoNextItem().good());

    OFString refSOPInstUID = GetReferencedFrameOfReferenceSOPInstanceUID(structureSetObject);

    //default slice thickness -> reads the thickness from a database ?!
    double sliceThickness = 2.0;

    Sint32 refRoiNumber;
    DRTROIContourSequence &roiContourSeqObject = structureSetObject.getROIContourSequence();
    if(!roiContourSeqObject.gotoFirstItem().good())
    {
      std::cout << "Error reading the contour sequence\n\n";
      std::deque<mitk::ContourModelSet::Pointer> x;
      return x;
    }
    do
    {
      mitk::ContourModelSet::Pointer contourSet = mitk::ContourModelSet::New();
      DRTROIContourSequence::Item &currentRoiObject = roiContourSeqObject.getCurrentItem();
      if(!currentRoiObject.isValid())
      {
        continue;
      }
      currentRoiObject.getReferencedROINumber(refRoiNumber);
      DRTContourSequence &contourSeqObject = currentRoiObject.getContourSequence();

      if(contourSeqObject.gotoFirstItem().good())
      {
        do
        {
          DRTContourSequence::Item &contourItem = contourSeqObject.getCurrentItem();
          if(!contourItem.isValid())
          {
            continue;
          }
          int number;
          OFString contourNumber;
          OFString numberOfPoints;
          OFVector<Float64> contourData_LPS;
          mitk::ContourModel::Pointer contourSequence = mitk::ContourModel::New();

          contourItem.getContourNumber(contourNumber);
          contourItem.getNumberOfContourPoints(numberOfPoints);
          contourItem.getContourData(contourData_LPS);

          std::stringstream stream;
          stream << numberOfPoints;
          stream >> number;

          for(int i=0; i<contourData_LPS.size()/3;i++)
          {
            mitk::Point3D point;
            point[0] = contourData_LPS.at(3*i);
            point[1] = contourData_LPS.at(3*i+1);
            point[2] = contourData_LPS.at(3*i+2);
            contourSequence->AddVertex(point);
          }
          contourSequence->Close();
          contourSet->AddContourModel(contourSequence);
        }
        while(contourSeqObject.gotoNextItem().good());
      }
      else
      {
        std::cout << "Error reading contourSeqObject\n\n";
      }
      RoiEntry* refROI = this->FindRoiByNumber(refRoiNumber);
      if(refROI==NULL)
      {
        std::cout << "Cant find referenced ROI\n\n";
        continue;
      }
      Sint32 roiColor;
      for(int j=0;j<3;j++)
      {
        currentRoiObject.getROIDisplayColor(roiColor, j);
        //what?
        refROI->DisplayColor[j] = roiColor/255.0;
      }

      //save all contours to one set here !
      contourModelSetVector.push_back(contourSet);
//      std::cout << "Before clearing: " << contourSet->GetSize() << "\n\n";
//      contourSet->Clear();
//      std::cout << "After clearing: " << contourSet->GetSize() << "\n\n";

    }
    while(roiContourSeqObject.gotoNextItem().good());

    std::cout << "Anzahl von ROI: " << contourModelSetVector.size() << "\n\n";

    for(int i=0; i<contourModelSetVector.size(); i++)
    {
      std::cout << "dingens " << contourModelSetVector.at(i)->GetSize() << "\n\n";
    }

    return contourModelSetVector;
  }

  OFString DicomRTReader::GetReferencedFrameOfReferenceSOPInstanceUID(DRTStructureSetIOD &structSetObject)
  {
    OFString invalid;
    DRTReferencedFrameOfReferenceSequence &refFrameOfRefSeqObject = structSetObject.getReferencedFrameOfReferenceSequence();
    if(!refFrameOfRefSeqObject.gotoFirstItem().good())
    {
      return invalid;
    }
    DRTReferencedFrameOfReferenceSequence::Item &currentRefFrameOfRefSeqItem = refFrameOfRefSeqObject.getCurrentItem();
    if(!currentRefFrameOfRefSeqItem.isValid())
    {
      return invalid;
    }
    DRTRTReferencedStudySequence &refStudySeqObject = currentRefFrameOfRefSeqItem.getRTReferencedStudySequence();
    if(!refStudySeqObject.gotoFirstItem().good())
    {
      return invalid;
    }
    DRTRTReferencedStudySequence::Item &refStudySeqItem = refStudySeqObject.getCurrentItem();
    if(!refStudySeqItem.isValid())
    {
      return invalid;
    }
    DRTRTReferencedSeriesSequence &refSeriesSeqObject = refStudySeqItem.getRTReferencedSeriesSequence();
    if(!refSeriesSeqObject.gotoFirstItem().good())
    {
      return invalid;
    }
    DRTRTReferencedSeriesSequence::Item &refSeriesSeqItem = refSeriesSeqObject.getCurrentItem();
    if(!refSeriesSeqItem.isValid())
    {
      return invalid;
    }
    DRTContourImageSequence &contourImageSeqObject = refSeriesSeqItem.getContourImageSequence();
    if(!contourImageSeqObject.gotoFirstItem().good())
    {
      return invalid;
    }
    DRTContourImageSequence::Item &contourImageSeqItem = contourImageSeqObject.getCurrentItem();
    if(!contourImageSeqItem.isValid())
    {
      return invalid;
    }

    OFString resultUid;
    contourImageSeqItem.getReferencedSOPInstanceUID(resultUid);

    return resultUid;
  }

  int DicomRTReader::LoadRTPlan(DcmDataset *dataset)
  {
    DRTPlanIOD planObject;
    OFCondition result = planObject.read(*dataset);
    if(result.good())
    {
      OFString tmpString, dummyString;
      DRTBeamSequence &planeBeamSeqObject = planObject.getBeamSequence();
      if(planeBeamSeqObject.gotoFirstItem().good())
      {
        do
        {
          DRTBeamSequence::Item &currentBeamSeqItem = planeBeamSeqObject.getCurrentItem();
          if(!currentBeamSeqItem.isValid())
         {
            std::cout << "Invalid Beam Sequence \n\n";
            continue;
          }
          BeamEntry beamEntry;
          OFString beamName, beamDescription, beamType;
          Sint32 beamNumber;
          Float64 srcAxisDistance;

          currentBeamSeqItem.getBeamName(beamName);
          currentBeamSeqItem.getBeamDescription(beamDescription);
          currentBeamSeqItem.getBeamType(beamType);
          currentBeamSeqItem.getBeamNumber(beamNumber);
          currentBeamSeqItem.getSourceAxisDistance(srcAxisDistance);

          beamEntry.Name = beamName.c_str();
          beamEntry.Description = beamDescription.c_str();
          beamEntry.Type = beamType.c_str();
          beamEntry.Number = beamNumber;
          beamEntry.SrcAxisDistance = srcAxisDistance;

          DRTControlPointSequence &controlPointSeqObject = currentBeamSeqItem.getControlPointSequence();
          if(controlPointSeqObject.gotoFirstItem().good())
          {
            DRTControlPointSequence::Item &controlPointItem = controlPointSeqObject.getCurrentItem();
            if(controlPointItem.isValid())
            {
              OFVector<Float64> isocenterPosData_LPS;
              Float64 gantryAngle, patientSupportAngle, beamLimitingDeviceAngle;
              unsigned int numOfCollimatorPosItems = 0;

              controlPointItem.getIsocenterPosition(isocenterPosData_LPS);
              controlPointItem.getGantryAngle(gantryAngle);
              controlPointItem.getPatientSupportAngle(patientSupportAngle);
              controlPointItem.getBeamLimitingDeviceAngle(beamLimitingDeviceAngle);

              beamEntry.GantryAngle = gantryAngle;
              beamEntry.PatientSupportAngle = patientSupportAngle;
              beamEntry.BeamLimitingDeviceAngle = beamLimitingDeviceAngle;

              DRTBeamLimitingDevicePositionSequence &currentCollimatorPosSeqObject = controlPointItem.getBeamLimitingDevicePositionSequence();
              if(currentCollimatorPosSeqObject.gotoFirstItem().good())
              {
                do
                {
                  if(++numOfCollimatorPosItems > 2)
                  {
                    std::cout << "Number of collimator position items is higher than 2 but should be exactly 2 ...";
                    return 0;
                  }
                  DRTBeamLimitingDevicePositionSequence::Item &collimatorPositionItem = currentCollimatorPosSeqObject.getCurrentItem();
                  if(collimatorPositionItem.isValid())
                  {
                    OFString beamLimitingDeviceType;
                    OFVector<Float64> leafJawPositions;

                    collimatorPositionItem.getRTBeamLimitingDeviceType(beamLimitingDeviceType);
                    collimatorPositionItem.getLeafJawPositions(leafJawPositions);

                    if(!beamLimitingDeviceType.compare("ASYMX") || !beamLimitingDeviceType.compare("X"))
                    {
                      beamEntry.LeafJawPositions[0][0] = leafJawPositions[0];
                      beamEntry.LeafJawPositions[0][1] = leafJawPositions[1];
                    }
                    else if(!beamLimitingDeviceType.compare("ASYMY") || !beamLimitingDeviceType.compare("Y"))
                    {
                      beamEntry.LeafJawPositions[1][0] = leafJawPositions[0];
                      beamEntry.LeafJawPositions[1][1] = leafJawPositions[0];
                    }
                    else
                    {
                      std::cout << "Unknown collimator type: " << beamLimitingDeviceType << "\n\n";
                    }
                  }
                }
                while(currentCollimatorPosSeqObject.gotoNextItem().good());
              }
            }//endif controlPointItem.isValid()
          }
          this->BeamSequenceVector.push_back(beamEntry);
        }
        while(planeBeamSeqObject.gotoNextItem().good());
      }
    }
    return 1;
  }

  mitk::LookupTable::Pointer DicomRTReader::LoadRTDose(DcmDataset* dataset)
  {
    DRTDoseIOD doseObject;

//#############################################################################################################
//######################################## READING CHECK ######################################################

    OFCondition result = doseObject.read(*dataset);
    if(result.bad())
    {
      std::cout << "Error reading the RT Dose dataset\n\n";
      return 0;
    }

//#############################################################################################################
//###################################### GRID SCALING CHECK ###################################################

    OFString doseGridScaling;
    if(doseObject.getDoseGridScaling(doseGridScaling).bad())
    {
      std::cout << "Error reading the Dose Grid Scaling\n\n";
      return 0;
    }
    doseObject.setDoseGridScaling(doseGridScaling.c_str());

    std::cout << "Dose Grid Scaling: " << doseGridScaling << "\n\n";

//#############################################################################################################
//####################################### DOSE UNITS CHECK ####################################################

//    OFString doseUnits;
//    if(doseObject.getDoseUnits(doseUnits).bad())
//    {
//      std::cout << "Error reading the dose units\n\n";
//      return 0;
//    }
//    doseObject.setDoseUnits(doseUnits.c_str());

//    std::cout << "Dose Units: " << doseUnits << "\n\n";

//#############################################################################################################
//##################################### PIXEL SPACING CHECK ###################################################

    OFVector<Float64> pixelSpacingOFVector;
    if(doseObject.getPixelSpacing(pixelSpacingOFVector).bad() || pixelSpacingOFVector.size() < 2)
    {
      std::cout << "Error reading the pixel spacing for dose object\n\n";
      return 0;
    }

    for(int i=0; i<pixelSpacingOFVector.size(); i++)
    {
      std::cout << "Pixel Spacing Vector at: " << i << " -> " << pixelSpacingOFVector.at(i) << "\n";
    }
    std::cout << "\n";

//#############################################################################################################
//######################################### TESTING CHECK #####################################################

//    DcmPixelData pixelData = doseObject.getPixelData();
//    if(pixelData.isEmpty())
//    {
//      std::cout << "Error reading the pixel data \n\n";
//      return 0;
//    }
//    std::cout << "Pixel Data Length: " << pixelData.getLength() << "\n\n";


//    OFVector<Float64> imagePositionPatient;
//    doseObject.getImagePositionPatient(imagePositionPatient);
//    for(int i=0; i<imagePositionPatient.size(); i++)
//    {
//      std::cout << "Image Position Patient " << i << " : " << imagePositionPatient.at(i) << "\n\n";
//    }


//    OFVector<Float64> gridFrameOffsetVector;
//    doseObject.getGridFrameOffsetVector(gridFrameOffsetVector);
//    for(int i=0; i<gridFrameOffsetVector.size(); i++)
//    {
//      if(i < 3 || i == gridFrameOffsetVector.size() - 1 )
//      {
//        std::cout << "Grid Frame Offset Vector " << i << " : " << gridFrameOffsetVector.at(i) << "\n\n";
//      }
//    }


//    Uint16 samplesperPixel;
//    doseObject.getSamplesPerPixel(samplesperPixel);
//    std::cout << "Samples per Pixel: " << samplesperPixel << "\n\n";


    Uint16 rows, columns, frames, planarConfig, samplesPP;
    OFString nrframes, doseUnits, doseType, summationType, gridScaling, photoInterpret, lutShape;
    Uint16 &rows_ref = rows;
    Uint16 &columns_ref = columns;
    Float32 gridscale;
    const Uint16 *pixelData = NULL;
    unsigned long count = 0;

    doseObject.getRows(rows_ref);
    doseObject.getColumns(columns_ref);
    doseObject.getNumberOfFrames(nrframes);
    doseObject.getDoseUnits(doseUnits);
    doseObject.getDoseType(doseType);
    doseObject.getDoseSummationType(summationType);
    doseObject.getDoseGridScaling(gridScaling);
    doseObject.getPhotometricInterpretation(photoInterpret);
    doseObject.getPlanarConfiguration(planarConfig);
    doseObject.getSamplesPerPixel(samplesPP);
    doseObject.getPresentationLUTShape(lutShape);

    gridscale = OFStandard::atof(gridScaling.c_str());
    frames = atoi(nrframes.c_str());

    if(dataset->findAndGetUint16Array(DCM_PixelData, pixelData, &count).good())
    {
      int counter = 0;
      for(int i=0;i<frames;i++)
      {
        for(int j=0;j<rows;j++)
        {
          for(int k=0;k<columns;k++)
          {
//            std::cout << pixelData[i*rows*columns + j*columns + k] << "\n";
//            std::cout << static_cast<Float32>(pixelData[i*rows*columns + j*columns + k]) * gridscale << "\n\n";
            counter++;
          }
        }
      }
      std::cout << "Number of Frames: " << frames << "\n\n";
      std::cout << "Number of Data in file: " << counter << "\n\n";
      std::cout << "Number of Data in file expacted overall: " << rows*columns*frames << "\n\n";
      std::cout << "Number of Data in file expacted per frame: " << rows*columns << "\n\n";
      std::cout << "Resolution: " << columns << "x" << rows << "\n\n";
      std::cout << "Photometricinterpretation: " << photoInterpret << "\n\n";
      std::cout << "Planar Configuration: " << planarConfig << "\n\n";
      std::cout << "Samples per Pixel: " << samplesPP << "\n\n";
      std::cout << "LUT Shape: " << lutShape << "\n\n";
    }

//#############################################################################################################
//#############################################################################################################

    mitk::LookupTable::Pointer mitkLut = mitk::LookupTable::New();
    vtkLookupTable* vtkLut = mitkLut->GetVtkLookupTable();
    vtkLut->SetTableRange(0,1000);
    vtkLut->Build();
    //vtkLut->SetNumberOfTableValues(8);
    //                     I    R     G     B     A
    vtkLut->SetTableValue( 0 , 0.0 , 0.0 , 0.0 , 0.3 );
    vtkLut->SetTableValue( 1 , 0.5 , 0.0 , 0.0 , 0.3 );
    vtkLut->SetTableValue( 2 , 1.0 , 0.0 , 0.0 , 0.3 );
    mitk::LookupTableProperty::Pointer mitkLutProp = mitk::LookupTableProperty::New();
    mitkLutProp->SetLookupTable(mitkLut);

    return mitkLut;
  }

  bool DicomRTReader::Equals(mitk::ContourModel::Pointer first, mitk::ContourModel::Pointer second)
  {
    if(first->GetNumberOfVertices() != second->GetNumberOfVertices())
    {
      return false;
    }
    for( int i = 0; i < first->GetNumberOfVertices(); i++)
    {
      const mitk::ContourElement::VertexType* x = first->GetVertexAt(i);
      const mitk::ContourElement::VertexType* y = second->GetVertexAt(i);

      mitk::ContourElement::VertexType xx = *x;
      mitk::ContourElement::VertexType yy = *y;

      for(int j=0; j<3; j++)
      {
        if(xx.Coordinates[j] != yy.Coordinates[j])
        {
          return false;
        }
      }
    }
    return true;
  }

}
