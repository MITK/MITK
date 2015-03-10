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

#ifndef __vtkMaskedGlyph3D_h
#define __vtkMaskedGlyph3D_h

#include "MitkMapperExtExports.h"

#include "vtkGlyph3D.h"

#include "mitkCommon.h"

class vtkMaskPoints;
/**
 * This class masked points of the input data set and glyphs
 * only the selected poitns. Points may be selected either by
 * random or by ratio.
 * Additionally, this class allows to set the InputScalars,
 * InputVectors and InputNormals by their field name in the
 * input dataset.
 */
class MITKMAPPEREXT_EXPORT vtkMaskedGlyph3D : public vtkGlyph3D
{
public:
  vtkTypeMacro(vtkMaskedGlyph3D,vtkGlyph3D);
  void PrintSelf(ostream& os, vtkIndent indent);

  /**
   * Constructor
   */
  static vtkMaskedGlyph3D *New();

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

  virtual void SetInputConnection(vtkAlgorithmOutput* input);

  using vtkGlyph3D::SetInputConnection;

protected:
  vtkMaskedGlyph3D();
  ~vtkMaskedGlyph3D();

  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  vtkMaskPoints *MaskPoints;
  int MaximumNumberOfPoints;
  int UseMaskPoints;

private:
  vtkMaskedGlyph3D(const vtkMaskedGlyph3D&);  // Not implemented.
  void operator=(const vtkMaskedGlyph3D&);  // Not implemented.
};

#endif
