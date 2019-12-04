/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __vtkMaskedGlyph2D_h
#define __vtkMaskedGlyph2D_h

#include "MitkMapperExtExports.h"

#include "mitkCommon.h"
#include "vtkGlyph2D.h"

class vtkMaskPoints;
/**
 * This class masked points of the input data set and glyphs
 * only the selected poitns. Points may be selected either by
 * random or by ratio.
 * Additionally, this class allows to set the InputScalars,
 * InputVectors and InputNormals by their field name in the
 * input dataset.
 */
class MITKMAPPEREXT_EXPORT vtkMaskedGlyph2D : public vtkGlyph2D
{
public:
  vtkTypeMacro(vtkMaskedGlyph2D, vtkGlyph2D);
  void PrintSelf(ostream &os, vtkIndent indent) override;

  /**
   * Constructor
   */
  static vtkMaskedGlyph2D *New();

  /**
   * Limit the number of points to glyph
   */
  vtkSetMacro(MaximumNumberOfPoints, int);
  vtkGetMacro(MaximumNumberOfPoints, int);

  /**
   * Set the input to this filter.
   */
  virtual void SetInput(vtkDataSet *input);

  /**
   * Set/get whether to mask points
   */
  vtkSetMacro(UseMaskPoints, int);
  vtkGetMacro(UseMaskPoints, int);

  /**
   * Set/get flag to cause randomization of which points to mask.
   */
  void SetRandomMode(int mode);
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
