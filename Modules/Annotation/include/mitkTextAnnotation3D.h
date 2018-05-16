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

#ifndef TextAnnotation3D_H
#define TextAnnotation3D_H

#include "MitkAnnotationExports.h"
#include <mitkLocalStorageHandler.h>
#include <mitkVtkAnnotation3D.h>

class vtkFollower;
class vtkVectorText;
class vtkTextActor3D;

namespace mitk
{
  /** \brief Displays at 3D position, always facing the camera */
  class MITKANNOTATION_EXPORT TextAnnotation3D : public mitk::VtkAnnotation3D
  {
  public:
    /** \brief Internal class holding the mapper, actor, etc. for each of the render windows */
    /**
       * To render the Annotation on transveral, coronal, and sagittal, the update method
       * is called for each renderwindow. For performance reasons, the corresponding data
       * for each view is saved in the internal helper class LocalStorage.
       * This allows rendering n views with just 1 mitkAnnotation using n vtkMapper.
       * */
    class LocalStorage : public mitk::Annotation::BaseLocalStorage
    {
    public:
      /** \brief Actor of a 2D render window. */
      vtkSmartPointer<vtkFollower> m_follower;

      vtkSmartPointer<vtkVectorText> m_textSource;

      /** \brief Timestamp of last update of stored data. */
      itk::TimeStamp m_LastUpdateTime;

      /** \brief Default constructor of the local storage. */
      LocalStorage();
      /** \brief Default deconstructor of the local storage. */
      ~LocalStorage();
    };

    mitkClassMacro(TextAnnotation3D, mitk::VtkAnnotation3D);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      protected :

      /** \brief The LocalStorageHandler holds all LocalStorages for the render windows. */
      mutable mitk::LocalStorageHandler<LocalStorage> m_LSH;

    vtkProp *GetVtkProp(BaseRenderer *renderer) const override;
    void UpdateVtkAnnotation(mitk::BaseRenderer *renderer) override;

    /** \brief explicit constructor which disallows implicit conversions */
    explicit TextAnnotation3D();

    /** \brief virtual destructor in order to derive from this class */
    ~TextAnnotation3D() override;

  private:
    /** \brief copy constructor */
    TextAnnotation3D(const TextAnnotation3D &);

    /** \brief assignment operator */
    TextAnnotation3D &operator=(const TextAnnotation3D &);
  };

} // namespace mitk
#endif // TextAnnotation3D_H
