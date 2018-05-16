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

#ifndef LOGOAnnotation_H
#define LOGOAnnotation_H

#include "MitkAnnotationExports.h"
#include <mitkLocalStorageHandler.h>
#include <mitkVtkAnnotation.h>
#include <vtkSmartPointer.h>

class mitkVtkLogoRepresentation;
class vtkImageData;
class vtkImageReader2Factory;
class vtkImageImport;

namespace mitk
{
  /** \brief Displays a logo on the renderwindow */
  class MITKANNOTATION_EXPORT LogoAnnotation : public mitk::VtkAnnotation
  {
  public:
    class LocalStorage : public mitk::Annotation::BaseLocalStorage
    {
    public:
      /** \brief Actor of a 2D render window. */
      vtkSmartPointer<vtkImageData> m_LogoImage;
      vtkSmartPointer<mitkVtkLogoRepresentation> m_LogoRep;

      /** \brief Timestamp of last update of stored data. */
      itk::TimeStamp m_LastUpdateTime;

      /** \brief Default constructor of the local storage. */
      LocalStorage();
      /** \brief Default deconstructor of the local storage. */
      ~LocalStorage();
    };

    mitkClassMacro(LogoAnnotation, mitk::VtkAnnotation);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

    vtkSmartPointer<vtkImageReader2Factory> m_readerFactory;
    void SetLogoImage(vtkSmartPointer<vtkImageData> logo);

    void SetLogoImagePath(std::string text);
    std::string GetLogoImagePath() const;
    void LoadLogoImageFromPath();

    /** \brief The relative offset to the corner position */
    void SetOffsetVector(const Point2D &OffsetVector);
    Point2D GetOffsetVector() const;

    /** \brief The corner where the logo is displayed.
   0 = Bottom left
   1 = Bottom right
   2 = Top right
   3 = Top left
   4 = Center*/
    void SetCornerPosition(const int &corner);
    int GetCornerPosition() const;

    void SetRelativeSize(const float &size);
    float GetRelativeSize() const;

  protected:
    /** \brief The LocalStorageHandler holds all LocalStorages for the render windows. */
    mutable mitk::LocalStorageHandler<LocalStorage> m_LSH;

    vtkProp *GetVtkProp(BaseRenderer *renderer) const override;
    void UpdateVtkAnnotation(mitk::BaseRenderer *renderer) override;

    /** \brief explicit constructor which disallows implicit conversions */
    explicit LogoAnnotation();

    /** \brief virtual destructor in order to derive from this class */
    ~LogoAnnotation() override;

  private:

    vtkSmartPointer<vtkImageData> m_UpdatedLogoImage;
    vtkSmartPointer<vtkImageImport> m_VtkImageImport;

    /** \brief copy constructor */
    LogoAnnotation(const LogoAnnotation &);

    /** \brief assignment operator */
    LogoAnnotation &operator=(const LogoAnnotation &);
  };

} // namespace mitk
#endif // LOGOAnnotation_H
