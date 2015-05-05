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

#ifndef TubeGraphVtkMapper3D_H_HEADER_INCLUDED
#define TubeGraphVtkMapper3D_H_HEADER_INCLUDED

#include <MitkTubeGraphExports.h>
#include <mitkLocalStorageHandler.h>

#include "mitkVtkMapper3D.h"
#include "mitkTubeGraphProperty.h"
#include "mitkTubeGraph.h"
#include "mitkCircularProfileTubeElement.h"

#include <vtkActor.h>
#include <vtkAppendPolyData.h>
#include <vtkAssembly.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

namespace mitk
{
  /**
  * 3D Mapper for mitk::Graph< TubeGraphVertex, TubeGraphEdge >. This mapper creates tubes
  * around each tubular structure by using vtkTubeFilter.
  *
  */

  class MITKTUBEGRAPH_EXPORT TubeGraphVtkMapper3D : public VtkMapper3D
  {
  public:

    /* Typedefs */
    typedef TubeGraph::EdgeDescriptorType EdgeDescriptorType;
    typedef TubeGraph::VertexDescriptorType VertexDescriptorType;

    mitkClassMacro(TubeGraphVtkMapper3D, VtkMapper3D);
    itkNewMacro(Self);
    /**
    * Returns the input data object of the given filter. In this
    * case, a mitk::Graph< TubeGraphVertex, TubeGraphEdge > is returned.
    */
    virtual const TubeGraph* GetInput();
    virtual vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

  protected:

    TubeGraphVtkMapper3D();
    virtual ~TubeGraphVtkMapper3D();

    /**
    * This method is called, each time a specific renderer is updated.
    */
    virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer) override;

    /**
    * Generate vtkPolyData containing the tube centerlines and
    * sets these as input for a vtkTubeFilter, which generates tubes
    * around the edges. Also generates vtkActors for each vertex. Here
    * the vtkPolyData are vtkSphereSources. All poly data will be clipped
    * with each other on an furcation. So you get end-caps and connecting
    * pieces from the spheres. Clipping the tubes with each other avoids
    * structures within the general view.
    */
    virtual void GenerateTubeGraphData(mitk::BaseRenderer* renderer);

    /**
    * Render only the visual information like color or visibility new.
    */
    virtual void RenderTubeGraphPropertyInformation(mitk::BaseRenderer* renderer);


    /**
    * Converts a single tube into a vtkPolyData. Each point of the
    * tube surface is labeled with the tube id.
    */
    void GeneratePolyDataForTube(TubeGraphEdge& edge, const TubeGraph::Pointer& graph, const TubeGraphProperty::Pointer& graphProperty, mitk::BaseRenderer* renderer);
    void GeneratePolyDataForFurcation(TubeGraphVertex& vertex, const TubeGraph::Pointer& graph, mitk::BaseRenderer* renderer);
    void ClipPolyData(TubeGraphVertex& vertex, const TubeGraph::Pointer& graph, const TubeGraphProperty::Pointer& graphProperty, mitk::BaseRenderer* renderer);
  private:
    bool ClipStructures();

    class LocalStorage : public mitk::Mapper::BaseLocalStorage
    {
    public:
      vtkSmartPointer<vtkAssembly> m_vtkTubeGraphAssembly;
      std::map<TubeGraph::TubeDescriptorType, vtkSmartPointer<vtkActor> > m_vtkTubesActorMap;
      std::map<TubeGraph::VertexDescriptorType, vtkSmartPointer<vtkActor> > m_vtkSpheresActorMap;

      itk::TimeStamp m_lastGenerateDataTime;
      itk::TimeStamp m_lastRenderDataTime;

      LocalStorage()
      {
        m_vtkTubeGraphAssembly = vtkSmartPointer<vtkAssembly>::New();
      }

      ~LocalStorage()
      {
      }
    };

    LocalStorageHandler<LocalStorage> m_LSH;

  };
} // namespace

#endif
