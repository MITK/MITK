/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIOExtActivator_h
#define mitkIOExtActivator_h

#include <usModuleActivator.h>

#include <memory>

namespace mitk
{
  struct IFileReader;
  struct IFileWriter;
  class IMapperProvider;

  /**
   * \brief Module activator for the IOExt module.
   *
   * Registers various file reader and writer services for formats such as
   * OBJ, and PLY when the module is loaded, as well as the mapper provider
   * for 3D volume rendering of images.
   */
  class IOExtActivator : public us::ModuleActivator
  {
  public:
    /** \brief Register extended IO reader and writer services. */
    void Load(us::ModuleContext *context) override;

    /** \brief Unregister all extended IO services. */
    void Unload(us::ModuleContext *context) override;

  private:
    std::unique_ptr<IFileReader> m_ObjReader;
    std::unique_ptr<IFileWriter> m_ObjWriter;

    std::unique_ptr<IFileReader> m_PlyReader;

    std::unique_ptr<IMapperProvider> m_VolumeMapperProvider;
  };
}

#endif
