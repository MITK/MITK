/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarFigureMaskGenerator_h
#define mitkPlanarFigureMaskGenerator_h

#include <MitkImageStatisticsExports.h>
#include <itkImage.h>
#include <mitkImage.h>
#include <mitkMaskGenerator.h>
#include <mitkPlanarFigure.h>
#include <vtkSmartPointer.h>

namespace mitk
{
  /**
   * \class PlanarFigureMaskGenerator
   * \brief Derived from MaskGenerator. This class is used to convert a mitk::PlanarFigure into a binary image mask
   */
  class MITKIMAGESTATISTICS_EXPORT PlanarFigureMaskGenerator : public MaskGenerator
  {
  public:
    /** Standard Self typedef */
    typedef PlanarFigureMaskGenerator Self;
    typedef MaskGenerator Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    /** Method for creation through the object factory. */
    itkNewMacro(Self); /** Runtime information support. */
      itkTypeMacro(PlanarFigureMaskGenerator, MaskGenerator);

    /**
     * \brief Get the number of masks this generator provides.
     * \return Always returns 1.
     */
    unsigned int GetNumberOfMasks() const override;

    /**
     * \brief Set the planar figure used to define the mask region.
     *
     * The planar figure can be a closed shape (polygon, circle, ellipse, etc.)
     * or an open shape (line, bezier curve). Closed shapes produce a filled
     * 2D binary mask; open shapes produce a mask along the figure's polyline.
     *
     * \param[in] planarFigure Pointer to the PlanarFigure.
     */
    void SetPlanarFigure(mitk::PlanarFigure* planarFigure);

    /**
     * \brief Get the reference image for this mask.
     *
     * Returns a 2D image slice extracted from the input image at the
     * planar figure's plane position.
     *
     * \return Const pointer to the 2D reference image slice.
     */
    mitk::Image::ConstPointer GetReferenceImage() override;

    /** \brief Get the image axis index perpendicular to the planar figure's plane. */
    itkGetConstMacro(PlanarFigureAxis, unsigned int);

    /** \brief Get the slice index at which the planar figure is located. */
    itkGetConstMacro(PlanarFigureSlice, unsigned int);

    /**
     * \brief Check whether a planar figure's plane is axis-aligned with a geometry.
     *
     * Returns true if the planar figure's plane normal is parallel to one of
     * the principal axes of the given geometry (i.e., the figure is not tilted).
     *
     * \param[in] planarGeometry The plane geometry of the planar figure.
     * \param[in] geometry The reference geometry to check against.
     *
     * \return True if the planar figure is not tilted; false if tilted or if
     *         either argument is nullptr.
     *
     * \pre If either planarGeometry or geometry is nullptr, returns false.
     */
    static bool CheckPlanarFigureIsNotTilted(const PlaneGeometry* planarGeometry, const BaseGeometry *geometry);

  protected:
    PlanarFigureMaskGenerator()
      : Superclass(),
        m_ReferenceImage(nullptr),
        m_PlanarFigureAxis(0),
        m_InternalMaskUpdateTime(0),
        m_PlanarFigureSlice(0)
    {
      m_InternalMask = mitk::Image::New();
    }

    Image::ConstPointer DoGetMask(unsigned int) override;

  private:
    void CalculateMask();

    template <typename TPixel, unsigned int VImageDimension>
    void InternalCalculateMaskFromClosedPlanarFigure(const itk::Image<TPixel, VImageDimension> *image, unsigned int axis);

    template <typename TPixel, unsigned int VImageDimension>
    void InternalCalculateMaskFromOpenPlanarFigure(const itk::Image<TPixel, VImageDimension> *image, unsigned int axis);

    mitk::Image::ConstPointer Extract2DImageSlice(const Image* input, unsigned int axis, unsigned int slice) const;

    /** Helper function that deduces if the passed vector is equal to one of the primary axis of the geometry.*/
    static bool GetPrincipalAxis(const BaseGeometry *geometry, Vector3D vector, unsigned int &axis);

    /** Connection from ITK to VTK */
    template <typename ITK_Exporter, typename VTK_Importer>
    void ConnectPipelines(ITK_Exporter exporter, vtkSmartPointer<VTK_Importer> importer)
    {
      importer->SetUpdateInformationCallback(exporter->GetUpdateInformationCallback());

      importer->SetPipelineModifiedCallback(exporter->GetPipelineModifiedCallback());
      importer->SetWholeExtentCallback(exporter->GetWholeExtentCallback());
      importer->SetSpacingCallback(exporter->GetSpacingCallback());
      importer->SetOriginCallback(exporter->GetOriginCallback());
      importer->SetScalarTypeCallback(exporter->GetScalarTypeCallback());

      importer->SetNumberOfComponentsCallback(exporter->GetNumberOfComponentsCallback());

      importer->SetPropagateUpdateExtentCallback(exporter->GetPropagateUpdateExtentCallback());
      importer->SetUpdateDataCallback(exporter->GetUpdateDataCallback());
      importer->SetDataExtentCallback(exporter->GetDataExtentCallback());
      importer->SetBufferPointerCallback(exporter->GetBufferPointerCallback());
      importer->SetCallbackUserData(exporter->GetCallbackUserData());
    }

    /** Connection from VTK to ITK */
    template <typename VTK_Exporter, typename ITK_Importer>
    void ConnectPipelines(vtkSmartPointer<VTK_Exporter> exporter, ITK_Importer importer)
    {
      importer->SetUpdateInformationCallback(exporter->GetUpdateInformationCallback());

      importer->SetPipelineModifiedCallback(exporter->GetPipelineModifiedCallback());
      importer->SetWholeExtentCallback(exporter->GetWholeExtentCallback());
      importer->SetSpacingCallback(exporter->GetSpacingCallback());
      importer->SetOriginCallback(exporter->GetOriginCallback());
      importer->SetScalarTypeCallback(exporter->GetScalarTypeCallback());

      importer->SetNumberOfComponentsCallback(exporter->GetNumberOfComponentsCallback());

      importer->SetPropagateUpdateExtentCallback(exporter->GetPropagateUpdateExtentCallback());
      importer->SetUpdateDataCallback(exporter->GetUpdateDataCallback());
      importer->SetDataExtentCallback(exporter->GetDataExtentCallback());
      importer->SetBufferPointerCallback(exporter->GetBufferPointerCallback());
      importer->SetCallbackUserData(exporter->GetCallbackUserData());
    }

    bool IsUpdateRequired() const;

    mitk::PlanarFigure::Pointer m_PlanarFigure;
    itk::Image<unsigned short, 2>::Pointer m_InternalITKImageMask2D;
    mitk::Image::ConstPointer m_ReferenceImage;
    unsigned int m_PlanarFigureAxis;
    unsigned long m_InternalMaskUpdateTime;
    unsigned int m_PlanarFigureSlice;
    mitk::Image::Pointer m_InternalMask;
  };

} // namespace mitk

#endif
