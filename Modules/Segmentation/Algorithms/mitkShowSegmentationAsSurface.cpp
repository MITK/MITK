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
  ShowSegmentationAsSurface::ShowSegmentationAsSurface()
    : m_UIDGeneratorSurfaces("Surface_"),
      m_IsLabelSetImage(false)
  {
  }

  ShowSegmentationAsSurface::~ShowSegmentationAsSurface()
  {
  }

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
    SetParameter("Gaussian SD", 1.5);
    SetParameter("Decimate mesh", true);
    SetParameter("Decimation rate", 0.8);
    SetParameter("Wireframe", false);

    m_SurfaceNodes.clear();
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

    double gaussianSD(1.5);
    GetParameter("Gaussian SD", gaussianSD);

    double reductionRate(0.8);
    GetParameter("Decimation rate", reductionRate);

    MITK_INFO << "Creating polygon model with smoothing " << smooth << " gaussianSD " << gaussianSD << " median "
              << applyMedian << " median kernel " << medianKernelSize << " mesh reduction " << decimateMesh
              << " reductionRate " << reductionRate;

    auto labelSetImage = dynamic_cast<LabelSetImage *>(image.GetPointer());

    if (nullptr != labelSetImage)
    {
      auto numberOfLayers = labelSetImage->GetNumberOfLayers();

      for (decltype(numberOfLayers) layerIndex = 0; layerIndex < numberOfLayers; ++layerIndex)
      {
        auto labelSet = labelSetImage->GetLabelSet(layerIndex);

        for (auto labelIter = labelSet->IteratorConstBegin(); labelIter != labelSet->IteratorConstEnd(); ++labelIter)
        {
          if (0 == labelIter->first)
            continue; // Do not process background label

          auto labelImage = labelSetImage->CreateLabelMask(labelIter->first, false, layerIndex);

          if (labelImage.IsNull())
            continue;

          auto labelSurface = this->ConvertBinaryImageToSurface(labelImage);

          if (labelSurface.IsNull())
            continue;

          auto node = DataNode::New();
          node->SetData(labelSurface);
          node->SetColor(labelIter->second->GetColor());
          node->SetName(labelIter->second->GetName());

          m_SurfaceNodes.push_back(node);
        }
      }
    }
    else
    {
      auto surface = this->ConvertBinaryImageToSurface(image);

      if (surface.IsNotNull())
      {
        auto node = DataNode::New();
        node->SetData(surface);
        m_SurfaceNodes.push_back(node);
      }
    }

    m_IsLabelSetImage = nullptr != labelSetImage;
    return true;
  }

  void ShowSegmentationAsSurface::ThreadedUpdateSuccessful()
  {
    for (const auto &node : m_SurfaceNodes)
    {
      bool wireframe = false;
      GetParameter("Wireframe", wireframe);

      if (wireframe)
      {
        auto representation = dynamic_cast<VtkRepresentationProperty *>(node->GetProperty("material.representation"));
        if (nullptr != representation)
          representation->SetRepresentationToWireframe();
      }

      node->SetProperty("opacity", FloatProperty::New(0.3f));
      node->SetProperty("line width", FloatProperty::New(1.0f));
      node->SetProperty("scalar visibility", BoolProperty::New(false));

      auto name = node->GetName();
      auto groupNode = this->GetGroupNode();

      if (!m_IsLabelSetImage)
      {
        if ((name.empty() || DataNode::NO_NAME_VALUE() == name) && nullptr != groupNode)
          name = groupNode->GetName();

        if (name.empty())
          name = "Surface";
      }

      bool smooth = true;
      GetParameter("Smooth", smooth);

      if (smooth)
        name.append(" (smoothed)");

      node->SetName(name);

      if (!m_IsLabelSetImage)
      {
        auto colorProp = groupNode->GetProperty("color");

        if (nullptr != colorProp)
        {
          node->ReplaceProperty("color", colorProp->Clone());
        }
        else
        {
          node->SetProperty("color", ColorProperty::New(1.0, 1.0, 0.0));
        }
      }

      bool showResult = true;
      GetParameter("Show result", showResult);

      bool syncVisibility = false;
      GetParameter("Sync visibility", syncVisibility);

      auto visibleProp = groupNode->GetProperty("visible");

      if (nullptr != visibleProp && syncVisibility)
      {
        node->ReplaceProperty("visible", visibleProp->Clone());
      }
      else
      {
        node->SetProperty("visible", BoolProperty::New(showResult));
      }

      if (!m_IsLabelSetImage)
      {
        Image::Pointer image;
        GetPointerParameter("Input", image);

        if (image.IsNotNull())
        {
          auto organTypeProp = image->GetProperty("organ type");

          if (nullptr != organTypeProp)
            node->GetData()->SetProperty("organ type", organTypeProp);
        }
      }

      this->InsertBelowGroupNode(node);
    }

    Superclass::ThreadedUpdateSuccessful();
  }

  Surface::Pointer ShowSegmentationAsSurface::ConvertBinaryImageToSurface(Image::Pointer binaryImage)
  {
    bool smooth = true;
    GetParameter("Smooth", smooth);

    bool applyMedian = true;
    GetParameter("Apply median", applyMedian);

    bool decimateMesh = true;
    GetParameter("Decimate mesh", decimateMesh);

    unsigned int medianKernelSize = 3;
    GetParameter("Median kernel size", medianKernelSize);

    double gaussianSD = 1.5;
    GetParameter("Gaussian SD", gaussianSD);

    double reductionRate = 0.8;
    GetParameter("Decimation rate", reductionRate);

    auto filter = ManualSegmentationToSurfaceFilter::New();
    filter->SetInput(binaryImage);
    filter->SetThreshold(0.5);
    filter->SetUseGaussianImageSmooth(smooth);
    filter->SetSmooth(smooth);
    filter->SetMedianFilter3D(applyMedian);

    if (smooth)
    {
      filter->InterpolationOn();
      filter->SetGaussianStandardDeviation(gaussianSD);
    }

    if (applyMedian)
      filter->SetMedianKernelSize(medianKernelSize, medianKernelSize, medianKernelSize);

    // Fix to avoid VTK warnings (see T5390)
    if (binaryImage->GetDimension() > 3)
      decimateMesh = false;

    if (decimateMesh)
    {
      filter->SetDecimate(ImageToSurfaceFilter::QuadricDecimation);
      filter->SetTargetReduction(reductionRate);
    }
    else
    {
      filter->SetDecimate(ImageToSurfaceFilter::NoDecimation);
    }

    filter->UpdateLargestPossibleRegion();

    auto surface = filter->GetOutput();
    auto polyData = surface->GetVtkPolyData();

    if (nullptr == polyData)
      throw std::logic_error("Could not create polygon model");

    polyData->SetVerts(nullptr);
    polyData->SetLines(nullptr);

    if (smooth || applyMedian || decimateMesh)
    {
      auto normals = vtkSmartPointer<vtkPolyDataNormals>::New();

      normals->AutoOrientNormalsOn();
      normals->FlipNormalsOff();
      normals->SetInputData(polyData);

      normals->Update();

      surface->SetVtkPolyData(normals->GetOutput());
    }
    else
    {
      surface->SetVtkPolyData(polyData);
    }

    return surface;
  }
}
