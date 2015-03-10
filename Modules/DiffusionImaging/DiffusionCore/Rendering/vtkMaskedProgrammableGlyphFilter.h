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

#ifndef __vtkMaskedProgrammableGlyphFilter_h
#define __vtkMaskedProgrammableGlyphFilter_h

#include <MitkDiffusionCoreExports.h>

#include "vtkProgrammableGlyphFilter.h"
#include "mitkBaseGeometry.h"

class vtkMaskPoints;
/**
 * This class masked points of the input data set and glyphs
 * only the selected poitns. Points may be selected either by
 * random or by ratio.
 * Additionally, this class allows to set the InputScalars,
 * InputVectors and InputNormals by their field name in the
 * input dataset.
 */
class MITKDIFFUSIONCORE_EXPORT vtkMaskedProgrammableGlyphFilter : public vtkProgrammableGlyphFilter
{
public:
  vtkTypeMacro(vtkMaskedProgrammableGlyphFilter,vtkProgrammableGlyphFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  /**
   * Constructor
   */
  static vtkMaskedProgrammableGlyphFilter *New();

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
  // * By default this in NULL and the filter will use the active scalar array.
  // */
  //vtkGetStringMacro(InputScalarsSelection);
  //void SelectInputScalars(const char *fieldName)
  //  {this->SetInputScalarsSelection(fieldName);}

  ///**
  // * If you want to use an arbitrary vectors array, then set its name here.
  // * By default this in NULL and the filter will use the active vector array.
  // */
  //vtkGetStringMacro(InputVectorsSelection);
  //void SelectInputVectors(const char *fieldName)
  //  {this->SetInputVectorsSelection(fieldName);}

  ///**
  // * If you want to use an arbitrary normals array, then set its name here.
  // * By default this in NULL and the filter will use the active normal array.
  // */
  //vtkGetStringMacro(InputNormalsSelection);
  //void SelectInputNormals(const char *fieldName)
  //  {this->SetInputNormalsSelection(fieldName);}

  void SetGeometry(mitk::BaseGeometry::Pointer geo)
  { this->m_Geometry = geo; }
  mitk::BaseGeometry::Pointer GetGeometry()
  { return this->m_Geometry; }

protected:
  vtkMaskedProgrammableGlyphFilter();
  ~vtkMaskedProgrammableGlyphFilter();

  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  vtkMaskPoints *MaskPoints;
  int MaximumNumberOfPoints;
  int UseMaskPoints;

  mitk::BaseGeometry::Pointer m_Geometry;

private:
  vtkMaskedProgrammableGlyphFilter(const vtkMaskedProgrammableGlyphFilter&);  // Not implemented.
  void operator=(const vtkMaskedProgrammableGlyphFilter&);  // Not implemented.
};

#endif
