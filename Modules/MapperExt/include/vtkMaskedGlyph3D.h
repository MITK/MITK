/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef vtkMaskedGlyph3D_h
#define vtkMaskedGlyph3D_h

#include <MitkMapperExtExports.h>

#include <vtkGlyph3D.h>

#include <mitkCommon.h>

class vtkMaskPoints;
/** \brief VTK filter that masks input points and generates 3D glyphs only for selected points.
 *
 * Extends vtkGlyph3D by adding point masking functionality. Points may be
 * selected by random sampling or by ratio to limit the number of glyphs
 * rendered, which is useful for large datasets where rendering all glyphs
 * would be too slow.
 *
 * \sa vtkMaskedGlyph2D, vtkGlyph3D
 */
class MITKMAPPEREXT_EXPORT vtkMaskedGlyph3D : public vtkGlyph3D
{
public:
  vtkTypeMacro(vtkMaskedGlyph3D, vtkGlyph3D);
  void PrintSelf(ostream &os, vtkIndent indent) override;

  /** \brief Create a new instance of vtkMaskedGlyph3D.
   * \return A new vtkMaskedGlyph3D instance.
   */
  static vtkMaskedGlyph3D *New();

  /** \brief Set the maximum number of points to glyph. */
  vtkSetMacro(MaximumNumberOfPoints, int);

  /** \brief Get the maximum number of points to glyph.
   * \return The current maximum point count.
   */
  vtkGetMacro(MaximumNumberOfPoints, int);

  /** \brief Set the input dataset to this filter.
   * \param[in] input The input dataset containing points to glyph.
   */
  virtual void SetInput(vtkDataSet *input);

  /** \brief Set whether to enable point masking. Non-zero to enable, 0 to disable. */
  vtkSetMacro(UseMaskPoints, int);

  /** \brief Get whether point masking is enabled.
   * \return Non-zero if masking is enabled.
   */
  vtkGetMacro(UseMaskPoints, int);

  /** \brief Set whether to randomly select which points to mask.
   * \param[in] mode Non-zero for random masking, 0 for uniform masking.
   */
  void SetRandomMode(int mode);

  /** \brief Get the current random masking mode.
   * \return Non-zero if random mode is active.
   */
  int GetRandomMode();

  /** \brief Set the input connection for the VTK pipeline.
   * \param[in] input The algorithm output port to connect as input.
   */
  void SetInputConnection(vtkAlgorithmOutput *input) override;

  using vtkGlyph3D::SetInputConnection;

protected:
  vtkMaskedGlyph3D();
  ~vtkMaskedGlyph3D() override;

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

  vtkMaskPoints *MaskPoints;
  int MaximumNumberOfPoints;
  int UseMaskPoints;

private:
  vtkMaskedGlyph3D(const vtkMaskedGlyph3D &); // Not implemented.
  void operator=(const vtkMaskedGlyph3D &);   // Not implemented.
};

#endif
