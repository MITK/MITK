
/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Module:    $RCSfile$
 Language:  C++
 Date:      $Date$
 Version:   $Revision: 11989 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#ifndef _MITK_FiberBundleXThreadMonitor_H
#define _MITK_FiberBundleXThreadMonitor_H

//includes for MITK datastructure
#include "mitkBaseData.h"
#include "MitkDiffusionImagingExports.h"



namespace mitk {

  /**
   * \brief Base Class for Fiber Bundles;   */
  class  MitkDiffusionImaging_EXPORT FiberBundleXThreadMonitor : public BaseData
  {
  public:
  

    // ======virtual methods must have======
    virtual void UpdateOutputInformation();
    virtual void SetRequestedRegionToLargestPossibleRegion();
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
    virtual bool VerifyRequestedRegion();
    virtual void SetRequestedRegion( itk::DataObject *data );
    //=======================================
    
    mitkClassMacro( FiberBundleXThreadMonitor, BaseData );
    itkNewMacro( Self );



  protected:
    FiberBundleXThreadMonitor();
    virtual ~FiberBundleXThreadMonitor();

  private:
    
        

    
  };

} // namespace mitk

#endif /*  _MITK_FiberBundleX_H */
