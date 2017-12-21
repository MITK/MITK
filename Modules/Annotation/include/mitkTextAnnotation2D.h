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

#ifndef TEXTAnnotation2D_H
#define TEXTAnnotation2D_H

#include "MitkAnnotationExports.h"
#include <mitkLocalStorageHandler.h>
#include <mitkVtkAnnotation2D.h>
#include <vtkSmartPointer.h>

class vtkTextActor;
class vtkPropAssembly;

namespace mitk
{
  /** \brief Displays text on the renderwindow */
  class MITKANNOTATION_EXPORT TextAnnotation2D : public mitk::VtkAnnotation2D
  {
  public:
    class LocalStorage : public mitk::Annotation::BaseLocalStorage
    {
    public:
      /** \brief Actor of a 2D render window. */
      vtkSmartPointer<vtkTextActor> m_TextActor;

      vtkSmartPointer<vtkTextProperty> m_TextProp;

      vtkSmartPointer<vtkTextActor> m_STextActor;

      vtkSmartPointer<vtkTextProperty> m_STextProp;

      vtkSmartPointer<vtkPropAssembly> m_Assembly;

      /** \brief Timestamp of last update of stored data. */
      itk::TimeStamp m_LastUpdateTime;

      /** \brief Default constructor of the local storage. */
      LocalStorage();
      /** \brief Default deconstructor of the local storage. */
      ~LocalStorage();
    };

    mitkClassMacro(TextAnnotation2D, mitk::VtkAnnotation2D);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      Annotation::Bounds GetBoundsOnDisplay(BaseRenderer *renderer) const override;
    void SetBoundsOnDisplay(BaseRenderer *renderer, const Bounds &bounds) override;

  protected:
    /** \brief The LocalStorageHandler holds all LocalStorages for the render windows. */
    mutable mitk::LocalStorageHandler<LocalStorage> m_LSH;

    vtkProp *GetVtkProp(BaseRenderer *renderer) const override;
    vtkActor2D *GetVtkActor2D(BaseRenderer *renderer) const override;
    void UpdateVtkAnnotation2D(mitk::BaseRenderer *renderer) override;

    /** \brief explicit constructor which disallows implicit conversions */
    explicit TextAnnotation2D();

    /** \brief virtual destructor in order to derive from this class */
    ~TextAnnotation2D() override;

  private:
    /** \brief copy constructor */
    TextAnnotation2D(const TextAnnotation2D &);

    /** \brief assignment operator */
    TextAnnotation2D &operator=(const TextAnnotation2D &);
  };

} // namespace mitk
#endif // TEXTAnnotation2D_H
