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


#ifndef MITKSCALARBAR_H_HEADER_INCLUDED_C10DC4EB
#define MITKSCALARBAR_H_HEADER_INCLUDED_C10DC4EB

#include <mitkCommon.h>
#include <MitkQtOverlaysExports.h>

#include <QPen>
#include <QWidget>



class MitkQtOverlays_EXPORT QmitkScalarBar : public QWidget
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


