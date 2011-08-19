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


#ifndef FiberBundleXMapper3D_H_HEADER_INCLUDED
#define FiberBundleXMapper3D_H_HEADER_INCLUDED

#include <mitkBaseData.h> //?? necessary
#include <MitkDiffusionImagingExports.h>
#include <mitkVtkMapper3D.h>
#include <mitkFiberBundleX.h>



class vtkPropAssembly;




namespace mitk {

  //##Documentation
  //## @brief Mapper for FiberBundleX
  //## @ingroup Mapper

  class MitkDiffusionImaging_EXPORT FiberBundleXMapper3D : public VtkMapper3D
  {
  public:

    mitkClassMacro(FiberBundleXMapper3D, VtkMapper3D);
    itkNewMacro(Self);

    //========== essential implementation for 3D mapper ========
    const FiberBundleX* GetInput();
    virtual vtkProp *GetVtkProp(mitk::BaseRenderer *renderer); //looks like depricated.. should be replaced bz GetViewProp()
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = NULL, bool overwrite = false );
    virtual void ApplyProperties(mitk::BaseRenderer* renderer);
    static void SetVtkMapperImmediateModeRendering(vtkMapper *mapper);
    virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer);
    virtual void GenerateData();
    //=========================================================
    
  protected:

    FiberBundleXMapper3D();
    virtual ~FiberBundleXMapper3D();

    void UpdateVtkObjects(); //??

    vtkPropAssembly* m_FiberAssembly;


  };

} // end namespace mitk




#endif /* FiberBundleXMapper3D_H_HEADER_INCLUDED */

