/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date: 2005/06/28 12:37:25 $
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include <QStringList>

QStringList QmitkSegmentationView::GetDefaultOrganColorString()
{
  QStringList organColors;

  AppendToOrganList(organColors, "Ankle", 255, 0, 0);
  AppendToOrganList(organColors, "Appendix", 255, 0, 0);
  AppendToOrganList(organColors, "Blood vessels", 255, 49, 49);
  AppendToOrganList(organColors, "Bone", 213, 213, 213);
  AppendToOrganList(organColors, "Brain", 255, 156, 202);
  AppendToOrganList(organColors, "Bronchial tree", 49, 104, 255);
  AppendToOrganList(organColors, "Coccyx", 255, 0, 0);
  AppendToOrganList(organColors, "Colon", 255, 0, 0);
  AppendToOrganList(organColors, "Cyst", 255, 0, 0);
  AppendToOrganList(organColors, "Elbow", 255, 0, 0);
  AppendToOrganList(organColors, "Eye", 255, 0, 0);
  AppendToOrganList(organColors, "Fallopian tube", 255, 0, 0);
  AppendToOrganList(organColors, "Fat", 255, 43, 238);
  AppendToOrganList(organColors, "Gall Bladder", 86, 127, 24);
  AppendToOrganList(organColors, "Hand", 255, 0, 0);
  AppendToOrganList(organColors, "Heart", 235, 29, 50);
  AppendToOrganList(organColors, "Hip", 255, 0, 0);
  AppendToOrganList(organColors, "Kidney", 211, 63, 0);
  AppendToOrganList(organColors, "Knee", 255, 0, 0);
  AppendToOrganList(organColors, "Larynx", 255, 0, 0);
  AppendToOrganList(organColors, "Liver", 255, 204, 61);
  AppendToOrganList(organColors, "Lung", 107, 220, 255);
  AppendToOrganList(organColors, "Lymph node", 255, 0, 0);
  AppendToOrganList(organColors, "Muscle", 255, 69, 106);
  AppendToOrganList(organColors, "Nerve", 255, 234, 79);
  AppendToOrganList(organColors, "Nose", 255, 0, 0);
  AppendToOrganList(organColors, "Oesophagus", 255, 0, 0);
  AppendToOrganList(organColors, "Ovaries", 255, 0, 0);
  AppendToOrganList(organColors, "Pancreas", 249, 171, 61);
  AppendToOrganList(organColors, "Pelvis", 255, 0, 0);
  AppendToOrganList(organColors, "Penis", 255, 0, 0);
  AppendToOrganList(organColors, "Pharynx", 255, 0, 0);
  AppendToOrganList(organColors, "Prostate", 255, 0, 0);
  AppendToOrganList(organColors, "Rectum", 255, 0, 0);
  AppendToOrganList(organColors, "Sacrum", 255, 0, 0);
  AppendToOrganList(organColors, "Seminal vesicle", 255, 0, 0);
  AppendToOrganList(organColors, "Shoulder", 255, 0, 0);
  AppendToOrganList(organColors, "Spinal cord", 245, 249, 61);
  AppendToOrganList(organColors, "Spleen", 249, 108, 61);
  AppendToOrganList(organColors, "Stomach", 249, 108, 61);
  AppendToOrganList(organColors, "Teeth", 255, 252, 216);
  AppendToOrganList(organColors, "Testicles", 255, 0, 0);
  AppendToOrganList(organColors, "Thyroid", 255, 246, 148);
  AppendToOrganList(organColors, "Tongue", 255, 0, 0);
  AppendToOrganList(organColors, "Tumor", 147, 112, 17);
  AppendToOrganList(organColors, "Urethra", 248, 255, 50);
  AppendToOrganList(organColors, "Urinary bladder", 248, 255, 50);
  AppendToOrganList(organColors, "Uterus", 255, 0, 0);
  AppendToOrganList(organColors, "Vagina", 255, 0, 0);
  AppendToOrganList(organColors, "Vertebra", 255, 0, 0);
  AppendToOrganList(organColors, "Wrist", 255, 0, 0);

  return organColors;
}

void QmitkSegmentationView::UpdateOrganList(QStringList& organColors, const QString& organname, mitk::Color color)
{
  QString listElement( organname + QColor(color.GetRed() * 255 , color.GetGreen() * 255 , color.GetBlue() * 255).name() );
  
  // remove previous definition if necessary
  int oldIndex = organColors.indexOf( QRegExp(QRegExp::escape(organname) + "#......", Qt::CaseInsensitive));
  if (oldIndex < 0 || organColors.at(oldIndex) != listElement )
  {
    if (oldIndex >= 0)
    {
      organColors.removeAt( oldIndex );
    }

    // add colored organ name AND sort list
    organColors.append( listElement );
    organColors.sort();
  }
}

void QmitkSegmentationView::AppendToOrganList(QStringList& organColors, const QString& organname, int r, int g, int b)
{
  organColors.append( organname + QColor(r, g, b).name() );
}

