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

#ifndef __mitkLabelSetImageVtkMapper2D_H_
#define __mitkLabelSetImageVtkMapper2D_H_

//MITK
#include "mitkCommon.h"
#include "SegmentationExports.h"

//MITK Rendering
#include "mitkBaseRenderer.h"
#include "mitkImageVtkMapper2D.h"
#include "mitkLabelSetImage.h"
#include "mitkExtractSliceFilter.h"

//VTK
#include <vtkSmartPointer.h>

class vtkActor;
class vtkPolyDataMapper;
class vtkPlaneSource;
class vtkTexture;
class vtkImageData;
class vtkLookupTable;
class vtkImageReslice;
class vtkPoints;
class vtkMitkThickSlicesFilter;
class vtkPolyData;
class vtkMitkLevelWindowFilter;
class vtkImageLabelOutline;
class vtkImageBlend;
class vtkPropAssembly;

namespace mitk {

/** \brief Mapper to resample and display 2D slices of a 3D labelset image.
 *
 * Properties that can be set for labelset images and influence this mapper are:
 *
 *   - \b "labelset.contour.all": (BoolProperty) whether to show all labels as contours or not
 *   - \b "labelset.contour.active": (BoolProperty) whether to show only the active label as a contour or not
 *   - \b "labelset.contour.width": (FloatProperty) line width of the contour

 * The default properties are:

 *   - \b "labelset.contour.all", mitk::BoolProperty::New( false ), renderer, overwrite )
 *   - \b "labelset.contour.active", mitk::BoolProperty::New( false ), renderer, overwrite )
 *   - \b "labelset.contour.width", mitk::FloatProperty::New( 2.0 ), renderer, overwrite )

 * \ingroup Mapper
 */
class Segmentation_EXPORT LabelSetImageVtkMapper2D : public ImageVtkMapper2D
{

public:
  /** Standard class typedefs. */
  mitkClassMacro( LabelSetImageVtkMapper2D,ImageVtkMapper2D );

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** \brief Checks whether this mapper needs to update itself and generate data. */
  virtual void Update(mitk::BaseRenderer * renderer);

  /** \brief Set the default properties for general image rendering. */
  static void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer = NULL, bool overwrite = false);

protected:

  /** Default constructor */
  LabelSetImageVtkMapper2D();
  /** Default destructor */
  virtual ~LabelSetImageVtkMapper2D();

  virtual void GenerateDataForRenderer(mitk::BaseRenderer *renderer);

  virtual void ApplyLookuptable(mitk::BaseRenderer* renderer);

  virtual void ApplyOpacity( mitk::BaseRenderer* renderer );

  virtual void ApplyColor( mitk::BaseRenderer* renderer );
};

} // namespace mitk

#endif // __mitkLabelSetImageVtkMapper2D_H_
