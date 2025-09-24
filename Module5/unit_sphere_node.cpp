#include "unit_sphere_node.hpp"
#include "geometry/types.hpp"
#include "geometry/point3.hpp"
#include "geometry/vector3.hpp"

#include <vector>
#include <cmath>

namespace cg
{
UnitSphere::UnitSphere(int32_t position_loc, int32_t normal_loc) : GeometryNode()
{
    std::vector<VertexAndNormal> vertex_list;
    std::vector<GLuint> indices;
    
    // Generate sphere geometry
    generateSphereGeometry(vertex_list, indices);
    
    index_count_ = static_cast<GLsizei>(indices.size());

    // Create and bind VBO for vertex data
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 vertex_list.size() * sizeof(VertexAndNormal),
                 &vertex_list[0],
                 GL_STATIC_DRAW);

    // Create and bind EBO for index data learned about loosely from here: https://stackoverflow.com/questions/22444450/drawing-circle-with-opengl
    glGenBuffers(1, &ebo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(GLuint),
                 &indices[0],
                 GL_STATIC_DRAW);

    // Create and configure VAO
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    // Bind VBO and set vertex attribute pointers
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    //link our position to VBO
    glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 
                         sizeof(VertexAndNormal), (void*)0);
    glEnableVertexAttribArray(position_loc);
    
    //link normal to VBO
    glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, 
                         sizeof(VertexAndNormal), (void*)(sizeof(Point3)));
    glEnableVertexAttribArray(normal_loc);

    // Bind EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

    // Unbind VAO to ensure changes are local
    glBindVertexArray(0);
}

UnitSphere::~UnitSphere()
{
    // Clean up OpenGL resources
    if (vao_ != 0)
        glDeleteVertexArrays(1, &vao_);
    if (vbo_ != 0) 
        glDeleteBuffers(1, &vbo_);
    if (ebo_ != 0) 
        glDeleteBuffers(1, &ebo_);
}

void UnitSphere::draw(SceneState &scene_state)
{
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, index_count_, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
void UnitSphere::generateSphereGeometry(std::vector<VertexAndNormal> &vertex_list, 
                                       std::vector<GLuint> &indices)
{
    vertex_list.clear();
    indices.clear();
    
    const float PI = 3.14159265359f;
    const float DEG_TO_RAD = PI / 180.0f;
    
    // Generate vertices
    // Latitude goes from -90 to +90 degrees
    // Longitude goes from 0 to 360 degrees
    for (float lat = -90.0f; lat <= 90.0f; lat += 10.0f) {
        for (float lon = 0.0f; lon < 360.0f; lon += 10.0f) {
            float lat_rad = lat * DEG_TO_RAD;
            float lon_rad = lon * DEG_TO_RAD;
            
            // Calculate vertex position using spherical coordinates
            // x = r * cos(lat) * cos(lon)
            // y = r * sin(lat) 
            // z = r * cos(lat) * sin(lon)
            // For unit sphere, r = 1
            float x = cosf(lat_rad) * cosf(lon_rad);
            float y = sinf(lat_rad);
            float z = cosf(lat_rad) * sinf(lon_rad);
            
            VertexAndNormal vertex;
            vertex.vertex.set(x, y, z);
            
            // For a unit sphere, the normal at any point is just 
            // the normalized position vector (which is already normalized)
            vertex.normal.set(x, y, z);
            
            vertex_list.push_back(vertex);
        }
    }
    
    // Generate indices for triangles
    // We have (lat_divisions + 1) rows and lon_divisions columns
    const int lat_divisions = 18;  // -90 to +90 in 10 degree steps = 18 divisions
    const int lon_divisions = 36;  // 0 to 360 in 10 degree steps = 36 divisions
    
    for (int lat_idx = 0; lat_idx < lat_divisions; ++lat_idx) {
        for (int lon_idx = 0; lon_idx < lon_divisions; ++lon_idx) {
            // Calculate vertex indices for current quad
            int current = lat_idx * (lon_divisions) + lon_idx;
            int next_lon = lat_idx * (lon_divisions) + ((lon_idx + 1) % lon_divisions);
            int next_lat = (lat_idx + 1) * (lon_divisions) + lon_idx;
            int next_both = (lat_idx + 1) * (lon_divisions) + ((lon_idx + 1) % lon_divisions);
            
            // Skip triangles at the poles to avoid degenerate triangles
            if (lat_idx < lat_divisions) {
                // First triangle of the quad
                indices.push_back(current);
                indices.push_back(next_lat);
                indices.push_back(next_lon);
                
                // Second triangle of the quad
                indices.push_back(next_lon);
                indices.push_back(next_lat);
                indices.push_back(next_both);
            }
        }
    }
}
}
