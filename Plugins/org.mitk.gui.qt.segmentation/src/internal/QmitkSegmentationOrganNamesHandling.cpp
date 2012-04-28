/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

  AppendToOrganList(organColors, "Ankle", 255, 255, 153);
  AppendToOrganList(organColors, "Appendix", 161, 107, 54);
  AppendToOrganList(organColors, "Blood vessels", 255, 49, 49);
  AppendToOrganList(organColors, "Bone", 255, 255, 153);
  AppendToOrganList(organColors, "Brain", 255, 156, 202);
  AppendToOrganList(organColors, "Bronchial tree", 0, 160, 209);
  AppendToOrganList(organColors, "Coccyx", 255, 255, 153);
  AppendToOrganList(organColors, "Colon", 161, 107, 54);
  AppendToOrganList(organColors, "Cyst", 150, 189, 228);
  AppendToOrganList(organColors, "Elbow", 255, 255, 153);
  AppendToOrganList(organColors, "Eye", 18, 7, 161);
  AppendToOrganList(organColors, "Fallopian tube", 161, 19, 39);
  AppendToOrganList(organColors, "Fat", 237, 255, 41);
  AppendToOrganList(organColors, "Gall Bladder", 86, 127, 24);
  AppendToOrganList(organColors, "Hand", 255, 222, 199);
  AppendToOrganList(organColors, "Heart", 153, 0, 0);
  AppendToOrganList(organColors, "Hip", 255, 255, 153);
  AppendToOrganList(organColors, "Kidney", 250, 89, 0);
  AppendToOrganList(organColors, "Knee", 255, 255, 153);
  AppendToOrganList(organColors, "Larynx", 102, 0, 0);
  AppendToOrganList(organColors, "Liver", 194, 142, 0);
  AppendToOrganList(organColors, "Lung", 107, 220, 255);
  AppendToOrganList(organColors, "Lymph node", 10, 250, 56);
  AppendToOrganList(organColors, "Muscle", 102, 0, 0);
  AppendToOrganList(organColors, "Nerve", 255, 234, 79);
  AppendToOrganList(organColors, "Nose", 255, 222, 199);
  AppendToOrganList(organColors, "Oesophagus", 102, 0, 0);
  AppendToOrganList(organColors, "Ovaries", 234, 0, 117);
  AppendToOrganList(organColors, "Pancreas", 249, 171, 61);
  AppendToOrganList(organColors, "Pelvis", 255, 255, 153);
  AppendToOrganList(organColors, "Penis", 255, 222, 199);
  AppendToOrganList(organColors, "Pharynx", 102, 0, 0);
  AppendToOrganList(organColors, "Prostate", 209, 163, 117);
  AppendToOrganList(organColors, "Rectum", 161, 107, 54);
  AppendToOrganList(organColors, "Sacrum", 255, 255, 153);
  AppendToOrganList(organColors, "Seminal vesicle", 199, 232, 255);
  AppendToOrganList(organColors, "Shoulder", 255, 255, 153);
  AppendToOrganList(organColors, "Spinal cord", 255, 234, 79);
  AppendToOrganList(organColors, "Spleen", 249, 108, 61);
  AppendToOrganList(organColors, "Stomach", 161, 107, 54);
  AppendToOrganList(organColors, "Teeth", 255, 252, 216);
  AppendToOrganList(organColors, "Testicles", 199, 232, 255);
  AppendToOrganList(organColors, "Thyroid", 255, 179, 184);
  AppendToOrganList(organColors, "Tongue", 102, 0, 0);
  AppendToOrganList(organColors, "Tumor", 147, 112, 17);
  AppendToOrganList(organColors, "Urethra", 197, 204, 0);
  AppendToOrganList(organColors, "Urinary bladder", 197, 204, 0);
  AppendToOrganList(organColors, "Uterus", 161, 19, 39);
  AppendToOrganList(organColors, "Vagina", 161, 19, 39);
  AppendToOrganList(organColors, "Vertebra", 255, 255, 153);
  AppendToOrganList(organColors, "Wrist", 255, 255, 153);

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

