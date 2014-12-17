#pragma once

#include "Leap/GL/GLHeaders.h"
#include "Leap/GL/BufferObject.h"
#include "Leap/GL/Internal/Meta.h"
#include "Leap/GL/ResourceBase.h"
#include "Leap/GL/VertexAttribute.h"
#include "Leap/GL/VertexBufferObjectException.h"

#include <cassert>
#include <tuple>
#include <type_traits>
#include <vector>

namespace Leap {
namespace GL {

/// @brief Encapsulates the concept of an OpenGL vertex buffer object.
/// @details A vertex buffer object is an array of vertex attributes that are uploaded 
/// to the GPU for use in a vertex shader program.  The vertex attributes correspond to
/// "attribute" variables in the vertex shader.  The vertex shader program is called on each
/// vertex, and each vertex must have each of the required attributes defined for it.  Thus
/// the set of attributes must be well-defined.  This is done via the variadic AttributeTypes
/// template parameter(s).
///
/// There is a vector of "intermediate attributes" which is a mutable buffer for creating/modifying
/// vertex attributes before they are [re]uploaded to the GPU.  Once the intermediate attributes
/// vector has been populated, the UploadIntermediateAttributes should be called, which will
/// create the necessary GL resource and upload the data to the GPU.  Unless the intermediate
/// attributes are going to be modified and re-uploaded, it is recommended to clear the
/// intermediate attributes after upload.
///
/// The only exceptions that this class explicitly throws derive from
/// Leap::GL::VertexBufferObjectException.
///
/// For more info, see https://www.opengl.org/wiki/Vertex_Specification#Vertex_Buffer_Object
///
/// TODO: Remove the intermediate storage concern.  This will simplify the resource interface.
template <typename... AttributeTypes>
class VertexBufferObject : public ResourceBase<VertexBufferObject<AttributeTypes...>> {
public:

  /// @brief Convenience typedef for the type of a single vertex attribute.
  typedef std::tuple<AttributeTypes...> Attributes;
  /// @brief Number of attributes specified in this VertexBufferObject.
  static const size_t ATTRIBUTE_COUNT = std::tuple_size<Attributes>::value;
  /// @brief Data type which holds the locations for the respective attributes.
  typedef typename Internal::UniformTuple<ATTRIBUTE_COUNT,GLint>::T AttributeLocations;

  /// @brief Construct an un-Initialize-d VertexBufferObject which has not acquired any GL (or other) resources.
  /// @details It will be necessary to call Initialize on this object to use it.
  VertexBufferObject ()
    : m_usage_pattern(GL_INVALID_ENUM)
  { }
  /// @brief Convenience constructor that will call Initialize with the given arguments.
  VertexBufferObject (GLenum usage_pattern)
    : m_usage_pattern(GL_INVALID_ENUM)
  {
    Initialize(usage_pattern);
  }
  /// @brief Destructor will call Shutdown.
  ~VertexBufferObject () {
    Shutdown();
  }

  using ResourceBase<VertexBufferObject<AttributeTypes...>>::IsInitialized;
  using ResourceBase<VertexBufferObject<AttributeTypes...>>::Initialize;
  using ResourceBase<VertexBufferObject<AttributeTypes...>>::Shutdown;

  /// @brief Returns the usage pattern used in upload operations.
  GLenum UsagePattern () const {
    if (!IsInitialized()) {
      throw VertexBufferObjectException("A VertexBufferObject that !IsInitialized() has no UsagePattern value.");
    }
    return m_usage_pattern;
  }
  /// @brief Returns a const reference to the vertex attributes' intermediate buffer that is used
  /// to create/modify attributes before uploading to the GPU.
  const std::vector<Attributes> &IntermediateAttributes () const {
    if (!IsInitialized()) {
      throw VertexBufferObjectException("A VertexBufferObject that !IsInitialized() has no IntermediateAttributes value.");
    }
    return m_intermediate_attributes;
  }
  /// @brief Returns a reference to the vertex attributes' intermediate buffer that is used
  /// to create/modify attributes before uploading to the GPU.
  std::vector<Attributes> &IntermediateAttributes () {
    if (!IsInitialized()) {
      throw VertexBufferObjectException("A VertexBufferObject that !IsInitialized() has no IntermediateAttributes value.");
    }
    return m_intermediate_attributes;
  }

  /// @brief This releases all resources (pre-load attribute buffer and GL buffer
  /// for vertex attributes, if it has been created already).
  void ClearEverything () {
    ClearIntermediateAttributes();
    ClearGLResources();
  }
  /// @brief This clears the intermediate attribute buffer, but preserves everything else.
  void ClearIntermediateAttributes () {
    m_intermediate_attributes.clear();
  }
  /// @brief This clears the GL buffer object, but preserves everything else.
  void ClearGLResources () const {
    m_gl_buffer_object.Shutdown();
  }
  /// @brief Allocates (if necessary) and populates a GL buffer object with the intermediate attribute buffer data.
  /// @details It is recommended to clear the intermediate attributes after calling this method, unless said
  /// attribute data is going to be modified and uploaded again.  This method should only be called after the
  /// intermediate attributes have changed and the changes need to be propagated to the GPU.
  void UploadIntermediateAttributes () const {
    if (!IsInitialized()) {
      throw VertexBufferObjectException("Can't call VertexBufferObject::UploadIntermediateAttributes on a VertexBufferObject that is !IsInitialized().");
    }

    GLsizeiptr intermediate_attributes_size(m_intermediate_attributes.size()*sizeof(Attributes));
    const void *intermediate_attributes_data(m_intermediate_attributes.data());
    // If the buffer is already created and is the same size as the intermediate attributes,
    // then map it and copy the data in.
    if (m_gl_buffer_object.IsInitialized() && m_gl_buffer_object.Size() == intermediate_attributes_size) {
      void *ptr = m_gl_buffer_object.MapBuffer(GL_WRITE_ONLY);
      memcpy(ptr, intermediate_attributes_data, intermediate_attributes_size);
      m_gl_buffer_object.UnmapBuffer();
    } else { // Otherwise ensure the buffer is created, 
      if (!m_gl_buffer_object.IsInitialized()) {
        m_gl_buffer_object.Initialize(GL_ARRAY_BUFFER);
      }
      m_gl_buffer_object.Bind();
      // This will delete and reallocate if it's already allocated.
      m_gl_buffer_object.BufferData(intermediate_attributes_data, intermediate_attributes_size, m_usage_pattern);
      m_gl_buffer_object.Unbind();
    }
    assert(IsUploaded());
  }

  /// @brief Returns true if UploadIntermediateAttributes has been called, i.e. if there are associated GL resources.
  bool IsUploaded () const {
    if (!IsInitialized()) {
      return false;
    }
    return m_gl_buffer_object.IsInitialized();
  }
  /// @brief This method calls glEnableVertexAttribArray and glVertexAttribPointer on each
  /// of the vertex attributes given valid locations (i.e. not equal to -1).
  /// @details The tuple argument attribute_locations must correspond exactly to Attributes
  /// (which is a tuple of VertexAttribute types defined by this VertexBufferObject).
  void Enable (const AttributeLocations &attribute_locations) const {
    if (!IsInitialized()) {
      throw VertexBufferObjectException("Can't call VertexBufferObject::Enable on a VertexBufferObject that is !IsInitialized().");
    }
    if (!IsUploaded()) {
      throw VertexBufferObjectException("can't Enable a VertexBufferObject that hasn't had UploadIntermediateAttributes called on it");
    }
    m_gl_buffer_object.Bind();
    // Begin iterated binding of vertex attributes starting at the 0th one.
    EnableAndIterate<0>(attribute_locations, sizeof(Attributes));
    m_gl_buffer_object.Unbind();
  }
  /// @brief This method calls glDisableVertexAttribArray on each of the vertex attributes
  /// given valid locations (i.e. not equal to -1).
  /// @details This method is analogous to the Enable method.
  static void Disable (const AttributeLocations &attribute_locations) {
    // Begin iterated unbinding of vertex attributes starting at the 0th one.
    DisableAndIterate<0>(attribute_locations);
  }

private:

  // This is one iteration of the Enable method.  It calls the next iteration.
  template <size_t INDEX>
  static typename std::enable_if<(INDEX<ATTRIBUTE_COUNT),void>::type
    EnableAndIterate (const AttributeLocations &locations, size_t stride)
  {
    // Get the INDEXth attribute type.
    typedef typename std::tuple_element<INDEX,Attributes>::type AttributeType;
    // Get the INDEXth location value.
    GLint location = std::get<INDEX>(locations);
    // Compute the offset of the current attribute into the Attributes tuple type.  It is NOT
    // necessarily layed out in memory in the same order as the variadic template parameters!
    static const Attributes A; // This is not actually used for anything at runtime, just for determining the offset.
    static const size_t OFFSET_OF_INDEXth_ATTRIBUTE = static_cast<size_t>(reinterpret_cast<const uint8_t *>(&std::get<INDEX>(A)) - reinterpret_cast<const uint8_t *>(&A));
    // Call the Enable method of the INDEXth attribute type with the INDEXth location value, etc.
    AttributeType::Enable(location, static_cast<GLsizei>(stride), static_cast<GLsizei>(OFFSET_OF_INDEXth_ATTRIBUTE));
    // Increment INDEX and call this method again (this is a meta-program for loop).
    EnableAndIterate<INDEX+1>(locations, stride);
  }
  // This is the end of the iteration in the Enable method.
  template <size_t INDEX>
  static typename std::enable_if<(INDEX>=ATTRIBUTE_COUNT),void>::type
    EnableAndIterate (const AttributeLocations &locations, size_t stride)
  {
    // Iteration complete -- do nothing.
  }

  // This is one iteration of the Disable method.  It calls the next iteration.
  template <size_t INDEX>
  static typename std::enable_if<(INDEX<ATTRIBUTE_COUNT),void>::type
    DisableAndIterate (const AttributeLocations &locations)
  {
    // Get the INDEXth attribute type.
    typedef typename std::tuple_element<INDEX,Attributes>::type AttributeType;
    // Get the INDEXth location value.
    GLint location = std::get<INDEX>(locations);
    // Call the Disable method of the INDEXth attribute type with the INDEXth location value.
    AttributeType::Disable(location);
    // Increment INDEX and call this method again (this is a meta-program for loop).
    DisableAndIterate<INDEX+1>(locations);
  }
  // This is the end of the iteration in the Disable method.
  template <size_t INDEX>
  static typename std::enable_if<(INDEX>=ATTRIBUTE_COUNT),void>::type
    DisableAndIterate (const AttributeLocations &locations)
  {
    // Iteration complete -- do nothing.
  }

  friend class ResourceBase<VertexBufferObject<AttributeTypes...>>;

  bool IsInitialized_Implementation () const { return m_usage_pattern != GL_INVALID_ENUM; }
  // The usage_pattern parameter specifies the expected usage pattern of the data store.
  // It must be one of: GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY, GL_STATIC_DRAW,
  // GL_STATIC_READ, GL_STATIC_COPY, GL_DYNAMIC_DRAW, GL_DYNAMIC_READ, GL_DYNAMIC_COPY.
  // See glBufferData for more on this.
  void Initialize_Implementation (GLenum usage_pattern) {
    switch (usage_pattern) {
      case GL_STREAM_DRAW:
      case GL_STREAM_READ:
      case GL_STREAM_COPY:
      case GL_STATIC_DRAW:
      case GL_STATIC_READ:
      case GL_STATIC_COPY:
      case GL_DYNAMIC_DRAW:
      case GL_DYNAMIC_READ:
      case GL_DYNAMIC_COPY:
        m_usage_pattern = usage_pattern;
        break; // Ok
      default:
        throw VertexBufferObjectException("usage must be one of GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY, GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY, GL_DYNAMIC_DRAW, GL_DYNAMIC_READ, GL_DYNAMIC_COPY.");
    }    
  }
  // Frees the allocated resources if IsInitialized(), otherwise does nothing (i.e. this method is
  // safe to call multiple times, and has no effect after the resources are freed).
  void Shutdown_Implementation () {
    ClearEverything();
    assert(!IsUploaded());
    m_usage_pattern = GL_INVALID_ENUM;
  }

  GLenum m_usage_pattern;
  // TODO: it might be a good thing to remove the storage of these intermediate attributes from the concern
  // of this class -- doing so would greatly simplify this class' interface.
  std::vector<Attributes> m_intermediate_attributes;
  mutable BufferObject m_gl_buffer_object;
};

} // end of namespace GL
} // end of namespace Leap