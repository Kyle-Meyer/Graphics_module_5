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
    
    // Generate sphere geometry (creates duplicated vertices for each triangle)
    generateSphereGeometry(vertex_list);
    
    vertex_count_ = static_cast<GLsizei>(vertex_list.size());

    // Create and bind VBO for vertex data
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 vertex_list.size() * sizeof(VertexAndNormal),
                 &vertex_list[0],
                 GL_STATIC_DRAW);

    // Create and configure VAO
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    // Bind VBO and set vertex attribute pointers
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 
                         sizeof(VertexAndNormal), (void*)0);
    glEnableVertexAttribArray(position_loc);
    
    glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, 
                         sizeof(VertexAndNormal), (void*)(sizeof(Point3)));
    glEnableVertexAttribArray(normal_loc);

    // Unbind VAO to ensure changes are local
    glBindVertexArray(0);
}

UnitSphere::~UnitSphere()
{
    // Clean up OpenGL resources
    if (vao_ != 0) {
        glDeleteVertexArrays(1, &vao_);
    }
    if (vbo_ != 0) {
        glDeleteBuffers(1, &vbo_);
    }
}

void UnitSphere::draw(SceneState &scene_state)
{
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count_);
    glBindVertexArray(0);
}

void UnitSphere::generateSphereGeometry(std::vector<VertexAndNormal> &vertex_list)
{
    vertex_list.clear();
    
    const float PI = 3.14159265359f;
    const float DEG_TO_RAD = PI / 180.0f;
    
    // Helper function to create a vertex
    auto createVertex = [&](float lat, float lon) -> VertexAndNormal {
        float lat_rad = lat * DEG_TO_RAD;
        float lon_rad = lon * DEG_TO_RAD;
        
        float x = cosf(lat_rad) * cosf(lon_rad);
        float y = sinf(lat_rad);
        float z = cosf(lat_rad) * sinf(lon_rad);
        
        VertexAndNormal vertex;
        vertex.vertex.set(x, y, z);
        vertex.normal.set(x, y, z);  // For unit sphere, normal = position
        return vertex;
    };
    
    // Generate triangles directly (no indexing - each triangle gets its own vertices)
    for (float lat = -90.0f; lat < 90.0f; lat += 10.0f) {
        for (float lon = 0.0f; lon < 360.0f; lon += 10.0f) {
            float next_lat = lat + 10.0f;
            float next_lon = (lon + 10.0f < 360.0f) ? lon + 10.0f : 0.0f;
            
            // Create 6 vertices for 2 triangles (duplicating shared vertices)
            // Triangle 1: (lat,lon) -> (next_lat,lon) -> (lat,next_lon)
            vertex_list.push_back(createVertex(lat, lon));
            vertex_list.push_back(createVertex(next_lat, lon));
            vertex_list.push_back(createVertex(lat, next_lon));
            
            // Triangle 2: (lat,next_lon) -> (next_lat,lon) -> (next_lat,next_lon)  
            vertex_list.push_back(createVertex(lat, next_lon));
            vertex_list.push_back(createVertex(next_lat, lon));
            vertex_list.push_back(createVertex(next_lat, next_lon));
        }
    }
}
}
