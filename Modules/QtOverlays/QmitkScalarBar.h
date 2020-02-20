/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKSCALARBAR_H_HEADER_INCLUDED_C10DC4EB
#define MITKSCALARBAR_H_HEADER_INCLUDED_C10DC4EB

#include <MitkQtOverlaysExports.h>
#include <mitkCommon.h>

#include <QPen>
#include <QWidget>

class MITKQTOVERLAYS_EXPORT QmitkScalarBar : public QWidget
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
  QmitkScalarBar(QWidget *parent = nullptr);

  /**
  * @brief Default Destructor
  **/
  ~QmitkScalarBar() override;

  virtual void SetScaleFactor(double scale);

  virtual void SetAlignment(alignment align);

  void SetPen(const QPen &pen);

  void SetNumberOfSubdivisions(unsigned int subs);

  unsigned int GetNumberOfSubdivisions();

protected:
  void paintEvent(QPaintEvent *event) override;

  void SetupGeometry(alignment align);

  void CleanUpLines();
  // void moveEvent(QMoveEvent*);

  alignment m_Alignment;

  double m_ScaleFactor;

  QLine *m_MainLine;

  std::vector<QLine *> m_SubDivisionLines;

  QPen m_Pen;

  unsigned int m_NumberOfSubDivisions;
};

#endif /* MITKSCALARBAR_H_HEADER_INCLUDED_C10DC4EB */
