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

#ifndef COLORBARAnnotation_H
#define COLORBARAnnotation_H

#include "MitkAnnotationExports.h"
#include <mitkLocalStorageHandler.h>
#include <mitkVtkAnnotation.h>
#include <vtkLookupTable.h>
#include <vtkSmartPointer.h>

class vtkScalarBarActor;

namespace mitk
{
  /** \brief Displays configurable scales on the renderwindow. The scale is determined by the image spacing. */
  class MITKANNOTATION_EXPORT ColorBarAnnotation : public mitk::VtkAnnotation
  {
  public:
    class LocalStorage : public mitk::Annotation::BaseLocalStorage
    {
    public:
      /** \brief Actor of a 2D render window. */
      vtkSmartPointer<vtkScalarBarActor> m_ScalarBarActor;

      /** \brief Timestamp of last update of stored data. */
      itk::TimeStamp m_LastUpdateTime;

      /** \brief Default constructor of the local storage. */
      LocalStorage();
      /** \brief Default deconstructor of the local storage. */
      ~LocalStorage();
    };

    mitkClassMacro(ColorBarAnnotation, mitk::VtkAnnotation);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      void SetDrawAnnotations(bool annotations);
    bool GetDrawAnnotations() const;

    void SetOrientationToHorizontal();
    void SetOrientationToVertical();
    void SetOrientation(int orientation);
    int GetOrientation() const;

    void SetMaxNumberOfColors(int numberOfColors);
    int GetMaxNumberOfColors() const;

    void SetNumberOfLabels(int numberOfLabels);
    int GetNumberOfLabels() const;

    void SetLookupTable(vtkSmartPointer<vtkLookupTable> table);
    vtkSmartPointer<vtkLookupTable> GetLookupTable() const;

    void SetDrawTickLabels(bool ticks);
    bool GetDrawTickLabels() const;

    void SetAnnotationTextScaling(bool scale);
    bool GetAnnotationTextScaling() const;

  protected:
    /** \brief The LocalStorageHandler holds all LocalStorages for the render windows. */
    mutable mitk::LocalStorageHandler<LocalStorage> m_LSH;

    vtkProp *GetVtkProp(BaseRenderer *renderer) const override;
    void UpdateVtkAnnotation(BaseRenderer *renderer) override;

    /** \brief explicit constructor which disallows implicit conversions */
    explicit ColorBarAnnotation();

    /** \brief virtual destructor in order to derive from this class */
    ~ColorBarAnnotation() override;

  private:
    /** \brief copy constructor */
    ColorBarAnnotation(const ColorBarAnnotation &);

    /** \brief assignment operator */
    ColorBarAnnotation &operator=(const ColorBarAnnotation &);
  };

} // namespace mitk
#endif // COLORBARAnnotation_H
