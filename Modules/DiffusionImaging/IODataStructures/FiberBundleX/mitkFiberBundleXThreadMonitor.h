
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
#include <QString>



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


    void setTextL1(QString);
    QString getTextL1();
    
    QString         getBracketOpen();
    mitk::Point2D   getBracketOpenPosition();
    void            setBracketOpenPosition(mitk::Point2D);
    int             getBracketOpenOpacity(); // range 0 - 10,  multiplicationfactor 0.1 (in mapper)
    
    QString         getBracketClose();
    mitk::Point2D   getBracketClosePosition();
    void            setBracketClosePosition(mitk::Point2D);
    int             getBracketCloseOpacity(); // range 0 - 10,  multiplicationfactor 0.1 (in mapper)

    QString         getHeading();
    mitk::Point2D   getHeadingPosition();
    void            setHeadingPosition(mitk::Point2D);
    int             getHeadingOpacity(); // range 0 - 10,  multiplicationfactor 0.1 (in mapper)
    void            setHeadingOpacity(int);
    
    QString         getMask();
    mitk::Point2D   getMaskPosition();
    void            setMaskPosition(mitk::Point2D);
    int             getMaskOpacity(); // multiplicationfactor 0.1 (in mapper)
    void            setMaskOpacity(int);
    
    
  protected:
    FiberBundleXThreadMonitor();
    virtual ~FiberBundleXThreadMonitor();
    

  private:
    QString m_monitorBracketOpen;
    mitk::Point2D m_monitorBracketOpenPosition;
    
    QString m_monitorBracketClose;
    mitk::Point2D m_monitorBracketClosePosition;

    QString m_monitorHeading;
    mitk::Point2D m_monitorHeadingPosition;
    int m_monitorHeadingOpacity;
    
    QString m_monitorMask;
    mitk::Point2D m_monitorMaskPosition;
    int m_monitorMaskOpacity;
    
    QString m_Label1;
    

    
        

    
  };

} // namespace mitk

#endif /*  _MITK_FiberBundleX_H */
