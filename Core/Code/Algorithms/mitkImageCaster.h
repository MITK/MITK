/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:56:03 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 17179 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkImageCast.h>
#include <itkImage.h>
#include <itkCastImageFilter.h>
#include <vtkRenderWindow.h>
#include <mitkSurface.h>

namespace mitk
{
  ///
  /// \brief This class is just a proxy for global functions which are needed by the
  /// python wrapping process since global functions cannnot be wrapped. Static method
  /// can be wrapped though.
  ///
  class MITK_CORE_EXPORT ImageCaster
  {
  public:
      // short
      static void CastToItkImage(const mitk::Image*, itk::SmartPointer<itk::Image<short, 2> >&);
      static void CastToMitkImage(const itk::Image<short, 2>*, itk::SmartPointer<mitk::Image>&);
      static void CastToItkImage(const mitk::Image*, itk::SmartPointer<itk::Image<short, 3> >&);
      static void CastToMitkImage(const itk::Image<short, 3>*, itk::SmartPointer<mitk::Image>&);

      // unsigned short
      static void CastToItkImage(const mitk::Image*, itk::SmartPointer<itk::Image<unsigned short, 2> >&);
      static void CastToMitkImage(const itk::Image<unsigned short, 2>*, itk::SmartPointer<mitk::Image>&);
      static void CastToItkImage(const mitk::Image*, itk::SmartPointer<itk::Image<unsigned short, 3> >&);
      static void CastToMitkImage(const itk::Image<unsigned short, 3>*, itk::SmartPointer<mitk::Image>&);

      // char
      static void CastToItkImage(const mitk::Image*, itk::SmartPointer<itk::Image<char, 2> >&);
      static void CastToMitkImage(const itk::Image<char, 2>*, itk::SmartPointer<mitk::Image>&);
      static void CastToItkImage(const mitk::Image*, itk::SmartPointer<itk::Image<char, 3> >&);
      static void CastToMitkImage(const itk::Image<char, 3>*, itk::SmartPointer<mitk::Image>&);

      // unsigned char
      static void CastToItkImage(const mitk::Image*, itk::SmartPointer<itk::Image<unsigned char, 2> >&);
      static void CastToMitkImage(const itk::Image<unsigned char, 2>*, itk::SmartPointer<mitk::Image>&);
      static void CastToItkImage(const mitk::Image*, itk::SmartPointer<itk::Image<unsigned char, 3> >&);
      static void CastToMitkImage(const itk::Image<unsigned char, 3>*, itk::SmartPointer<mitk::Image>&);

      static void CastToItkImage(const mitk::Image*, itk::SmartPointer<itk::Image<int, 3> >&);
      static void CastToMitkImage(const itk::Image<int, 3>*, itk::SmartPointer<mitk::Image>&);

      static void CastToItkImage(const mitk::Image*, itk::SmartPointer<itk::Image<unsigned int, 3> >&);
      static void CastToMitkImage(const itk::Image<unsigned int, 3>*, itk::SmartPointer<mitk::Image>&);

      static void CastToItkImage(const mitk::Image*, itk::SmartPointer<itk::Image<float, 3> >&);
      static void CastToMitkImage(const itk::Image<float, 3>*, itk::SmartPointer<mitk::Image>&);

      static void CastToItkImage(const mitk::Image*, itk::SmartPointer<itk::Image<double, 3> >&);
      static void CastToMitkImage(const itk::Image<double, 3>*, itk::SmartPointer<mitk::Image>&);

      static void CastBaseData(mitk::BaseData* const, itk::SmartPointer<mitk::Image>&);

  };

  class MITK_CORE_EXPORT Caster
  {
  public:
    static void Cast(BaseData* dat, Surface* surface);
  };

  class MITK_CORE_EXPORT RendererAccess
  {
  public:
    static void Set3DRenderer(vtkRenderer* renderer);
    static vtkRenderer* Get3DRenderer();
  protected:
    static vtkRenderer* m_3DRenderer;
  };
}
