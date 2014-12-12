#pragma once

#include <cassert>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "Leap/GL/Common.h"
#include "Leap/GL/GLHeaders.h" // convenience header for cross-platform GL includes
#include "Leap/GL/Error.h"
#include "Leap/GL/Internal/ShaderUniform.h"
#include "Leap/GL/Internal/UniformUploader.h"
#include "Leap/GL/ResourceBase.h"
#include "Leap/GL/ShaderException.h"

namespace Leap {
namespace GL {

enum class VariableIs { REQUIRED, OPTIONAL_NO_WARN, OPTIONAL_BUT_WARN };

/// @brief This class wraps compiling and binding GLSL shaders, as well as discovering and
/// setting their uniforms and attributes.
/// @details Some of the code was initially taken from Jerry Coffin's answer at
/// http://stackoverflow.com/questions/2795044/easy-framework-for-opengl-shaders-in-c-c
/// Currently only the OpenGL 2.1 standard uniform types are supported (in particular, this
/// is missing unsigned ints, unsigned int vectors, and a bunch of sampler types).
///
/// Upon successful linking, the shader program will be queried for all its active uniforms
/// and attributes, storing the relevant info (name, location, size, type) in a map which is
/// indexed by name.  These maps can be accessed via the ActiveUniformInfoMap and
/// ActiveAttributeInfoMap methods.
///
/// The only exceptions that this class explicitly throws derive from Leap::GL::ShaderException.
class Shader : public ResourceBase<Shader> {
public:

  // Stores information about a named variable in a shader program.
  class VarInfo {
  public:

    VarInfo (const std::string &name, GLint location, GLint size, GLenum type);

    const std::string &Name () const { return m_name; }
    GLint Location () const { return m_location; }
    GLint Size () const { return m_size; }
    // The Type defines what uniform modifier function can be used with each variable.
    // Note that sampler types must be set using integer values indicating which texture
    // unit is bound to it.  See http://www.opengl.org/wiki/Sampler_(GLSL)#Binding_textures_to_samplers
    GLenum Type () const { return m_type; }

  private:

    std::string m_name;
    GLint m_location;
    GLint m_size;
    GLenum m_type;
  };

  typedef std::unordered_map<std::string,VarInfo> VarInfoMap;

  // Construct an un-Initialize-d shader.
  Shader ();
  // Construct an Initialize-d shader.
  Shader (const std::string &vertex_shader_source, const std::string &fragment_shader_source);
  // Will call Shutdown.
  ~Shader ();

  using ResourceBase<Shader>::IsInitialized;
  using ResourceBase<Shader>::Initialize;
  using ResourceBase<Shader>::Shutdown;

  // Returns the shader program handle, which is the integer "name" of this shader program in OpenGL.
  // Will throw ShaderException if !IsInitialized().
  GLint ProgramHandle () const {
    if (!IsInitialized()) {
      throw ShaderException("A Shader that !IsInitialized() has no ProgramHandle value.");
    }
    return m_program_handle;
  }
  // This method should be called to bind this shader.
  void Bind () const {
    if (!IsInitialized()) {
      throw ShaderException("Can't Bind a Shader that !IsInitialized().");
    }
    THROW_UPON_GL_ERROR(glUseProgram(m_program_handle));
  }
  // This method should be called when no shader program should be used.
  static void Unbind () {
    THROW_UPON_GL_ERROR(glUseProgram(0));
  }
  // Returns the currently bound shader program (the integer handle generated by OpenGL).
  // This should only generate a GL error if it is called between glBegin and glEnd.
  static GLint CurrentlyBoundProgramHandle () {
    GLint current_program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current_program); // We know that GL_CURRENT_PROGRAM is a valid param.
    return current_program;
  }

  // Returns a map, indexed by name, containing all the active uniforms in this shader program.
  // This shader does not need to be bound for this call to succeed.
  const VarInfoMap &ActiveUniformInfoMap () const {
    if (!IsInitialized()) {
      throw ShaderException("A Shader that !IsInitialized() has no ActiveUniformInfoMap value.");
    }
    return m_active_uniform_info_map;
  }
  // Returns a map, indexed by name, containing all the active attributes in this shader program.
  // This shader does not need to be bound for this call to succeed.
  const VarInfoMap &ActiveAttributeInfoMap () const {
    if (!IsInitialized()) {
      throw ShaderException("A Shader that !IsInitialized() has no ActiveAttributeInfoMap value.");
    }
    return m_active_attribute_info_map;
  }

  // Returns the location of the requested uniform (its handle into the GL apparatus) or -1 if not found.
  // The -1 return value is what is used by the glUniform* functions as a sentinel value for "this
  // uniform is not found, so do nothing silently".
  // This shader does not need to be bound for this call to succeed.
  GLint LocationOfUniform (const std::string &name) const {
    if (!IsInitialized()) {
      throw ShaderException("Can't call LocationOfUniform on a Shader that !IsInitialized().");
    }
    return glGetUniformLocation(m_program_handle, name.c_str());
  }
  // Returns the location of the requested attribute (its handle into the GL apparatus) or -1 if not found.
  // The -1 return value is what is used by the glUniform* functions as a sentinel value for "this
  // uniform is not found, so do nothing silently".
  // This shader does not need to be bound for this call to succeed.
  GLint LocationOfAttribute (const std::string &name) const {
    if (!IsInitialized()) {
      throw ShaderException("Can't call LocationOfUniform on a Shader that !IsInitialized().");
    }
    return glGetAttribLocation(m_program_handle, name.c_str());
  }

  // These SetUniform* methods require this shader to currently be bound.  They are named
  // with type annotators to avoid confusion in situations where types are implicitly coerced.
  // The uniform has a fixed type in the shader, so the call to SetUniform* should reflect that.

  template <GLenum GL_TYPE_, typename... Types_>
  static void UploadUniform (GLint location, Types_... args) {
    Internal::UniformUploader<GL_TYPE_>::Upload(location, args...);
  }
  template <GLenum GL_TYPE_, typename... Types_>
  void UploadUniform (const std::string &name, Types_... args) const {
    if (!IsInitialized()) {
      throw ShaderException("Can't call [the name version of] UploadUniform on a Shader that !IsInitialized().");
    }
    Internal::UniformUploader<GL_TYPE_>::Upload(LocationOfUniform(name), args...);
  }
  template <GLenum GL_TYPE_, size_t ARRAY_LENGTH_, typename... Types_>
  static void UploadUniformArray (GLint location, Types_... args) {
    Internal::UniformUploader<GL_TYPE_>::template UploadArray<ARRAY_LENGTH_>(location, args...);
  }
  template <GLenum GL_TYPE_, size_t ARRAY_LENGTH_, typename... Types_>
  void UploadUniformArray (const std::string &name, Types_... args) const {
    if (!IsInitialized()) {
      throw ShaderException("Can't call [the name version of] UploadUniformArray on a Shader that !IsInitialized().");
    }
    Internal::UniformUploader<GL_TYPE_>::template UploadArray<ARRAY_LENGTH_>(LocationOfUniform(name), args...);
  }

  // Returns (enum_name_string, type_name_string) for the given shader variable type.  Throws an
  // error if that type is not a shader variable type.
  static const std::string &VariableTypeString (GLenum type);
  
  static const std::unordered_map<GLenum,std::string> OPENGL_2_1_UNIFORM_TYPE_MAP;
  static const std::unordered_map<GLenum,std::string> OPENGL_3_3_UNIFORM_TYPE_MAP;

private:

  // Compiles the specified type of shader program, using the given source.  If an error
  // in encountered, a std::logic_error is thrown.
  static GLuint Compile (GLuint type, const std::string &source);

  friend class ResourceBase<Shader>;

  bool IsInitialized_Implementation () const { return m_program_handle != 0; }
  // Construct a shader with given vertex and fragment programs.
  void Initialize_Implementation (const std::string &vertex_shader_source, const std::string &fragment_shader_source);
  // Frees the allocated resources.
  void Shutdown_Implementation ();

  GLuint m_vertex_shader;   ///< Handle to the vertex shader in the GL apparatus.
  GLuint m_fragment_shader; ///< Handle to the fragment shader in the GL apparatus.
  GLuint m_program_handle;  ///< Handle to the shader program in the GL apparatus.

  VarInfoMap m_active_uniform_info_map;
  VarInfoMap m_active_attribute_info_map;
};

} // end of namespace GL
} // end of namespace Leap
