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
#ifndef MITKREGIONGROW3DTOOL_H
#define MITKREGIONGROW3DTOOL_H

#include "mitkTool.h"
#include "mitkPointSet.h"
#include "SegmentationExports.h"
#include "mitkStateEvent.h"

#include "itkImage.h"

namespace mitk{
  class Segmentation_EXPORT RegionGrow3DTool : public Tool
  {
  public:
    mitkClassMacro(RegionGrow3DTool, Tool);
    itkNewMacro(RegionGrow3DTool);

    mitk::Message1<int> UpperThresholdValueChanged;
    mitk::Message1<int> LowerThresholdValueChanged;
    mitk::Message1<int> SliderValueChanged;
    mitk::Message1<bool> SeedButtonToggled;


    virtual const char* GetName() const;
    virtual const char** GetXPM() const;

    virtual void Activated();
    virtual void Deactivated();

    void RunSegmentation();

    void ConfirmSegmentation(std::string name, mitk::Color color);
    void CancelSegmentation();
    void InitializeLevelWindow();
    void ChangeLevelWindow(int);
    void SetSeedPoint(bool);
    void SetCurrentRGDirectionIsUpwards(bool);

  protected:
    RegionGrow3DTool();
    virtual ~RegionGrow3DTool();

    void SetupPreviewNodeFor(mitk::DataNode* nodeToProceed);
    void UpdatePreview();
    template < typename TPixel, unsigned int VImageDimension >
    void StartRegionGrowing( itk::Image< TPixel, VImageDimension >* itkImage, mitk::Geometry3D* imageGeometry, mitk::PointSet::PointType seedPoint );

    bool OnMouseReleased(Action*, const StateEvent*);


    int m_SeedpointValue;
    mitk::ScalarType m_RoiMax;
    mitk::ScalarType m_RoiMin;

    int m_LowerThreshold;
    int m_UpperThreshold;
    int m_DetectedLeakagePoint;
    bool m_CurrentRGDirectionIsUpwards;
    mitk::DataNode::Pointer m_PointSetNode;
    mitk::DataNode::Pointer m_FeedbackNode;
    mitk::DataNode::Pointer m_NodeToProceed;
    mitk::DataNode::Pointer m_OriginalImageNode;

  };//class
}//namespace

#endif
