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

#include <vtkColorTransferFunction.h>
#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>

#include <mitkIOUtil.h>
#include <mitkDataNodeFactory.h>
#include <mitkImagePixelWriteAccessor.h>
#include <mitkImageAccessByItk.h>
#include <itkImageIterator.h>

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
    ContourModelSet=mitk::ContourModelSet::New();
  }

  DicomRTReader::RoiEntry::~RoiEntry(){}

  DicomRTReader::RoiEntry::RoiEntry(const RoiEntry& src)
  {
    Number=src.Number;
    Name=src.Name;
    Description=src.Description;
    DisplayColor[0]=src.DisplayColor[0];
    DisplayColor[1]=src.DisplayColor[1];
    DisplayColor[2]=src.DisplayColor[2];
    ContourModelSet=mitk::ContourModelSet::New();
    SetPolyData(src.ContourModelSet);
  }

  DicomRTReader::RoiEntry& DicomRTReader::RoiEntry::
        operator=(const RoiEntry &src)
  {
    Number=src.Number;
    Name=src.Name;
    Description=src.Description;
    DisplayColor[0]=src.DisplayColor[0];
    DisplayColor[1]=src.DisplayColor[1];
    DisplayColor[2]=src.DisplayColor[2];
    SetPolyData(src.ContourModelSet);
    return (*this);
  }

  void DicomRTReader::RoiEntry::
        SetPolyData(mitk::ContourModelSet::Pointer roiPolyData)
  {
    if (roiPolyData == this->ContourModelSet)
      return;
    this->ContourModelSet = roiPolyData;
  }

  std::deque<mitk::ContourModelSet::Pointer> DicomRTReader::
        ReadDicomFile(char* filename)
  {
    DcmFileFormat file;
    OFCondition outp;
    outp = file.loadFile(filename, EXS_Unknown);
    if(outp.good())
    {
      DcmDataset *dataset = file.getDataset();
      OFString sopClass;
      if(dataset->findAndGetOFString
              (DCM_SOPClassUID, sopClass).good() && !sopClass.empty())
      {
        if(sopClass == UID_RTDoseStorage)
        {
          mitk::DataNode::Pointer x = mitk::DataNode::New();
          x = this->DicomRTReader::LoadRTDose(dataset, filename);
          ContourModelSetVector y;
          return y;
        }
        else if(sopClass == UID_RTStructureSetStorage)
        {
          ContourModelSetVector x =
                  this->DicomRTReader::ReadStructureSet(dataset);
          return x;
        }
        else
        {
          MITK_ERROR << "Error matching SOP Class, maybe an unsupported type"
                     << endl;
          ContourModelSetVector y;
          return y;
        }
      }
      else
      {
        MITK_ERROR << "Error reading the SOPClassID" << endl;
        ContourModelSetVector y;
        return y;
      }
    }
    else
    {
      MITK_ERROR << "Error rading the input file" << endl;
      ContourModelSetVector y;
      return y;
    }
  }

  size_t DicomRTReader::GetNumberOfRois()
  {
    return this->RoiSequenceVector.size();
  }

  DicomRTReader::RoiEntry* DicomRTReader::FindRoiByNumber(unsigned int roiNum)
  {
    for(unsigned int i=0; i<this->RoiSequenceVector.size(); ++i)
    {
      if(this->RoiSequenceVector[i].Number == roiNum)
        return &this->RoiSequenceVector[i];
    }
    return NULL;
  }

  std::deque<mitk::ContourModelSet::Pointer> DicomRTReader::
        ReadStructureSet(DcmDataset* dataset)
  {
    /**
     * @brief For storing contourmodelsets that belongs to the same object
     *
     * e.g. An eye consists of several contourmodels (contourmodel consists
     * of several 3D-Points) and together they are a contourmodelset
     */
    ContourModelSetVector contourModelSetVector;

    DRTStructureSetIOD structureSetObject;
    OFCondition outp = structureSetObject.read(*dataset);
    if(!outp.good())
    {
      MITK_ERROR << "Error reading the file" << endl;
      std::deque<mitk::ContourModelSet::Pointer> x;
      return x;
    }
    DRTStructureSetROISequence &roiSequence =
            structureSetObject.getStructureSetROISequence();
    if(!roiSequence.gotoFirstItem().good())
    {
      MITK_ERROR << "Error reading the structure sequence" << endl;
      std::deque<mitk::ContourModelSet::Pointer> x;
      return x;
    }
    do{
      DRTStructureSetROISequence::Item &currentSequence =
              roiSequence.getCurrentItem();
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

    Sint32 refRoiNumber;
    DRTROIContourSequence &roiContourSeqObject =
            structureSetObject.getROIContourSequence();
    if(!roiContourSeqObject.gotoFirstItem().good())
    {
      MITK_ERROR << "Error reading the contour sequence" << endl;
      std::deque<mitk::ContourModelSet::Pointer> x;
      return x;
    }
    do
    {
      mitk::ContourModelSet::Pointer contourSet = mitk::ContourModelSet::New();
      DRTROIContourSequence::Item &currentRoiObject =
              roiContourSeqObject.getCurrentItem();
      if(!currentRoiObject.isValid())
      {
        continue;
      }
      currentRoiObject.getReferencedROINumber(refRoiNumber);
      DRTContourSequence &contourSeqObject =
              currentRoiObject.getContourSequence();

      if(contourSeqObject.gotoFirstItem().good())
      {
        do
        {
          DRTContourSequence::Item &contourItem =
                  contourSeqObject.getCurrentItem();
          if(!contourItem.isValid())
          {
            continue;
          }
          int number;
          OFString contourNumber;
          OFString numberOfPoints;
          OFVector<Float64> contourData_LPS;
          mitk::ContourModel::Pointer contourSequence =
                  mitk::ContourModel::New();

          contourItem.getContourNumber(contourNumber);
          contourItem.getNumberOfContourPoints(numberOfPoints);
          contourItem.getContourData(contourData_LPS);

          std::stringstream stream;
          stream << numberOfPoints;
          stream >> number;

          for(unsigned int i=0; i<contourData_LPS.size()/3;i++)
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
        MITK_ERROR << "Error reading contourSeqObject" << endl;
      }
      RoiEntry* refROI = this->FindRoiByNumber(refRoiNumber);
      if(refROI==NULL)
      {
        MITK_ERROR << "Can not find references ROI" << endl;
        continue;
      }
      Sint32 roiColor;
      for(int j=0;j<3;j++)
      {
        currentRoiObject.getROIDisplayColor(roiColor, j);
        refROI->DisplayColor[j] = roiColor/255.0;
      }
      refROI->ContourModelSet = contourSet;
      contourSet->SetProperty("name", mitk::StringProperty::New(refROI->Name));
      contourModelSetVector.push_back(contourSet);

    }
    while(roiContourSeqObject.gotoNextItem().good());

    return contourModelSetVector;
  }

  OFString DicomRTReader::
        GetRefFrameOfRefSOPInstanceUID(DRTStructureSetIOD &structSetObject)
  {
    OFString invalid;
    DRTReferencedFrameOfReferenceSequence &refFrameOfRefSeqObject =
            structSetObject.getReferencedFrameOfReferenceSequence();
    if(!refFrameOfRefSeqObject.gotoFirstItem().good())
      return invalid;
    DRTReferencedFrameOfReferenceSequence::Item &currentRefFrameOfRefSeqItem =
            refFrameOfRefSeqObject.getCurrentItem();
    if(!currentRefFrameOfRefSeqItem.isValid())
      return invalid;
    DRTRTReferencedStudySequence &refStudySeqObject =
            currentRefFrameOfRefSeqItem.getRTReferencedStudySequence();
    if(!refStudySeqObject.gotoFirstItem().good())
      return invalid;
    DRTRTReferencedStudySequence::Item &refStudySeqItem =
            refStudySeqObject.getCurrentItem();
    if(!refStudySeqItem.isValid())
      return invalid;
    DRTRTReferencedSeriesSequence &refSeriesSeqObject =
            refStudySeqItem.getRTReferencedSeriesSequence();
    if(!refSeriesSeqObject.gotoFirstItem().good())
      return invalid;
    DRTRTReferencedSeriesSequence::Item &refSeriesSeqItem =
            refSeriesSeqObject.getCurrentItem();
    if(!refSeriesSeqItem.isValid())
      return invalid;
    DRTContourImageSequence &contourImageSeqObject =
            refSeriesSeqItem.getContourImageSequence();
    if(!contourImageSeqObject.gotoFirstItem().good())
      return invalid;
    DRTContourImageSequence::Item &contourImageSeqItem =
            contourImageSeqObject.getCurrentItem();
    if(!contourImageSeqItem.isValid())
      return invalid;

    OFString resultUid;
    contourImageSeqItem.getReferencedSOPInstanceUID(resultUid);

    return resultUid;
  }

  mitk::DataNode::Pointer DicomRTReader::
        LoadRTDose(DcmDataset* dataset, char* filename)
  {
    std::string name = filename;
    itk::FilenamesContainer file;
    file.push_back(name);

    mitk::DicomSeriesReader* reader = new mitk::DicomSeriesReader;

    mitk::DataNode::Pointer originalNode = reader->LoadDicomSeries(file,false);
    mitk::Image::Pointer originalImage =
            dynamic_cast<mitk::Image*>(originalNode->GetData());

    mitk::Geometry3D::Pointer geo = originalImage->GetGeometry()->Clone();

    DRTDoseIOD doseObject;

    OFCondition result = doseObject.read(*dataset);
    if(result.bad())
    {
      MITK_ERROR << "Error reading the Dataset" << endl;
      return 0;
    }

    Uint16 rows, columns, frames, planarConfig, samplesPP;
    OFString nrframes, doseUnits, doseType, summationType,
            gridScaling, photoInterpret, lutShape;
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

    dataset->findAndGetUint16Array(DCM_PixelData, pixelData, &count);

    mitk::Image::Pointer image = mitk::Image::New();
    mitk::PixelType pt = mitk::MakeScalarPixelType<float>();
    unsigned int dim[] = {columns,rows,frames};

    image->Initialize( pt, 3, dim);
    image->SetSpacing(1.0);
    mitk::Point3D m_origin;
    m_origin[0] = 0.0;
    m_origin[1] = 0.0;
    m_origin[2] = 0.0;
    image->SetOrigin(m_origin);

//    mitk::IOUtil* io = new mitk::IOUtil();
    mitk::DataNodeFactory::Pointer nodeReader = mitk::DataNodeFactory::New();
    nodeReader->SetFileName(name);
    nodeReader->Update();
    mitk::DataNode::Pointer superNode = nodeReader->GetOutput();
    mitk::Image::Pointer superImage = dynamic_cast<mitk::Image*>(superNode->GetData());

    MITK_INFO << "SuperImagePixelType: " << superImage->GetPixelType().GetTypeAsString() << endl;

    mitk::ImagePixelWriteAccessor<unsigned int,3> accessor(superImage);
    unsigned int* pixelAccessData = accessor.GetData();

    AccessByItk_1(superImage, MultiplayGridScaling, gridscale);

    int pixelNumber = 300;

    for(int i=0;i<pixelNumber;++i)
    {
      *pixelAccessData *= gridscale;
      pixelAccessData++;
    }

    //HELP CAST UND DEPRECATED
    float* pixel = reinterpret_cast<float*>(image->GetData());
//    float* pixel = (float*)image->GetData();
    int size = dim[0]*dim[1]*dim[2];

//    for(int i=0; i<size; ++i, ++pixel)
//    {
//      imageData->GetScalar
//      *pixel=pixelData[i] * gridscale;
//    }

    image->SetGeometry(geo);

    double prescripeDose = this->GetMaxDoseValue(dataset);

    mitk::DataNode::Pointer node = mitk::DataNode::New();
    superNode->SetName("DicomRT Dosis");
    superNode->SetFloatProperty(mitk::rt::Constants::
                           PRESCRIBED_DOSE_PROPERTY_NAME.c_str(),prescripeDose);
    superNode->SetFloatProperty(mitk::rt::Constants::
                           REFERENCE_DOSE_PROPERTY_NAME.c_str(), 40);
    superNode->SetBoolProperty(mitk::rt::Constants::DOSE_PROPERTY_NAME.c_str(),true);
    node->SetData(image);

    return superNode;
  }

  template<typename TPixel, unsigned int VImageDimension>
  void DicomRTReader::MultiplayGridScaling(itk::Image<TPixel,VImageDimension>* image , Float32 gridscale)
  {
    typedef itk::Image<TPixel, VImageDimension> InputImageType;
    itk::ImageIterator<InputImageType> it( image, image->GetRequestedRegion() );
    for(it=it.Begin(); !it.IsAtEnd(); ++it)
    {
      it.Set(it.Get()*gridscale);
    }
  }

  double DicomRTReader::GetMaxDoseValue(DcmDataset* dataSet)
  {
    DRTDoseIOD doseObject;
    OFCondition result = doseObject.read(*dataSet);
    if(result.bad())
    {
      std::cout << "Error reading the RT Dose dataset\n\n";
      return 0;
    }

    Uint16 rows, columns, frames;
    OFString nrframes, gridScaling;
    const Uint16 *pixelData = NULL;
    Float32 gridscale;

    Uint16 &rows_ref = rows;
    Uint16 &columns_ref = columns;

    doseObject.getRows(rows_ref);
    doseObject.getColumns(columns_ref);
    doseObject.getNumberOfFrames(nrframes);
    doseObject.getDoseGridScaling(gridScaling);

    frames = atoi(nrframes.c_str());
    gridscale = OFStandard::atof(gridScaling.c_str());
    MITK_INFO << "Gridscale " << gridscale << endl;
    MITK_INFO << "As String: " << gridScaling << endl;
    dataSet->findAndGetUint16Array(DCM_PixelData, pixelData, 0);

    int size = columns*rows*frames;
    double highest = 0;

    for(int i=0; i<size; ++i)
    {
      if((pixelData[i]*gridscale)>highest)
      {
        highest = pixelData[i] * gridscale;
      }
    }

    return highest;
  }

}
