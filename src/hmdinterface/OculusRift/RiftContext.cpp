#include "RiftContext.h"
#include "OVR.h"
#include "OVR_Kernel.h"

#include <iostream>

namespace OculusRift {

Context::Context () :
  m_is_initialized(false)
{ }

Context::~Context () {
  if (m_is_initialized) {
    std::cerr << "Warning: Leap::OculusRift::Context instance was not Shutdown() before destruction.\n";
    Context::Shutdown();
  }
}

void Context::Initialize () {
  if (m_is_initialized) {
    std::cerr << "Warning: Leap::OculusRift::Context instance is already initialized upon calling Initialize().\n";
  } else {
    // The return value of ovr_Initialize is not documented.  Looking at the source
    // of this function in SDK 0.4.1, it can only possibly return ovrBool(1).
    // TODO: detect if/when this fails and throw a Leap::Hmd::IException.
    ovr_Initialize();
    m_is_initialized = true;
  }
}

bool Context::IsInitialized () {
  return m_is_initialized;
}

void Context::Shutdown () {
  if (m_is_initialized) {
    ovr_Shutdown();
    m_is_initialized = false;
  } else {
    std::cerr << "Warning: Leap::OculusRift::Context instance was already shutdown upon calling Shutdown().\n";
  } 
}

} // end of namespace OculusRift
