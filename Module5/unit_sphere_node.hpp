#ifndef __MODULE5_UNIT_SPHERE_GEOMETRY_NODE_HPP__
#define __MODULE5_UNIT_SPHERE_GEOMETRY_NODE_HPP__

#include "scene/geometry_node.hpp"

namespace cg
{

/**
 * Unit sphere geometry node. Creates a sphere with radius = 1.
 */
class UnitSphere : public GeometryNode
{
  public:
    /**
     * Constructor. Creates vertex list for a unit sphere using 
     * 10 degree increments for both latitude and longitude.
     * @param position_loc  Shader attribute location for vertex positions
     * @param normal_loc    Shader attribute location for vertex normals
     */
    UnitSphere(int32_t position_loc, int32_t normal_loc);

    /**
     * Destructor. Cleans up OpenGL resources.
     */
    ~UnitSphere();

    /**
     * Draw the unit sphere. Geometry nodes are leaf nodes.
     * @param scene_state  Current scene state
     */
    void draw(SceneState &scene_state) override;

  protected:
    GLuint  vao_;          // Vertex Array Object
    GLuint  vbo_;          // Vertex Buffer Object
    GLuint  ebo_;          // Element Buffer Object (for indices)
    GLsizei index_count_;  // Number of indices to draw
    
    /**
     * Generate sphere vertices and normals using spherical coordinates
     * @param vertex_list  Vector to store generated vertices and normals
     * @param indices      Vector to store triangle indices
     */
    void generateSphereGeometry(std::vector<VertexAndNormal> &vertex_list, 
                               std::vector<GLuint> &indices);
};

} // namespace cg

#endif
