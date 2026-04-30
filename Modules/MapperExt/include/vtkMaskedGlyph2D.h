/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef vtkMaskedGlyph2D_h
#define vtkMaskedGlyph2D_h

#include <MitkMapperExtExports.h>

#include <mitkCommon.h>
#include <vtkGlyph2D.h>

class vtkMaskPoints;
/** \brief VTK filter that masks input points and generates 2D glyphs only for selected points.
 *
 * Extends vtkGlyph2D by adding point masking functionality. Points may be
 * selected by random sampling or by ratio to limit the number of glyphs
 * rendered, which is useful for large datasets where rendering all glyphs
 * would be too slow.
 *
 * \sa vtkMaskedGlyph3D, vtkGlyph2D
 */
class MITKMAPPEREXT_EXPORT vtkMaskedGlyph2D : public vtkGlyph2D
{
public:
  vtkTypeMacro(vtkMaskedGlyph2D, vtkGlyph2D);
  void PrintSelf(ostream &os, vtkIndent indent) override;

  /** \brief Create a new instance of vtkMaskedGlyph2D.
   * \return A new vtkMaskedGlyph2D instance.
   */
  static vtkMaskedGlyph2D *New();

  /** \brief Set the maximum number of points to glyph.
   *
   * Points exceeding this limit are masked out. Default: all points.
   */
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

  ///**
  // * If you want to use an arbitrary scalars array, then set its name here.
  // * By default this in nullptr and the filter will use the active scalar array.
  // */
  // vtkGetStringMacro(InputScalarsSelection);
  // void SelectInputScalars(const char *fieldName)
  //  {this->SetInputScalarsSelection(fieldName);}

  ///**
  // * If you want to use an arbitrary vectors array, then set its name here.
  // * By default this in nullptr and the filter will use the active vector array.
  // */
  // vtkGetStringMacro(InputVectorsSelection);
  // void SelectInputVectors(const char *fieldName)
  //  {this->SetInputVectorsSelection(fieldName);}

  ///**
  // * If you want to use an arbitrary normals array, then set its name here.
  // * By default this in nullptr and the filter will use the active normal array.
  // */
  // vtkGetStringMacro(InputNormalsSelection);
  // void SelectInputNormals(const char *fieldName)
  //  {this->SetInputNormalsSelection(fieldName);}

protected:
  vtkMaskedGlyph2D();
  ~vtkMaskedGlyph2D() override;

  int RequestData(vtkInformation *info,
                          vtkInformationVector **inInfoVec,
                          vtkInformationVector *outInfoVec) override;

  vtkMaskPoints *MaskPoints;
  int MaximumNumberOfPoints;
  int UseMaskPoints;

private:
  vtkMaskedGlyph2D(const vtkMaskedGlyph2D &); // Not implemented.
  void operator=(const vtkMaskedGlyph2D &);   // Not implemented.
};

#endif
