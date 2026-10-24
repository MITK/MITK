/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarFigureVtkMapper3D_h
#define mitkPlanarFigureVtkMapper3D_h

#include <MitkPlanarFigureExports.h>
#include <mitkLocalStorageHandler.h>
#include <mitkVtkMapper.h>

#include <vtkSmartPointer.h>

namespace mitk
{
  /**
   * \brief Draws planar figures in 3D render windows.
   *
   * The contours of the figure are drawn as lines in its plane. Closed
   * contours can be filled; holes stay open.
   *
   * Node properties:
   *   - \c planarfigure.3drendering (BoolProperty): whether the figure is shown in 3D, false by default
   *   - \c planarfigure.3drendering.fill (BoolProperty): whether closed contours are filled, false by default
   *   - \c color, \c opacity and \c line \c width as in 2D render windows
   */
  class MITKPLANARFIGURE_EXPORT PlanarFigureVtkMapper3D : public VtkMapper
  {
  public:
    mitkClassMacro(PlanarFigureVtkMapper3D, VtkMapper);
    itkFactorylessNewMacro(Self);

    static void SetDefaultProperties(DataNode *node, BaseRenderer *renderer = nullptr, bool overwrite = false);

    vtkProp *GetVtkProp(BaseRenderer *renderer) override;

    /** \brief Does nothing; the contours are drawn in world coordinates. */
    void UpdateVtkTransform(BaseRenderer *renderer) override;

  protected:
    PlanarFigureVtkMapper3D();
    ~PlanarFigureVtkMapper3D() override;

    void GenerateDataForRenderer(BaseRenderer *renderer) override;

  private:
    class LocalStorage : public Mapper::BaseLocalStorage
    {
    public:
      LocalStorage();
      ~LocalStorage() override;

      vtkSmartPointer<vtkActor> m_Actor;
      itk::ModifiedTimeType m_FigureTime = 0;
      bool m_IsFilled = false;
    };

    LocalStorageHandler<LocalStorage> m_LocalStorageHandler;
  };
}

#endif
