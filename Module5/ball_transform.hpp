#ifndef __BALL_TRANSFORM_HPP__
#define __BALL_TRANSFORM_HPP__

#include "scene/transform_node.hpp"
#include "geometry/point3.hpp"
#include "geometry/vector3.hpp"
#include "geometry/plane.hpp"
#include "geometry/bounding_sphere.hpp"

namespace cg {

class BallTransform : public TransformNode {
private:
    Point3 position;
    Vector3 velocity;      // direction * speed combined
    float radius;
    float speed;
    
    // Collision detection state
    float intersect_time;
    Plane* intersect_plane;
    bool collision_occurred;
    
    // Time delta for movement (assuming 72 FPS = 1/72 seconds per frame)
    static constexpr float FRAME_TIME = 1.0f / 72.0f;
    
public:
    BallTransform(float r, const Point3& pos, const Vector3& dir, float spd);
    virtual ~BallTransform() = default;
    
    // Override the update method from TransformNode
    void update(SceneState& scene_state) override;
    
    // Collision detection interface
    void setCollision(float t, Plane* plane);
    void clearCollision();
    
    // Getters for collision detection system
    Point3 getPosition() const { return position; }
    float getRadius() const { return radius; }
    Vector3 getVelocity() const { return velocity; }
    BoundingSphere getBoundingSphere() const { return BoundingSphere(position, radius); }
    
    // Setter for sphere-to-sphere collision
    void setVelocity(const Vector3& vel) { velocity = vel; }
    void setPosition(const Point3& pos) { position = pos; }
};

} // namespace cg

#endif
