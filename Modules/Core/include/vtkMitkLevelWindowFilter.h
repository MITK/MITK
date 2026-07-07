/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef vtkMitkLevelWindowFilter_h
#define vtkMitkLevelWindowFilter_h

class vtkScalarsToColors;
class vtkPiecewiseFunction;
#include <vtkImageData.h>
#include <vtkThreadedImageAlgorithm.h>

#include <MitkCoreExports.h>
/**
 * \brief Applies the gray-value or color/opacity level window to scalar or RGB(A) images.
 *
 * This filter is used to apply the color level window to RGB images.
 * The RGB channels are converted to the HSI color space, where the level
 * window can be applied. Afterwards, the HSI values are transformed back to the RGB space.
 *
 * The filter is also able to apply an opacity level window to RGBA images.
 *
 * \ingroup Renderer
 */
class MITKCORE_EXPORT vtkMitkLevelWindowFilter : public vtkThreadedImageAlgorithm
{
public:
  vtkTypeMacro(vtkMitkLevelWindowFilter, vtkThreadedImageAlgorithm);

  /** \brief Create a new instance of vtkMitkLevelWindowFilter. */
  static vtkMitkLevelWindowFilter *New();

  /**
   * \brief Get the modification time, considering the lookup table's modification time.
   *
   * \return The latest modification time of this filter or its lookup table.
   */
  vtkMTimeType GetMTime() override;

  /**
   * \brief Get the lookup table for the RGB level window.
   *
   * \return The current vtkScalarsToColors lookup table, or nullptr.
   */
  vtkScalarsToColors *GetLookupTable();

  /**
   * \brief Set the lookup table for the RGB level window.
   *
   * \param[in] lookupTable  The vtkScalarsToColors lookup table to use.
   */
  void SetLookupTable(vtkScalarsToColors *lookupTable);

  /**
   * \brief Get the piecewise function used to map scalar values to the alpha component.
   *
   * This is only used when the lookup table is a vtkColorTransferFunction.
   *
   * \return The opacity piecewise function, or nullptr.
   */
  vtkPiecewiseFunction *GetOpacityPiecewiseFunction() { return m_OpacityFunction; }

  /**
   * \brief Set the piecewise function used to map scalar values to the alpha component.
   *
   * This is only used when the lookup table is a vtkColorTransferFunction.
   *
   * \param[in] opacityFunction  The piecewise function for opacity mapping.
   */
  void SetOpacityPiecewiseFunction(vtkPiecewiseFunction *opacityFunction);

  /**
   * \brief Set the lower window opacity for the alpha level window.
   *
   * \param[in] minOpacity  The minimum opacity value.
   */
  void SetMinOpacity(double minOpacity);

  /**
   * \brief Get the lower window opacity for the alpha level window.
   *
   * \return The minimum opacity value.
   */
  inline double GetMinOpacity() const;

  /**
   * \brief Set the upper window opacity for the alpha level window.
   *
   * \param[in] maxOpacity  The maximum opacity value.
   */
  void SetMaxOpacity(double maxOpacity);

  /**
   * \brief Get the upper window opacity for the alpha level window.
   *
   * \return The maximum opacity value.
   */
  inline double GetMaxOpacity() const;

  /**
   * \brief Set clipping bounds for the opaque part of the resliced 2D image.
   *
   * The bounds are specified as a four-element array (xmin, xmax, ymin, ymax).
   */
  void SetClippingBounds(double *);

protected:
  /** \brief Default constructor. */
  vtkMitkLevelWindowFilter();

  /** \brief Destructor. */
  ~vtkMitkLevelWindowFilter() override;

  /**
   * \brief Threaded execution method that applies the level window to image data.
   *
   * \param[in]  inData   The input image data.
   * \param[out] outData  The output image data.
   * \param[in]  extent   Six-component array (xmin, xmax, ymin, ymax, zmin, zmax)
   *                      specifying the region to be processed by this thread.
   * \param[in]  id       The thread id.
   */
  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData, int extent[6], int id) override;

  /**
   * \brief Standard VTK pipeline method to propagate output information.
   *
   * \return 1 on success.
   */
  int RequestInformation(vtkInformation *request,
                         vtkInformationVector **inputVector,
                         vtkInformationVector *outputVector) override;
  //  /** Standard VTK filter method to apply the filter. See VTK documentation. Not used at the moment.*/
  //  void ExecuteInformation(vtkImageData *vtkNotUsed(inData), vtkImageData *vtkNotUsed(outData));

private:
  /** m_LookupTable contains the lookup table for the RGB level window.*/
  vtkScalarsToColors *m_LookupTable;
  /** The transfer function to map the scalar to alpha (4th component of the RGBA output value) */
  vtkPiecewiseFunction *m_OpacityFunction;
  /** m_MinOpacity contains the lower bound for the alpha level window.*/
  double m_MinOpacity;
  /** m_MaxOpacity contains the upper bound for the alpha level window.*/
  double m_MaxOpacity;

  double m_ClippingBounds[4];
};
#endif
