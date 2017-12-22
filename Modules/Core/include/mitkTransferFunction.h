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

#ifndef MITK_TRANSFER_FUNCTION_H_HEADER_INCLUDED
#define MITK_TRANSFER_FUNCTION_H_HEADER_INCLUDED

#include "mitkHistogramGenerator.h"
#include "mitkImage.h"
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
   * @brief The TransferFunction class A wrapper class for VTK scalar opacity,
   * gradient opacity, and color transfer functions.
   * @ingroup DataManagement
   *
   * Holds a copy of each of the three standard VTK transfer functions (scalar
   * opacity, gradient opacity, color) and provides an interface for manipulating
   * their control points. Each original function can be retrieved by a Get()
   * method.
   *
   * @note Currently, transfer function initialization based on histograms or
   * computed-tomography-presets is also provided by this class, but will likely
   * be separated into a specific initializer class.
   *
   * @note If you want to use this as a property for an mitk::Image, make sure
   * to use the mitk::TransferFunctionProperty and set the mitk::RenderingModeProperty
   * to a mode which supports transfer functions (e.g. COLORTRANSFERFUNCTION_COLOR).
   * Make sure to check the documentation of the mitk::RenderingModeProperty. For a
   * code example how to use the mitk::TransferFunction check the
   * mitkImageVtkMapper2DTransferFunctionTest.cpp in Core\Code\Testing.
   */
  class MITKCORE_EXPORT TransferFunction : public itk::Object
  {
  public:
    typedef std::vector<std::pair<double, double>> ControlPoints;
    typedef std::vector<std::pair<double, itk::RGBPixel<double>>> RGBControlPoints;

    mitkClassMacroItkParent(TransferFunction, itk::DataObject);

    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

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

    /** \brief Initialize transfer function based on the histogram of an mitk::Image. */
    void InitializeByMitkImage(const mitk::Image *image);

    /** \brief Initialize transfer function based on the specified histogram. */
    void InitializeByItkHistogram(const itk::Statistics::Histogram<double> *histogram);

    /** \brief Initialize the internal histogram and min/max range based on the
     * specified mitk::Image. */
    void InitializeHistogram(const mitk::Image *image);

    /** \brief Insert control points and values into the scalar opacity transfer
     * function. */
    void SetScalarOpacityPoints(TransferFunction::ControlPoints points);

    /** \brief Insert control points and values into the gradient opacity transfer
     * function. */
    void SetGradientOpacityPoints(TransferFunction::ControlPoints points);

    /** \brief Insert control points and RGB values into the color transfer
     * function. */
    void SetRGBPoints(TransferFunction::RGBControlPoints rgbpoints);

    /** \brief Add a single control point to the scalar opacity transfer function. */
    void AddScalarOpacityPoint(double x, double value);

    /** \brief Add a single control point to the gradient opacity transfer function. */
    void AddGradientOpacityPoint(double x, double value);

    /** \brief Add a single control point to the color opacity transfer function. */
    void AddRGBPoint(double x, double r, double g, double b);

    /** \brief Get a copy of the scalar opacity transfer function control-points. */
    TransferFunction::ControlPoints &GetScalarOpacityPoints();

    /** \brief Get a copy of the gradient opacity transfer function control-points. */
    TransferFunction::ControlPoints &GetGradientOpacityPoints();

    /** \brief Get a copy of the color transfer function control-points. */
    TransferFunction::RGBControlPoints &GetRGBPoints();

    /** \brief Remove the specified control point from the scalar opacity transfer
     * function. */
    int RemoveScalarOpacityPoint(double x);

    /** \brief Remove the specified control point from the gradient opacity transfer
     * function. */
    int RemoveGradientOpacityPoint(double x);

    /** \brief Remove the specified control point from the color transfer function. */
    int RemoveRGBPoint(double x);

    /** \brief Removes all control points from the scalar opacity transfer function. */
    void ClearScalarOpacityPoints();

    /** \brief Removes all control points from the gradient opacity transfer
     * function. */
    void ClearGradientOpacityPoints();

    /** \brief Removes all control points from the color transfer function. */
    void ClearRGBPoints();

    bool operator==(Self &other);

  protected:
    TransferFunction();
    ~TransferFunction() override;

    TransferFunction(const TransferFunction &other);

    itk::LightObject::Pointer InternalClone() const override;

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

#endif /* MITK_TRANSFER_FUNCTION_H_HEADER_INCLUDED */
