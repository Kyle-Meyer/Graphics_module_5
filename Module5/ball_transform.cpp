#include "ball_transform.hpp"
#include <iostream>

namespace cg {

BallTransform::BallTransform(float r, const Point3& pos, const Vector3& dir, float spd)
    : TransformNode(), radius(r), position(pos), speed(spd),
      intersect_time(-1.0f), intersect_plane(nullptr), collision_occurred(false)
{
    // Combine direction and speed into velocity vector
    Vector3 normalized_dir = dir;
    normalized_dir.normalize();
    velocity = normalized_dir * speed;
    
    // Set initial transformation matrix
    load_identity();
    translate(position.x, position.y, position.z);
    scale(radius, radius, radius);
}

void BallTransform::update(SceneState& scene_state) 
{
    // Handle collision response first
    if (collision_occurred && intersect_plane != nullptr) 
    {
        // Reflect velocity off the intersect plane
        Vector3 normal = intersect_plane->get_normal();
        velocity = velocity.reflect(normal);
        collision_occurred = false;
        intersect_plane = nullptr;
    }
    
    // Move ball along current path
    Point3 movement = Point3(velocity.x * FRAME_TIME, 
                            velocity.y * FRAME_TIME, 
                            velocity.z * FRAME_TIME);
    position.x += movement.x;
    position.y += movement.y;
    position.z += movement.z;
    
    // Update transformation matrix
    load_identity();
    translate(position.x, position.y, position.z);
    scale(radius, radius, radius);
    
    // Call base class update for children
    TransformNode::update(scene_state);
}

void BallTransform::setCollision(float t, Plane* plane) 
{
    intersect_time = t;
    intersect_plane = plane;
    collision_occurred = true;
}

void BallTransform::clearCollision() 
{
    intersect_time = -1.0f;
    intersect_plane = nullptr;
    collision_occurred = false;
}

} // namespace cg
