#pragma once

#include "Color.h"
#include "EigenTypes.h"
#include "GLBuffer.h"
#include "GLVertexBuffer.h"
#include "RenderState.h"

#include <map>
#include <vector>

class GLShader;

// Convenience class for creating and managing GPU geometry.
// Simple shapes can be created/uploaded automatically using the available factory functions.
// More complex shapes can created by manually inputting triangles then calling UploadDataToBuffers()
class PrimitiveGeometry {
public:

  enum class ClearOption { KEEP_INTERMEDIATE_DATA, CLEAR_INTERMEDIATE_DATA };
  
  PrimitiveGeometry();

  typedef GLVertexBuffer<GLVertexAttribute<GL_FLOAT_VEC3>, // Position
                         GLVertexAttribute<GL_FLOAT_VEC3>, // Normal vector
                         GLVertexAttribute<GL_FLOAT_VEC2>, // 2D texture coordinate
                         GLVertexAttribute<GL_FLOAT_VEC4>  // RGBA color
                        > VertexBuffer;
  typedef VertexBuffer::Attributes VertexAttributes;

  // Gives direct access to the list of vertices.
  std::vector<VertexBuffer::Attributes> &Vertices () { return m_Vertices; }
  
  // functions for manipulating GPU-side buffers
  void CleanUpBuffers();
  void UploadDataToBuffers(ClearOption clear_option = ClearOption::CLEAR_INTERMEDIATE_DATA);

  // after geometry is uploaded, draws the geometry using the current render state
  void Draw(const GLShader &bound_shader, GLenum drawMode) const;

  // Factory functions for generating some simple shapes.  These functions assume that the draw mode (see Draw) is GL_TRIANGLES.
  static PrimitiveGeometry CreateUnitSphere(int resolution);
  static PrimitiveGeometry CreateUnitCylinder(int radialResolution, int verticalResolution);
  static PrimitiveGeometry CreateUnitSquare();
  static PrimitiveGeometry CreateUnitDisk(int resolution);
  static PrimitiveGeometry CreateUnitBox();

  // Add a quad with the specified points, winding counterclockwise, with the given normal vector, adding
  // texture coordinates if requested.  This function assumes that the draw mode (see Draw) is GL_TRIANGLES.
  void PushTri (const VertexAttributes &p0, const VertexAttributes &p1, const VertexAttributes &p2);
  // Add a quad with the specified points, winding counterclockwise, with the given normal vector, adding
  // texture coordinates if requested.  This function assumes that the draw mode (see Draw) is GL_TRIANGLES.
  void PushTri (const Vector3f &p0, const Vector3f &p1, const Vector3f &p2);
  // Add a quad with the specified points, winding counterclockwise, with the given normal vector, adding
  // texture coordinates if requested.  This function assumes that the draw mode (see Draw) is GL_TRIANGLES.
  void PushQuad (const Vector3f &p0, const Vector3f &p1, const Vector3f &p2, const Vector3f &p3);
  
  static VertexAttributes MakeVertexAttributes (const Vector3f &position, const Vector3f &normal, const Vector2f &tex_coord = Vector2f::Zero(), const Color &color = Color::White()) {
    return VertexAttributes(position, normal, tex_coord, color);
  }

private:

  struct VertexAttributesCompare {
    bool operator () (const VertexAttributes &lhs, const VertexAttributes &rhs) const {
      return memcmp(reinterpret_cast<const void *>(&lhs), reinterpret_cast<const void *>(&rhs), sizeof(VertexAttributes)) < 0;
    }
  };

  typedef std::map<VertexAttributes,GLuint,VertexAttributesCompare> VertexIndexMap;
  
  // This intermediate storage for vertex data as it is being generated, and may be cleared during upload.
  std::vector<VertexAttributes> m_Vertices;
  // This is the vertex buffer object for uploaded vertex attribute data.
  VertexBuffer m_VertexBuffer;
  // This is the number of indices used to pass to glDrawElements when drawing the VBO.
  int m_NumIndices;
  // This is the buffer containing the index elements.
  GLBuffer m_IndexBuffer;
};
