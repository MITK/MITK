/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 13900 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QMITKSLICEWIDGET_H_
#define QMITKSLICEWIDGET_H_

#include "ui_QmitkMaterialEditor.h"

#include <vtkSphereSource.h>
#include <vtkPolyData.h>
#include <Q3Grid>
#include <QmitkMaterialShowcase.h>
#include <qlayout.h>
#include <qcolordialog.h>
#include <qcolor.h>
#include <mitkRenderingManager.h>
#include <QWidget>

class QMITK_EXPORT QmitkMaterialEditor : public QDialog, public Ui::QmitkMaterialEditor
{
  Q_OBJECT

public:

  QmitkMaterialEditor(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags f = 0);
  virtual ~QmitkMaterialEditor();
  mitk::MaterialProperty::Color ValueToColor( int value );
  int ColorToValue( mitk::MaterialProperty::Color color );
  mitk::MaterialPropertyVectorContainer * GetMaterialProperties();

  public slots:
    void OnLineWidthChanged( int value );
    void Initialize( mitk::MaterialProperty * materialProperty );
    void Initialize( mitk::MaterialPropertyVectorContainer::Pointer materialPropertyVectorContainer );
    void SetActiveShowcase( unsigned int number );
    void setInline( bool i );

  protected slots:
    void OnColorChanged( int value );
    void OnSpecularPowerChanged( int value );
    void OnOpacityChanged( int value );
    void OnOKClicked();
    void OnCancelClicked();
    void OnPreviewClicked();
    void Initialize();
    void ClearMaterialProperties();
    void OnColorSelectButtonClickedClicked();
    void OnCoefficientsChanged( int value );
    void OnInterpolationChanged( int item );
    void OnRepresentationChanged( int item );
    void OnMaterialShowcaseSelected( QmitkMaterialShowcase * showcase );

  signals:
    void ChangesAccepted(QmitkMaterialEditor*);

  protected:
    mitk::DataTree::Pointer m_DataTree;
    mitk::MaterialPropertyVectorContainer::Pointer m_MaterialProperties;
    std::vector<QmitkMaterialShowcase*> m_Showcases;
    unsigned int m_ActiveShowcase;
    mitk::MaterialPropertyVectorContainer::Pointer m_OriginalMaterialProperties;
    bool m_Inline;

};

#endif /*QMITKSLICEWIDGET_H_*/