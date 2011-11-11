/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKGEOMETRY2DDATAVTKMAPPER3D_H_HEADER_INCLUDED_C196C71F
#define MITKGEOMETRY2DDATAVTKMAPPER3D_H_HEADER_INCLUDED_C196C71F

#include <MitkExports.h>
#include "mitkVtkMapper3D.h"
#include "mitkDataStorage.h"
#include "mitkGeometry2DDataToSurfaceFilter.h"
#include "mitkWeakPointer.h"

#include <vtkSystemIncludes.h>
#include <vtkCleanPolyData.h>

class vtkActor;
class vtkPolyDataMapper;
class vtkAssembly;
class vtkFeatureEdges;
class vtkTubeFilter;
class vtkTransformPolyDataFilter;
class vtkHedgeHog;

namespace mitk {

class Geometry2DData;
class BaseRenderer;
class ImageVtkMapper2D;
class DataStorage;

/**
 *  \brief Vtk-based mapper to display a Geometry2D in a 3D window
 *  \ingroup Mapper
 *
 *  Uses a Geometry2DDataToSurfaceFilter object to create a vtkPolyData representation of a given Geometry2D instance.
 *  Geometry2D may either contain a common flat plane or a curved plane (ThinPlateSplineCurvedGeometry).
 *
 *  The vtkPolyData object is then decorated by a colored tube on the edges and by image textures if possible
 *  (currently this requires that there is a 2D render window rendering the same geometry as this mapper).
 *
 * Properties that influence rendering are:
 *
 *   - \b "color": (ColorProperty) Color of the tubed frame.
 *   - \b "xresolution": (FloatProperty) Resolution (=number of tiles) in x direction. Only relevant for ThinPlateSplineCurvedGeometry
 *   - \b "yresolution": (FloatProperty) Resolution (=number of tiles) in y direction. Only relevant for ThinPlateSplineCurvedGeometry
 *   - \b "draw normals 3D": (BoolProperty) If true, a vtkHedgeHog is used to display normals for the generated surface object. Useful to distinguish front and back of a plane. Hedgehogs are colored according to "front color" and "back color"
 *   - \b "color two sides": (BoolProperty) If true, front and back side of the plane are colored differently ("front color" and "back color")
 *   - \b "invert normals": (BoolProperty) Inverts front/back for display.
 *   - \b "front color": (ColorProperty) Color for front side of the plane
 *   - \b "back color": (ColorProperty) Color for back side of the plane
 *   - \b "material.representation": (BoolProperty) Choose the representation to draw the mesh in (Surface, Wireframe, Point Cloud)
 *   - \b "surfacegeometry": TODO: Add documentation
 *   - \b "LookupTable": (LookupTableProperty) Set the lookuptable to render with.
 *
 *  Note: The following properties are set for each image individually, and thus, also influence the rendering of this mapper:
 *
 *   - \b "texture interpolation": (BoolProperty) Turn on/off the texture interpolation of each image
 *   - \b "use color": (BoolProperty) Decide whether we want to use the color property or a lookuptable.
 *   - \b "binary": (BoolProperty) Binary image handling: Color the value=1.0 with the color property and make the background (value=0.0) of the image translucent.
 *   - \b "layer": (IntProperty) Controls what image is considered "on top" of another. In the case that two should inhabit the same space, higher layer occludes lower layer.
 *   - \b "opacity": (FloatProperty) Set the opacity for each rendered image.
 *   - \b "color": (FloatProperty) Set the color for each rendered image.
 *
 *  The internal filter pipeline which combines a (sometimes deformed) 2D surface
 *  with a nice frame and image textures is illustrated in the following sketch:
 *
 *  \image html mitkGeometry2DDataVtkMapper3D.png "Internal filter pipeline"
 *
 */
class MITK_CORE_EXPORT Geometry2DDataVtkMapper3D : public VtkMapper3D
{
public:
  mitkClassMacro(Geometry2DDataVtkMapper3D, VtkMapper3D);

  itkNewMacro(Geometry2DDataVtkMapper3D);

  /**
   * Overloaded since the displayed color-frame of the image mustn't be
   * transformed after generation of poly data, but before (vertex coordinates
   * only)
   */
  virtual vtkProp *GetVtkProp(mitk::BaseRenderer *renderer);
  virtual void UpdateVtkTransform(mitk::BaseRenderer *renderer);

  /**
   *  \brief Get the Geometry2DData to map
   */
  virtual const Geometry2DData *GetInput();

  /**
   * \brief All images found when traversing the (sub-) tree starting at
   * \a iterator which are resliced by an ImageVtkMapper2D will be mapped.
   * This method is used to set the data storage to traverse. This offers
   * the possibility to use this mapper for other data storages (not only
   * the default data storage).
   */
  virtual void SetDataStorageForTexture(mitk::DataStorage* storage);

protected:

  typedef std::multimap< int, vtkActor * > LayerSortedActorList;

  Geometry2DDataVtkMapper3D();

  virtual ~Geometry2DDataVtkMapper3D();

  virtual void GenerateDataForRenderer(BaseRenderer* renderer);

  void ProcessNode( DataNode * node, BaseRenderer* renderer, Surface * surface, LayerSortedActorList &layerSortedActors );

  void ImageMapperDeletedCallback( itk::Object *caller, const itk::EventObject &event );

  /** \brief general PropAssembly to hold the entire scene */
  vtkAssembly *m_Prop3DAssembly;

  /** \brief PropAssembly to hold the planes */
  vtkAssembly *m_ImageAssembly;

  Geometry2DDataToSurfaceFilter::Pointer m_SurfaceCreator;

  BoundingBox::Pointer m_SurfaceCreatorBoundingBox;

  BoundingBox::PointsContainer::Pointer m_SurfaceCreatorPointsContainer;

  /** \brief Edge extractor for tube-shaped frame */
  vtkFeatureEdges *m_Edges;

  /** \brief Filter to apply object transform to the extracted edges */
  vtkTransformPolyDataFilter *m_EdgeTransformer;

  /** \brief Source to create the tube-shaped frame  */
  vtkTubeFilter *m_EdgeTuber;

  /** \brief Mapper for the tube-shaped frame  */
  vtkPolyDataMapper *m_EdgeMapper;

  /** \brief Actor for the tube-shaped frame */
  vtkActor *m_EdgeActor;

  /** \brief Mapper for black plane background */
  vtkPolyDataMapper *m_BackgroundMapper;

  /** \brief Actor for black plane background */
  vtkActor *m_BackgroundActor;

  /** \brief Transforms the suface before applying the glyph filter */
  vtkTransformPolyDataFilter* m_NormalsTransformer;

  /** \brief Mapper for normals representation (thin lines) */
  vtkPolyDataMapper* m_FrontNormalsMapper;
  vtkPolyDataMapper* m_BackNormalsMapper;

  /** \brief  Generates lines for surface normals */
  vtkHedgeHog* m_FrontHedgeHog;
  vtkHedgeHog* m_BackHedgeHog;

  /** \brief Actor to hold the normals arrows */
  vtkActor* m_FrontNormalsActor;
  vtkActor* m_BackNormalsActor;

  /** Cleans the polyline in order to avoid phantom boundaries */
  vtkCleanPolyData *m_Cleaner;

  /** Internal flag, if actors for normals are already added to m_Prop3DAssembly*/
  bool m_NormalsActorAdded;

  /** \brief The DataStorage defines which part of the data tree is traversed for renderering. */
  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;

  class MITK_CORE_EXPORT ActorInfo
  {
  public:
    vtkActor * m_Actor;
    // we do not need a smart-pointer, because we delete our
    // connection, when the referenced mapper is destroyed
    itk::Object* m_Sender;
    unsigned long m_ObserverID;

    void Initialize(vtkActor* actor, itk::Object* sender, itk::Command* command);

    ActorInfo();
    ~ActorInfo();
  };

  /** \brief List holding the vtkActor to map the image into 3D for each
   * ImageMapper
   */
  typedef std::map< ImageVtkMapper2D *, ActorInfo > ActorList;
  ActorList m_ImageActors;

  // responsiblity to remove the observer upon its destruction
  typedef itk::MemberCommand< Geometry2DDataVtkMapper3D > MemberCommandType;
  MemberCommandType::Pointer m_ImageMapperDeletedCommand;
};
} // namespace mitk
#endif /* MITKGEOMETRY2DDATAVTKMAPPER3D_H_HEADER_INCLUDED_C196C71F */
