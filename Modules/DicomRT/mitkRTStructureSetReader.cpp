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

#include "mitkRTStructureSetReader.h"
#include <mitkIOUtil.h>

#include <mitkBaseRenderer.h>

namespace mitk
{
  RTStructureSetReader::RTStructureSetReader(){}

  RTStructureSetReader::~RTStructureSetReader(){}

  RTStructureSetReader::RoiEntry::RoiEntry()
  {
    Number = 0;
    DisplayColor[0]=1.0;
    DisplayColor[1]=0.0;
    DisplayColor[2]=0.0;
    ContourModelSet=mitk::ContourModelSet::New();
  }

  RTStructureSetReader::RoiEntry::RoiEntry(const RoiEntry &src)
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

  RTStructureSetReader::RoiEntry::~RoiEntry(){}

  RTStructureSetReader::RoiEntry& RTStructureSetReader::
      RoiEntry::operator =(const RoiEntry &src)
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

  void RTStructureSetReader::RoiEntry::
      SetPolyData(mitk::ContourModelSet::Pointer roiPolyData)
  {
    if (roiPolyData == this->ContourModelSet)
      return;
    this->ContourModelSet = roiPolyData;
  }

  size_t RTStructureSetReader::GetNumberOfROIs()
  {
    return this->ROISequenceVector.size();
  }

  RTStructureSetReader::RoiEntry* RTStructureSetReader::
      FindRoiByNumber(unsigned int roiNum)
  {
    for(unsigned int i=0; i<this->ROISequenceVector.size(); ++i)
    {
      if(this->ROISequenceVector[i].Number == roiNum)
        return &this->ROISequenceVector[i];
    }
    return NULL;
  }

  RTStructureSetReader::ContourModelSetNodes RTStructureSetReader::
      ReadStructureSet(const char* filepath)
  {
    DcmFileFormat file;
    OFCondition output = file.loadFile(filepath, EXS_Unknown);
    if(output.bad())
    {
      MITK_ERROR << "Cant read the file" << std::endl;
    }
    DcmDataset *dataset = file.getDataset();

    DRTStructureSetIOD structureSetObject;
    OFCondition outp = structureSetObject.read(*dataset);
    if(!outp.good())
    {
      MITK_ERROR << "Error reading the file" << std::endl;
      RTStructureSetReader::ContourModelSetNodes x;
      return x;
    }
    DRTStructureSetROISequence &roiSequence =
            structureSetObject.getStructureSetROISequence();
    if(!roiSequence.gotoFirstItem().good())
    {
      MITK_ERROR << "Error reading the structure sequence" << std::endl;
      RTStructureSetReader::ContourModelSetNodes x;
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

      this->ROISequenceVector.push_back(roi);
    }
    while(roiSequence.gotoNextItem().good());

    Sint32 refRoiNumber;
    DRTROIContourSequence &roiContourSeqObject =
            structureSetObject.getROIContourSequence();
    if(!roiContourSeqObject.gotoFirstItem().good())
    {
      MITK_ERROR << "Error reading the contour sequence" << std::endl;
      RTStructureSetReader::ContourModelSetNodes x;
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
          OFString contourNumber;
          OFString numberOfPoints;
          OFVector<Float64> contourData_LPS;
          mitk::ContourModel::Pointer contourSequence =
                  mitk::ContourModel::New();

          contourItem.getContourNumber(contourNumber);
          contourItem.getNumberOfContourPoints(numberOfPoints);
          contourItem.getContourData(contourData_LPS);

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
        MITK_ERROR << "Error reading contourSeqObject" << std::endl;
      }
      RoiEntry* refROI = this->FindRoiByNumber(refRoiNumber);
      if(refROI==NULL)
      {
        MITK_ERROR << "Can not find references ROI" << std::endl;
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
      contourSet->SetProperty("contour.color", mitk::ColorProperty::New(
                                refROI->DisplayColor[0],
                                refROI->DisplayColor[1],
                                refROI->DisplayColor[2]));

    }
    while(roiContourSeqObject.gotoNextItem().good());

    std::deque<mitk::DataNode::Pointer> nodes;

    for(unsigned int i=0; i<ROISequenceVector.size();i++)
    {
      mitk::DataNode::Pointer node = mitk::DataNode::New();
      node->SetData(ROISequenceVector.at(i).ContourModelSet);
      node->SetProperty("name",
                     ROISequenceVector.at(i).ContourModelSet->GetProperty("name"));
      node->SetProperty("color",
                     ROISequenceVector.at(i).ContourModelSet->GetProperty("contour.color"));
      node->SetProperty("contour.color",
                     ROISequenceVector.at(i).ContourModelSet->GetProperty("contour.color"));

      /*The StructureSets should only be visible in the axial-view and the
        3D-view because in the other views they are just points and lines*/
      node->SetVisibility(true, mitk::BaseRenderer::GetInstance
              ( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1")));
      node->SetVisibility(false, mitk::BaseRenderer::GetInstance
              ( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget2")));
      node->SetVisibility(false, mitk::BaseRenderer::GetInstance
              ( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3")));
      node->SetVisibility(true, mitk::BaseRenderer::GetInstance
              ( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4")));
      nodes.push_back(node);
    }

    return nodes;
  }
}
