/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkBoundingShapeCropper.h"
#include "mitkGeometry3D.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"
#include "mitkImageToItk.h"
#include "mitkStatusBar.h"
#include "mitkTimeHelper.h"

#include <cmath>

#include "vtkMatrix4x4.h"
#include "vtkSmartPointer.h"
#include "vtkTransform.h"

#include "itkImageRegionIteratorWithIndex.h"
#include <itkImageIOBase.h>
#include <itkImageRegionConstIterator.h>
#include <itkRGBAPixel.h>
#include <itkRGBPixel.h>

namespace mitk
{
  BoundingShapeCropper::BoundingShapeCropper()
    : m_Geometry(nullptr),
    m_OutsideValue(0),
    m_UseCropTimeStepOnly(false),
    m_CurrentTimeStep(0),
    m_UseWholeInputRegion(false),
    m_InputTimeSelector(mitk::ImageTimeSelector::New()),
    m_OutputTimeSelector(mitk::ImageTimeSelector::New())
  {
    this->SetNumberOfIndexedInputs(2);
    this->SetNumberOfRequiredInputs(2);
  }

  BoundingShapeCropper::~BoundingShapeCropper() {}

  template <typename TPixel, unsigned int VImageDimension>
  void BoundingShapeCropper::CutImage(itk::Image<TPixel, VImageDimension> *inputItkImage, int timeStep)
  {
    MITK_INFO << "Scalar Pixeltype" << std::endl;

    typedef TPixel TOutputPixel;
    typedef itk::Image<TPixel, VImageDimension> ItkInputImageType;
    typedef itk::Image<TOutputPixel, VImageDimension> ItkOutputImageType;
    typedef typename itk::ImageBase<VImageDimension>::RegionType ItkRegionType;
    typedef itk::ImageRegionIteratorWithIndex<ItkInputImageType> ItkInputImageIteratorType;
    typedef itk::ImageRegionIteratorWithIndex<ItkOutputImageType> ItkOutputImageIteratorType;

    TOutputPixel outsideValue = this->GetOutsideValue();
    // currently 0 if not set in advance
    // TODO: change default value to itk::NumericTraits<TOutputPixel>::min();

    if (this->m_Geometry.IsNull())
      return;

    if (inputItkImage == nullptr)
    {
      mitk::StatusBar::GetInstance()->DisplayErrorText(
        "An internal error occurred. Can't convert Image. Please report to bugs@mitk.org");
      std::cout << " image is nullptr...returning" << std::endl;
      return;
    }

    // first convert the index
    typename ItkRegionType::IndexType::IndexValueType tmpIndex[3];
    itk2vtk(this->m_InputRequestedRegion.GetIndex(), tmpIndex);
    typename ItkRegionType::IndexType index;
    index.SetIndex(tmpIndex);

    // then convert the size
    typename ItkRegionType::SizeType::SizeValueType tmpSize[3];
    itk2vtk(this->m_InputRequestedRegion.GetSize(), tmpSize);
    typename ItkRegionType::SizeType size;
    size.SetSize(tmpSize);

    // create the ITK-image-region out of index and size
    ItkRegionType inputRegionOfInterest(index, size);

    // Get access to the MITK output image via an ITK image
    typename mitk::ImageToItk<ItkOutputImageType>::Pointer outputimagetoitk =
      mitk::ImageToItk<ItkOutputImageType>::New();
    outputimagetoitk->SetInput(this->m_OutputTimeSelector->GetOutput());
    outputimagetoitk->Update();
    typename ItkOutputImageType::Pointer outputItkImage = outputimagetoitk->GetOutput();

    // create the iterators
    ItkInputImageIteratorType inputIt(inputItkImage, inputRegionOfInterest);
    ItkOutputImageIteratorType outputIt(outputItkImage, outputItkImage->GetLargestPossibleRegion());

    // Cut the boundingbox out of the image by iterating through all images
    // TODO: use more efficient method by using the contour instead off all single pixels
    mitk::Point3D p;
    mitk::BaseGeometry *inputGeometry = this->GetInput()->GetGeometry(timeStep);

    // calculates translation based on offset+extent not on the transformation matrix
    // NOTE: center of the box is
    vtkSmartPointer<vtkMatrix4x4> imageTransform = this->m_Geometry->GetGeometry()->GetVtkTransform()->GetMatrix();
    Point3D center = this->m_Geometry->GetGeometry()->GetCenter();
    auto translation = vtkSmartPointer<vtkTransform>::New();
    translation->Translate(center[0] - imageTransform->GetElement(0, 3),
      center[1] - imageTransform->GetElement(1, 3),
      center[2] - imageTransform->GetElement(2, 3));
    auto transform = vtkSmartPointer<vtkTransform>::New();
    transform->SetMatrix(imageTransform);
    transform->PostMultiply();
    transform->Concatenate(translation);
    transform->Update();

    mitk::Vector3D extent;
    for (unsigned int i = 0; i < 3; ++i)
      extent[i] = (this->m_Geometry->GetGeometry()->GetExtent(i));

    for (inputIt.GoToBegin(), outputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt, ++outputIt)
    {
      vtk2itk(inputIt.GetIndex(), p);
      inputGeometry->IndexToWorld(p, p);
      ScalarType p2[4];
      p2[0] = p[0];
      p2[1] = p[1];
      p2[2] = p[2];
      p2[3] = 1;
      // transform point from world to object coordinates
      transform->GetInverse()->TransformPoint(p2, p2);
      // check if the world point is within bounds
      bool isInside = (p2[0] >= (-extent[0] / 2.0)) && (p2[0] <= (extent[0] / 2.0)) && (p2[1] >= (-extent[1] / 2.0)) &&
        (p2[1] <= (extent[1] / 2.0)) && (p2[2] >= (-extent[2] / 2.0)) && (p2[2] <= (extent[2] / 2.0));

      if ((!this->m_UseCropTimeStepOnly && isInside) ||
        (this->m_UseCropTimeStepOnly && timeStep == this->m_CurrentTimeStep && isInside))
      {
        outputIt.Set((TOutputPixel)inputIt.Value());
      }
      else
      {
        outputIt.Set(outsideValue);
      }
    }
  }

  void BoundingShapeCropper::SetGeometry(const mitk::GeometryData *geometry)
  {
    m_Geometry = const_cast<mitk::GeometryData *>(geometry);
    // Process object is not const-correct so the const_cast is required here
    this->ProcessObject::SetNthInput(1, const_cast<mitk::GeometryData *>(geometry));
  }

  // const mitk::GeometryData* BoundingShapeCropper::GetGeometryData() const
  //{
  // return m_Geometry.GetPointer();
  //}

  const mitk::PixelType BoundingShapeCropper::GetOutputPixelType() { return this->GetInput()->GetPixelType(); }
  void BoundingShapeCropper::GenerateInputRequestedRegion()
  {
    mitk::Image *output = this->GetOutput();
    if ((output->IsInitialized() == false) || (m_Geometry.IsNull()) ||
      (m_Geometry->GetTimeGeometry()->CountTimeSteps() == 0))
      return;

    GenerateTimeInInputRegion(output, this->GetInput());
  }

  void BoundingShapeCropper::GenerateOutputInformation()
  {
    // Set Cropping region
    mitk::Image::Pointer output = this->GetOutput();
    if ((output->IsInitialized()) && (output->GetPipelineMTime() <= m_TimeOfHeaderInitialization.GetMTime()))
      return;

    mitk::Image::Pointer input = this->GetInput();

    if (input.IsNull())
    {
      mitkThrow() << "Input is not a mitk::Image";
    }
    itkDebugMacro(<< "GenerateOutputInformation()");

    unsigned int dimension = input->GetDimension();
    if (dimension < 3)
    {
      mitkThrow() << "ImageCropper cannot handle 1D or 2D Objects.";
    }

    if ((m_Geometry.IsNull()) || (m_Geometry->GetTimeGeometry()->CountTimeSteps() == 0))
      return;

    mitk::BaseGeometry *bsGeometry = m_Geometry->GetGeometry();
    mitk::BaseGeometry *inputImageGeometry = input->GetSlicedGeometry();

    // calculate bounding box
    mitk::BoundingBox::Pointer bsBoxRelativeToImage =
      bsGeometry->CalculateBoundingBoxRelativeToTransform(inputImageGeometry->GetIndexToWorldTransform());

    // pre-initialize input-requested-region to largest-possible-region
    m_InputRequestedRegion = input->GetLargestPossibleRegion();
    // build region out of bounding-box of index and size of the bounding box
    mitk::SlicedData::IndexType index = m_InputRequestedRegion.GetIndex(); // init times and channels
    mitk::BoundingBox::PointType min = bsBoxRelativeToImage->GetMinimum();
    mitk::SlicedData::SizeType size = m_InputRequestedRegion.GetSize(); // init times and channels
    mitk::BoundingBox::PointType max = bsBoxRelativeToImage->GetMaximum();
    mitk::Point<BoundingBox::PointType::CoordRepType, 5> maxCorrected;
    mitk::Point<BoundingBox::PointType::CoordRepType, 5> minCorrected;

    for (unsigned int i = 0; i < 3; i++)
    {
      maxCorrected[i] = max[i];
      minCorrected[i] = min[i];
    }

    maxCorrected[3] = input->GetDimensions()[3];
    maxCorrected[4] = 0;
    minCorrected[3] = 0;
    minCorrected[4] = 0;

    for (unsigned int i = 0; i < dimension; i++)
    {
      index[i] = (mitk::SlicedData::IndexType::IndexValueType)(std::ceil(minCorrected[i]));
      size[i] = (mitk::SlicedData::SizeType::SizeValueType)(std::ceil(maxCorrected[i]) - index[i]);
    }

    mitk::SlicedData::RegionType bsRegion(index, size);

    if (m_UseWholeInputRegion == false)
    {
      // crop input-requested-region with region of bounding-object
      if (m_InputRequestedRegion.Crop(bsRegion) == false)
      {
        // crop not possible => do nothing: set time size to 0.
        size.Fill(0);
        m_InputRequestedRegion.SetSize(size);
        bsRegion.SetSize(size);
        mitkThrow() << "No overlap of the image and the cropping object.";
      }
    }

    // initialize output image
    auto dimensions = new unsigned int[dimension];
    if (dimension > 3 && !this->GetUseCropTimeStepOnly())
      memcpy(dimensions + 3, input->GetDimensions() + 3, (dimension - 3) * sizeof(unsigned int));
    else
      dimension = 3; // set timeStep to zero if GetUseCropTimeStepOnly is true

    itk2vtk(m_InputRequestedRegion.GetSize(), dimensions);

    output->Initialize(mitk::PixelType(GetOutputPixelType()), dimension, dimensions);
    delete[] dimensions;

    // Apply transform of the input image to the new generated output image
    mitk::BoundingShapeCropper::RegionType outputRegion = output->GetRequestedRegion();

    m_TimeOfHeaderInitialization.Modified();
  }

  void BoundingShapeCropper::ComputeData(mitk::Image *image, int boTimeStep)
  {
    // examine dimension and pixeltype
    if ((image == nullptr) || (image->GetDimension() > 4) || (image->GetDimension() <= 2))
    {
      MITK_ERROR << "Filter cannot handle dimensions less than 2 and greater than 4" << std::endl;
      itkExceptionMacro("Filter cannot handle dimensions less than 2 and greater than 4");
      return;
    }

    AccessByItk_1(image, CutImage, boTimeStep);
  }

  void BoundingShapeCropper::GenerateData()
  {
    MITK_INFO << "Generate Data" << std::endl;
    mitk::Image::ConstPointer input = this->GetInput();
    mitk::Image::Pointer output = this->GetOutput();

    if (input.IsNull())
      return;

    if ((output->IsInitialized() == false) || (m_Geometry.IsNull()) ||
      (m_Geometry->GetTimeGeometry()->CountTimeSteps() == 0))
      return;

    m_InputTimeSelector->SetInput(input);
    m_OutputTimeSelector->SetInput(this->GetOutput());

    mitk::BoundingShapeCropper::RegionType outputRegion = output->GetRequestedRegion();
    mitk::BaseGeometry *inputImageGeometry = input->GetSlicedGeometry();

    // iterate over all time steps and perform cropping or masking on all or a specific timestep (perviously specified
    // by UseCurrentTimeStepOnly)
    int tstart = outputRegion.GetIndex(3);
    int tmax = tstart + outputRegion.GetSize(3);

    if (this->m_UseCropTimeStepOnly)
    {
      mitk::SlicedGeometry3D *slicedGeometry = output->GetSlicedGeometry(tstart);
      auto indexToWorldTransform = AffineTransform3D::New();
      indexToWorldTransform->SetParameters(input->GetSlicedGeometry(tstart)->GetIndexToWorldTransform()->GetParameters());
      slicedGeometry->SetIndexToWorldTransform(indexToWorldTransform);
      const mitk::SlicedData::IndexType &start = m_InputRequestedRegion.GetIndex();
      mitk::Point3D origin;
      vtk2itk(start, origin);
      inputImageGeometry->IndexToWorld(origin, origin);
      slicedGeometry->SetOrigin(origin);
      m_InputTimeSelector->SetTimeNr(m_CurrentTimeStep);
      m_InputTimeSelector->UpdateLargestPossibleRegion();
      m_OutputTimeSelector->SetTimeNr(tstart);
      m_OutputTimeSelector->UpdateLargestPossibleRegion();
      ComputeData(m_InputTimeSelector->GetOutput(), m_CurrentTimeStep);
    }
    else
    {
      int t;
      for (t = tstart; t < tmax; ++t)
      {
        mitk::SlicedGeometry3D *slicedGeometry = output->GetSlicedGeometry(t);
        auto indexToWorldTransform = AffineTransform3D::New();
        indexToWorldTransform->SetParameters(input->GetSlicedGeometry(t)->GetIndexToWorldTransform()->GetParameters());
        slicedGeometry->SetIndexToWorldTransform(indexToWorldTransform);
        const mitk::SlicedData::IndexType &start = m_InputRequestedRegion.GetIndex();
        mitk::Point3D origin;
        vtk2itk(start, origin);
        inputImageGeometry->IndexToWorld(origin, origin);
        slicedGeometry->SetOrigin(origin);
        m_InputTimeSelector->SetTimeNr(t);
        m_InputTimeSelector->UpdateLargestPossibleRegion();
        m_OutputTimeSelector->SetTimeNr(t);
        m_OutputTimeSelector->UpdateLargestPossibleRegion();
        ComputeData(m_InputTimeSelector->GetOutput(), t);
      }
    }
    m_InputTimeSelector->SetInput(nullptr);
    m_OutputTimeSelector->SetInput(nullptr);
    m_TimeOfHeaderInitialization.Modified();
  }
} // of namespace mitk
