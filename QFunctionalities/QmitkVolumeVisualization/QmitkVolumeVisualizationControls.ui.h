/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/
 int number = 0;
 int cs_number = 0;



void QmitkVolumeVisualizationControls::EnableRenderingCB_toggled( bool state)
{
  emit EnableRenderingToggled(state);
  if(state)
  {
    this->SetDefaultShadingValues();
  }
}

void QmitkVolumeVisualizationControls::SendChoice()
{
    emit Choice(number); 
}
 void SetAuswahl(int i){
     number = i;
 }

  void SetCS(int i){
     cs_number = i;
 }
 
void QmitkVolumeVisualizationControls::ComboBox( const QString &auswahl )
{  

 if(auswahl == "bone + soft-tissue"){
 SetAuswahl(1);
     }
 else if(auswahl == "soft-tissue"){
 SetAuswahl(2);
     }
 else if(auswahl == "bones"){
 SetAuswahl(3);
     }
 else{
 SetAuswahl(0);
 }

}

void QmitkVolumeVisualizationControls::resetTF()
{
    emit Choice(999); 

}


void QmitkVolumeVisualizationControls::ColorStyle( const QString & cstyle )
{
 if(cstyle == "bone - natural"){
 SetCS(1);
 }
 else if(cstyle == "bone - high contrast"){
 SetCS(2);
 }
 else if(cstyle == "soft tissue - natural"){
 SetCS(3);
 }
  else if(cstyle == "muscles - bones"){
 SetCS(4);
 }
 

}


void QmitkVolumeVisualizationControls::SendCS()
{
    emit StyleChoice(cs_number);
}


void QmitkVolumeVisualizationControls::SetPreset1()
{
    emit PresetTF(101);
}

void QmitkVolumeVisualizationControls::SetPreset2()
{
     emit PresetTF(102);
}


void QmitkVolumeVisualizationControls::SetPreset3()
{
    emit PresetTF(103);
}


void QmitkVolumeVisualizationControls::SetPreset4()
{
 emit PresetTF(104);
}


void QmitkVolumeVisualizationControls::SetPreset5()
{
 emit PresetTF(105);
}


void QmitkVolumeVisualizationControls::SetPreset6()
{
 emit PresetTF(106);
}


void QmitkVolumeVisualizationControls::SetPreset7()
{
emit PresetTF(107);
}


void QmitkVolumeVisualizationControls::SetPreset8()
{
emit PresetTF(108);
}


void QmitkVolumeVisualizationControls::SetPreset9()
{
emit PresetTF(109);
}


void QmitkVolumeVisualizationControls::SetPreset10()
{
emit PresetTF(110);
}

void QmitkVolumeVisualizationControls::ImmediateUpdate(bool state)
{
emit ImmUpdate(state);
}

//Preferences

void QmitkVolumeVisualizationControls::EnableShadingLow( bool state)
{
  emit EnableShadingToggled(state,0);
}

void QmitkVolumeVisualizationControls::EnableShadingMed( bool state)
{
  emit EnableShadingToggled(state,1);
}

void QmitkVolumeVisualizationControls::EnableShadingHigh( bool state)
{
  emit EnableShadingToggled(state,2);
}

void QmitkVolumeVisualizationControls::EnableClippingPlane( bool state )
{
  emit EnableCPToggled(state);
}

void QmitkVolumeVisualizationControls::SetShadingValues()
{
  emit ShadingValues(m_AmbientEdit->text().toFloat(), m_DiffuseEdit->text().toFloat(), m_SpecularEdit->text().toFloat(), m_SpecPowerEdit->text().toFloat());
}


void QmitkVolumeVisualizationControls::SetDefaultShadingValues()
{
  m_AmbientEdit->setText(QString::number(0.1));
  m_DiffuseEdit->setText(QString::number(0.7));
  m_SpecularEdit->setText(QString::number(0.2));
  m_SpecPowerEdit->setText(QString::number(10));
}
