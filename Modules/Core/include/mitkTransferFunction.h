/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTransferFunction_h
#define mitkTransferFunction_h

#include <mitkHistogramGenerator.h>
#include <mitkImage.h>
#include <MitkCoreExports.h>

#include <itkHistogram.h>
#include <itkObject.h>
#include <itkRGBPixel.h>

#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkSmartPointer.h>

#include <algorithm>
#include <set>
#include <vector>

namespace mitk
{
  /**
   * \brief Wrapper for VTK scalar opacity, gradient opacity, and color transfer functions.
   *
   * Holds and manages three standard VTK transfer functions: scalar opacity
   * (vtkPiecewiseFunction), gradient opacity (vtkPiecewiseFunction), and color
   * (vtkColorTransferFunction). Provides an interface for adding, removing,
   * and querying control points, as well as initialization from image histograms.
   *
   * \note If you want to use this as a property for an mitk::Image, wrap it in
   *       a mitk::TransferFunctionProperty and set mitk::RenderingModeProperty
   *       to a mode that supports transfer functions (e.g. COLORTRANSFERFUNCTION_COLOR).
   *       See the test mitkImageVtkMapper2DTransferFunctionTest.cpp for an example.
   *
   * \sa TransferFunctionProperty, RenderingModeProperty, LookupTable
   */
  class MITKCORE_EXPORT TransferFunction : public itk::Object
  {
  public:
    /** \brief Type for scalar opacity or gradient opacity control points (x, value). */
    typedef std::vector<std::pair<double, double>> ControlPoints;

    /** \brief Type for color transfer function control points (x, RGB). */
    typedef std::vector<std::pair<double, itk::RGBPixel<double>>> RGBControlPoints;

    mitkClassMacroItkParent(TransferFunction, itk::DataObject);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /** \brief Get/Set min/max of transfer function range for initialization. */
      itkSetMacro(Min, int);

    /** \brief Get/Set min/max of transfer function range for initialization. */
    itkSetMacro(Max, int);

    /** \brief Get/Set min/max of transfer function range for initialization. */
    itkGetMacro(Min, int);

    /** \brief Get/Set min/max of transfer function range for initialization. */
    itkGetMacro(Max, int);

    /** \brief Get/Set wrapped vtk transfer function. */
    itkGetMacro(ScalarOpacityFunction, vtkPiecewiseFunction *);

    /** \brief Get/Set wrapped vtk transfer function. */
    itkGetMacro(GradientOpacityFunction, vtkPiecewiseFunction *);

    /** \brief Get/Set wrapped vtk transfer function. */
    itkGetMacro(ColorTransferFunction, vtkColorTransferFunction *);
    itkSetMacro(ColorTransferFunction, vtkSmartPointer<vtkColorTransferFunction>);

    /** \brief Get histogram used for transfer function initialization. */
    itkGetConstObjectMacro(Histogram, HistogramGenerator::HistogramType);

    /**
     * \brief Initialize all three transfer functions from an mitk::Image histogram.
     * \param[in] image The image whose histogram drives the initialization.
     */
    void InitializeByMitkImage(const mitk::Image *image);

    /**
     * \brief Initialize all three transfer functions from an ITK histogram.
     * \param[in] histogram The histogram to use for initialization.
     */
    void InitializeByItkHistogram(const itk::Statistics::Histogram<double> *histogram);

    /**
     * \brief Compute and store the histogram and min/max range from an image.
     * \param[in] image The image from which the histogram is computed.
     */
    void InitializeHistogram(const mitk::Image *image);

    /**
     * \brief Replace all scalar opacity control points.
     * \param[in] points The new set of (x, opacity) control points.
     */
    void SetScalarOpacityPoints(TransferFunction::ControlPoints points);

    /**
     * \brief Replace all gradient opacity control points.
     * \param[in] points The new set of (x, opacity) control points.
     */
    void SetGradientOpacityPoints(TransferFunction::ControlPoints points);

    /**
     * \brief Replace all color transfer function control points.
     * \param[in] rgbpoints The new set of (x, RGB) control points.
     */
    void SetRGBPoints(TransferFunction::RGBControlPoints rgbpoints);

    /**
     * \brief Add a single control point to the scalar opacity function.
     * \param[in] x     The scalar value position.
     * \param[in] value The opacity at this position (0.0 to 1.0).
     */
    void AddScalarOpacityPoint(double x, double value);

    /**
     * \brief Add a single control point to the gradient opacity function.
     * \param[in] x     The gradient magnitude position.
     * \param[in] value The opacity at this position (0.0 to 1.0).
     */
    void AddGradientOpacityPoint(double x, double value);

    /**
     * \brief Add a single control point to the color transfer function.
     * \param[in] x The scalar value position.
     * \param[in] r Red component (0.0 to 1.0).
     * \param[in] g Green component (0.0 to 1.0).
     * \param[in] b Blue component (0.0 to 1.0).
     */
    void AddRGBPoint(double x, double r, double g, double b);

    /**
     * \brief Get the scalar opacity control points.
     * \return Reference to the internal control point vector.
     */
    TransferFunction::ControlPoints &GetScalarOpacityPoints();

    /**
     * \brief Get the gradient opacity control points.
     * \return Reference to the internal control point vector.
     */
    TransferFunction::ControlPoints &GetGradientOpacityPoints();

    /**
     * \brief Get the color transfer function control points.
     * \return Reference to the internal RGB control point vector.
     */
    TransferFunction::RGBControlPoints &GetRGBPoints();

    /**
     * \brief Remove a control point from the scalar opacity function.
     * \param[in] x The scalar position of the control point to remove.
     * \return The index of the removed point, or -1 if not found.
     */
    int RemoveScalarOpacityPoint(double x);

    /**
     * \brief Remove a control point from the gradient opacity function.
     * \param[in] x The gradient position of the control point to remove.
     * \return The index of the removed point, or -1 if not found.
     */
    int RemoveGradientOpacityPoint(double x);

    /**
     * \brief Remove a control point from the color transfer function.
     * \param[in] x The scalar position of the control point to remove.
     * \return The index of the removed point, or -1 if not found.
     */
    int RemoveRGBPoint(double x);

    /** \brief Remove all control points from the scalar opacity function. */
    void ClearScalarOpacityPoints();

    /** \brief Remove all control points from the gradient opacity function. */
    void ClearGradientOpacityPoints();

    /** \brief Remove all control points from the color transfer function. */
    void ClearRGBPoints();

    /**
     * \brief Compare two TransferFunction objects for equality.
     * \param[in] other The TransferFunction to compare against.
     * \return True if all three transfer functions have identical control points.
     */
    bool operator==(Self &other);

  protected:
    TransferFunction();
    ~TransferFunction() override;

    TransferFunction(const Self &other);

    mitkCloneMacro(Self);

    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

    /** Wrapped VTK scalar opacity transfer function */
    vtkSmartPointer<vtkPiecewiseFunction> m_ScalarOpacityFunction;

    /** Wrapped VTK gradient opacity transfer function */
    vtkSmartPointer<vtkPiecewiseFunction> m_GradientOpacityFunction;

    /** Wrapped VTK color transfer function */
    vtkSmartPointer<vtkColorTransferFunction> m_ColorTransferFunction;

    /** Current range of transfer function (used for initialization) */
    int m_Min;

    /** Current range of transfer function (used for initialization) */
    int m_Max;

    /** Specified or calculated histogram (used for initialization) */
    mitk::HistogramGenerator::HistogramType::ConstPointer m_Histogram;

  private:
    /** Temporary STL style copy of VTK internal control points */
    TransferFunction::ControlPoints m_ScalarOpacityPoints;

    /** Temporary STL style copy of VTK internal control points */
    TransferFunction::ControlPoints m_GradientOpacityPoints;

    /** Temporary STL style copy of VTK internal control points */
    TransferFunction::RGBControlPoints m_RGBPoints;
  };
}

#endif
