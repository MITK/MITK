#ifndef MITKVOLUMEDATAVTKMAPPER3D_H_HEADER_INCLUDED
#define MITKVOLUMEDATAVTKMAPPER3D_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "BaseVtkMapper3D.h"
#include "mitkVolumeData.h"
#include "BaseRenderer.h"

class vtkActor;

namespace mitk {


//##Documentation
//## @brief Vtk-based mapper for VolumeData
//## @ingroup Mapper
class VolumeDataVtkMapper3D : public BaseVtkMapper3D
{
public:

  mitkClassMacro(VolumeDataVtkMapper3D, BaseVtkMapper3D);

  itkNewMacro(Self);


  virtual const mitk::VolumeData* GetInput();


  virtual void GenerateData();


  //##Documentation
  //## @brief Called by BaseRenderer when an update is required
  virtual void Update(mitk::BaseRenderer* renderer);

  virtual void Update();

  virtual void ApplyProperties(vtkActor* actor, mitk::BaseRenderer* renderer);

protected:

  virtual void GenerateOutputInformation();

  VolumeDataVtkMapper3D();


  virtual ~VolumeDataVtkMapper3D();


  vtkActor* m_Actor;


  vtkVolumeMapper* m_VtkVolumeMapper;
};

} // namespace mitk

#endif /* MITKVOLUMEDATAVTKMAPPER3D_H_HEADER_INCLUDED */
