#pragma once

// This file contains functions useful in checking and handling OpenGL errors.

#include "Leap/GL/Exception.h"
#include "Leap/GL/GLHeaders.h"
#include <iostream>
#include <string>

// TODO: Make a function that replaces gluErrorString
#define USE_gluErrorString 0

#if !USE_gluErrorString
  #include <sstream>
#endif

namespace Leap {
namespace GL {

namespace Internal {

inline std::string ErrorMessage(GLenum error_code, const std::string &during) {
  std::string error_message;
#if USE_gluErrorString
  error_message += "GL error \"" + std::string(reinterpret_cast<const char *>(gluErrorString(error_code))) + "\" " + during;
#else
  std::ostringstream out;
  out << std::hex << error_code << " - " << glewGetErrorString(error_code);
  error_message += "GL error 0x" + out.str() + ' ' + during;
#endif
  return error_message;
}

} // end of namespace Internal

inline void ThrowUponGLError(const std::string& during) {
  GLenum error_code = glGetError();
  if (error_code != GL_NO_ERROR) {
    throw Leap::GL::Exception(Internal::ErrorMessage(error_code, during));
  }
}

inline void WarnUponGLError(const std::string& during, std::ostream *out = &std::cerr) {
  GLenum error_code = glGetError();
  if (error_code != GL_NO_ERROR && out) {
    *out << Internal::ErrorMessage(error_code, during) << std::endl;;
  }
}

inline void ClearGLError() {
  glGetError(); // This clears the error flag.
}

#if !defined(NDEBUG)
  // Convenience macro for checking the GL error flag before and after the call.  An exception
  // will be thrown upon error either before or after with an indicative message.  This macro
  // should NOT be used as the body of an if statement without { } brackets!
  #define THROW_UPON_GL_ERROR(single_gl_call_statement) \
    Leap::GL::ThrowUponGLError("before " #single_gl_call_statement); \
    single_gl_call_statement; \
    Leap::GL::ThrowUponGLError("during " #single_gl_call_statement);
#else
  // In release mode, don't mess with performance! -JH
  // Note: ThrowUponGLError calls glGetError, which is quite expensive.
  #define THROW_UPON_GL_ERROR(single_gl_call_statement) \
  single_gl_call_statement;
#endif

#if !defined(NDEBUG)
  // Convenience macro for checking the GL error state and issuing a non-fatal warning if there is
  // an error either before or after the call.  Warnings are printed to the ostream which is the 
  // default parameter value of WarnUponGLError.  This macro should NOT be used as the body of an
  // if statement without { } brackets!
  #define WARN_UPON_GL_ERROR(single_gl_call_statement) \
    Leap::GL::WarnUponGLError("before " #single_gl_call_statement); \
    single_gl_call_statement; \
    Leap::GL::WarnUponGLError("during " #single_gl_call_statement);
#else
  // In release mode, this simply silently clears the error flag after the call.
  // This macro should NOT be used as the body of an if statement without { } brackets!
  #define WARN_UPON_GL_ERROR(single_gl_call_statement, ...) \
    single_gl_call_statement; \
    Leap::GL::ClearGLError();
#endif

} // end of namespace GL
} // end of namespace Leap
