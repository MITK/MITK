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

#include "mitkShowSegmentationAsSurface.h"
#include "mitkManualSegmentationToSurfaceFilter.h"
#include "mitkVtkRepresentationProperty.h"
#include <mitkCoreObjectFactory.h>
#include <mitkLabelSetImage.h>
#include <vtkPolyDataNormals.h>

namespace mitk
{
  ShowSegmentationAsSurface::ShowSegmentationAsSurface() : m_UIDGeneratorSurfaces("Surface_") {}
  ShowSegmentationAsSurface::~ShowSegmentationAsSurface() {}
  void ShowSegmentationAsSurface::Initialize(const NonBlockingAlgorithm *other)
  {
    Superclass::Initialize(other);

    bool syncVisibility(false);

    if (other)
    {
      other->GetParameter("Sync visibility", syncVisibility);
    }

    SetParameter("Sync visibility", syncVisibility);
    SetParameter("Median kernel size", 3u);
    SetParameter("Apply median", true);
    SetParameter("Smooth", true);
    SetParameter("Gaussian SD", 1.5f);
    SetParameter("Decimate mesh", true);
    SetParameter("Decimation rate", 0.8f);
    SetParameter("Wireframe", false);

    m_Surfaces.clear();
    m_DeleteMe.clear();
  }

  bool ShowSegmentationAsSurface::ReadyToRun()
  {
    try
    {
      Image::Pointer image;
      GetPointerParameter("Input", image);

      return image.IsNotNull() && GetGroupNode();
    }
    catch (std::invalid_argument &)
    {
      return false;
    }
  }

  bool ShowSegmentationAsSurface::ThreadedUpdateFunction()
  {
    Image::Pointer image;
    GetPointerParameter("Input", image);

    bool smooth(true);
    GetParameter("Smooth", smooth);

    bool applyMedian(true);
    GetParameter("Apply median", applyMedian);

    bool decimateMesh(true);
    GetParameter("Decimate mesh", decimateMesh);

    unsigned int medianKernelSize(3);
    GetParameter("Median kernel size", medianKernelSize);

    float gaussianSD(1.5);
    GetParameter("Gaussian SD", gaussianSD);

    float reductionRate(0.8);
    GetParameter("Decimation rate", reductionRate);

    MITK_INFO << "Creating polygon model with smoothing " << smooth << " gaussianSD " << gaussianSD << " median "
              << applyMedian << " median kernel " << medianKernelSize << " mesh reduction " << decimateMesh
              << " reductionRate " << reductionRate;

    // TODO: Move loop out of the multithreaded method

    auto labelSetImage = dynamic_cast<LabelSetImage *>(image.GetPointer());

    if (nullptr != labelSetImage)
    {
      auto numberOfLayers = labelSetImage->GetNumberOfLayers();

      for (decltype(numberOfLayers) layerIndex = 0; layerIndex < numberOfLayers; ++layerIndex)
      {
        auto labelSet = labelSetImage->GetLabelSet(layerIndex);
        auto numberOfLabels = labelSet->GetNumberOfLabels();

        for (decltype(numberOfLabels) labelIndex = 0; labelIndex < numberOfLabels; ++labelIndex)
        {
          for (auto labelIter = labelSet->IteratorConstBegin(); labelIter != labelSet->IteratorConstEnd(); ++labelIter)
          {
            if (0 == labelIter->first)
              continue;

            MITK_INFO << "Layer " << layerIndex << ", Label " << labelIndex << " (" << labelIter->first << ")";
            auto labelImage = labelSetImage->CreateLabelMask(labelIter->first, false, layerIndex);
            m_DeleteMe.push_back(labelImage);
          }
        }
      }
    }
    else
    {
      ManualSegmentationToSurfaceFilter::Pointer surfaceFilter = ManualSegmentationToSurfaceFilter::New();
      surfaceFilter->SetInput(image);
      surfaceFilter->SetThreshold(0.5); // expects binary image with zeros and ones

      surfaceFilter->SetUseGaussianImageSmooth(smooth); // apply gaussian to thresholded image ?
      surfaceFilter->SetSmooth(smooth);
      if (smooth)
      {
        surfaceFilter->InterpolationOn();
        surfaceFilter->SetGaussianStandardDeviation(gaussianSD);
      }

      surfaceFilter->SetMedianFilter3D(applyMedian); // apply median to segmentation before marching cubes ?
      if (applyMedian)
      {
        surfaceFilter->SetMedianKernelSize(
          medianKernelSize, medianKernelSize, medianKernelSize); // apply median to segmentation before marching cubes
      }

      // fix to avoid vtk warnings see bug #5390
      if (image->GetDimension() > 3)
        decimateMesh = false;

      if (decimateMesh)
      {
        surfaceFilter->SetDecimate(ImageToSurfaceFilter::QuadricDecimation);
        surfaceFilter->SetTargetReduction(reductionRate);
      }
      else
      {
        surfaceFilter->SetDecimate(ImageToSurfaceFilter::NoDecimation);
      }

      surfaceFilter->UpdateLargestPossibleRegion();

      // calculate normals for nicer display
      auto surface = surfaceFilter->GetOutput();
      vtkPolyData *polyData = surface->GetVtkPolyData();

      if (!polyData)
        throw std::logic_error("Could not create polygon model");

      polyData->SetVerts(nullptr);
      polyData->SetLines(nullptr);

      if (smooth || applyMedian || decimateMesh)
      {
        vtkPolyDataNormals *normalsGen = vtkPolyDataNormals::New();

        normalsGen->AutoOrientNormalsOn();
        normalsGen->FlipNormalsOff();
        normalsGen->SetInputData(polyData);
        normalsGen->Update();

        surface->SetVtkPolyData(normalsGen->GetOutput());

        normalsGen->Delete();
      }
      else
      {
        surface->SetVtkPolyData(polyData);
      }

      m_Surfaces.push_back(surface);
    }

    return true;
  }

  void ShowSegmentationAsSurface::ThreadedUpdateSuccessful()
  {
    if (!m_DeleteMe.empty())
    {
      for (const auto &labelImage : m_DeleteMe)
      {
        auto node = DataNode::New();
        node->SetData(labelImage);
        this->InsertBelowGroupNode(node);
      }

      return;
    }

    for (auto surface : m_Surfaces)
    {
      auto node = DataNode::New();

      bool wireframe(false);
      GetParameter("Wireframe", wireframe);
      if (wireframe)
      {
        auto *np =
          dynamic_cast<VtkRepresentationProperty *>(node->GetProperty("material.representation"));
        if (np)
          np->SetRepresentationToWireframe();
      }

      node->SetProperty("opacity", FloatProperty::New(0.3));
      node->SetProperty("line width", IntProperty::New(1));
      node->SetProperty("scalar visibility", BoolProperty::New(false));

      std::string groupNodesName("surface");

      DataNode *groupNode = GetGroupNode();
      if (groupNode)
      {
        groupNode->GetName(groupNodesName);
        // if parameter smooth is set add extension to node name
        bool smooth(true);
        GetParameter("Smooth", smooth);
        if (smooth)
          groupNodesName.append("_smoothed");
      }
      node->SetProperty("name", StringProperty::New(groupNodesName));

      // synchronize this object's color with the parent's color
      // surfaceNode->SetProperty( "color", parentNode->GetProperty( "color" ) );
      // surfaceNode->SetProperty( "visible", parentNode->GetProperty( "visible" ) );

      node->SetData(surface);

      BaseProperty *colorProp = groupNode->GetProperty("color");
      if (colorProp)
        node->ReplaceProperty("color", colorProp->Clone());
      else
        node->SetProperty("color", ColorProperty::New(1.0, 1.0, 0.0));

      bool showResult(true);
      GetParameter("Show result", showResult);

      bool syncVisibility(false);
      GetParameter("Sync visibility", syncVisibility);

      Image::Pointer image;
      GetPointerParameter("Input", image);

      BaseProperty *organTypeProp = image->GetProperty("organ type");
      if (organTypeProp)
        surface->SetProperty("organ type", organTypeProp);

      BaseProperty *visibleProp = groupNode->GetProperty("visible");
      if (visibleProp && syncVisibility)
        node->ReplaceProperty("visible", visibleProp->Clone());
      else
        node->SetProperty("visible", BoolProperty::New(showResult));

      InsertBelowGroupNode(node);
    }

    Superclass::ThreadedUpdateSuccessful();
  }

} // namespace
