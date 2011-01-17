/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Thu, 28 May 2009) $
Version:   $Revision: 17495 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKSCALARBAR_H_HEADER_INCLUDED_C10DC4EB
#define MITKSCALARBAR_H_HEADER_INCLUDED_C10DC4EB

#include <mitkCommon.h>
#include "OverlaysExports.h"

#include <QPen>
#include <QWidget>



class Overlays_EXPORT QmitkScalarBar : public QWidget
  {
    Q_OBJECT
  public:

    enum alignment 
    {
      vertical = 0,
      horizontal = 1
    };

    /**
    * @brief Default Constructor
    **/
    QmitkScalarBar( QWidget *parent = 0 );

    /**
    * @brief Default Destructor
    **/
    virtual ~QmitkScalarBar();

    virtual void SetScaleFactor( double scale );

    virtual void SetAlignment( alignment align );

    void SetPen( const QPen& pen );

    void SetNumberOfSubdivisions( unsigned int subs );

    unsigned int GetNumberOfSubdivisions();

  protected:
   
    void paintEvent(QPaintEvent* event);

    void SetupGeometry( alignment align );
   
    void CleanUpLines();
    //void moveEvent(QMoveEvent*);

    alignment m_Alignment;
    
    double m_ScaleFactor;

    QLine* m_MainLine;

    std::vector<QLine*> m_SubDivisionLines;

    QPen m_Pen;

    unsigned int m_NumberOfSubDivisions;

  };


#endif /* MITKSCALARBAR_H_HEADER_INCLUDED_C10DC4EB */


