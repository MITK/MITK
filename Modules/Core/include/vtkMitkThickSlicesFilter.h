/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef vtkMitkThickSlicesFilter_h
#define vtkMitkThickSlicesFilter_h

#include <MitkCoreExports.h>

#include <vtkThreadedImageAlgorithm.h>

/**
 * \brief Computes a thick slice projection from a 3D image stack.
 *
 * vtkMitkThickSlicesFilter collapses a set of 2D slices into a single
 * output slice using one of several projection modes (MIP, MinIP, SUM,
 * WEIGHTED, MEAN). The Dimensionality property determines whether to
 * treat the input as a set of 2D or 3D slices.
 *
 * \ingroup Renderer
 */
class MITKCORE_EXPORT vtkMitkThickSlicesFilter : public vtkThreadedImageAlgorithm
{
public:
  /** \brief Create a new instance of vtkMitkThickSlicesFilter. */
  static vtkMitkThickSlicesFilter *New();
  vtkTypeMacro(vtkMitkThickSlicesFilter, vtkThreadedImageAlgorithm);

  /** \brief Print the filter state to the given output stream. */
  void PrintSelf(ostream &os, vtkIndent indent) override;

  /** \brief Determines how the input is interpreted (set of 2D slices). Clamped to [2, 3]. */
  vtkSetClampMacro(Dimensionality, int, 2, 3);
  vtkGetMacro(Dimensionality, int);

  /**
   * \brief Get/Set whether to handle boundaries.
   *
   * If enabled, boundary pixels are treated as duplicated so that central
   * differencing works for the boundary pixels. If disabled, the output
   * whole extent of the image is reduced by one pixel.
   */
  vtkSetMacro(HandleBoundaries, int);
  vtkGetMacro(HandleBoundaries, int);
  vtkBooleanMacro(HandleBoundaries, int);

  /**
   * \brief Enumeration of thick slice projection modes.
   *
   * \li MIP -- Maximum Intensity Projection
   * \li SUM -- Sum of all slices (normalized)
   * \li WEIGHTED -- Gaussian-weighted projection
   * \li MINIP -- Minimum Intensity Projection
   * \li MEAN -- Mean of all slices
   */
  enum
  {
    MIP = 0,
    SUM,
    WEIGHTED,
    MINIP,
    MEAN
  };

protected:
  /** \brief Constructor. Sets defaults: HandleBoundaries=1, Dimensionality=2, mode=MIP. */
  vtkMitkThickSlicesFilter();

  /** \brief Destructor. */
  ~vtkMitkThickSlicesFilter() override{};

  int HandleBoundaries;
  int Dimensionality;

  /** \brief Propagate output information (reduces Z extent to a single slice). */
  int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

  /** \brief Compute the input update extent from the output request. */
  int RequestUpdateExtent(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

  /** \brief Execute the filter and process output data arrays. */
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

  /**
   * \brief Threaded execution method that applies the selected projection mode.
   *
   * \param[in]  inData    Array of input image data.
   * \param[out] outData   Array of output image data.
   * \param[in]  outExt    Six-component output extent array.
   * \param[in]  threadId  The thread id.
   */
  void ThreadedRequestData(vtkInformation *,
                           vtkInformationVector **,
                           vtkInformationVector *,
                           vtkImageData ***inData,
                           vtkImageData **outData,
                           int outExt[6],
                           int threadId) override;

  int m_CurrentMode;

private:
  vtkMitkThickSlicesFilter(const vtkMitkThickSlicesFilter &); // Not implemented.
  void operator=(const vtkMitkThickSlicesFilter &);           // Not implemented.

public:
  /**
   * \brief Set the thick slice projection mode.
   *
   * \param[in] mode  One of the projection mode enum values (MIP, SUM, WEIGHTED, MINIP, MEAN).
   */
  void SetThickSliceMode(int mode) { m_CurrentMode = mode; }

  /**
   * \brief Get the current thick slice projection mode.
   *
   * \return The current projection mode enum value.
   */
  int GetThickSliceMode() { return m_CurrentMode; }
};

#endif
