//============================================================================
//	Johns Hopkins University Whiting School of Engineering
//	605.667  Computer Graphics
//	Instructor:	Brian Russin
//
//	Author:  David W. Nesbitt
//	File:    Module5/main.cpp
//	Purpose: OpenGL and SDL program to draw a simple 3D animation with
//          collision detection. The scene contains moving spheres inside
//          a fixed, cube enclosure. A simple scene graph is used to define the
//          scene. Transform nodes, geometry nodes, and presentation nodes
//          are used.
//
//============================================================================

#include "filesystem_support/file_locator.hpp"
#include "geometry/bounding_sphere.hpp"
#include "geometry/geometry.hpp"
#include "geometry/plane.hpp"
#include "scene/color_node.hpp"
#include "scene/graphics.hpp"
#include "scene/scene.hpp"

#include "Module5/lighting_shader_node.hpp"
#include "Module5/unit_square_node.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include "Module5/unit_sphere_node.hpp"
#include <Module5/ball_transform.hpp>
//for random
#include <cstdlib>
#include <ctime>
namespace cg
{

// Simple logging function, should be defined in the cg namespace
void logmsg(const char *message, ...)
{
    // Open file if not already opened
    static FILE *lfile = NULL;
    if(lfile == NULL) { lfile = fopen("Module5.log", "w"); }

    va_list arg;
    va_start(arg, message);
    vfprintf(lfile, message, arg);
    putc('\n', lfile);
    fflush(lfile);
    va_end(arg);
}

} // namespace cg
// SDL Objects
SDL_Window       *g_sdl_window = nullptr;
SDL_GLContext     g_gl_context;
constexpr int32_t DRAWS_PER_SECOND = 72;
constexpr int32_t DRAW_INTERVAL_MILLIS =
    static_cast<int32_t>(1000.0 / static_cast<double>(DRAWS_PER_SECOND));

// Root of the scene graph
std::shared_ptr<cg::SceneNode> g_scene_root;

cg::SceneState g_scene_state;

std::shared_ptr<cg::BallTransform> g_test_ball;
std::vector<std::shared_ptr<cg::BallTransform>> g_balls;
std::vector<cg::Plane> g_bounding_planes;


//function to assist in ball creation 
void create_balls(std::shared_ptr<cg::UnitSphere> unit_sphere,
                  std::shared_ptr<cg::SceneNode> shader)
{
   g_balls.clear();

   //colors 
   std::vector<cg::Color4> colors = {
      cg::Color4(1.0f, 0.0f, 0.0f), 
      cg::Color4(0.0f, 1.0f, 0.0f),
      cg::Color4(0.0f, 0.0f, 1.0f)
   };

   //create 9 balsl 
   for(int color_idx = 0; color_idx < 3; color_idx++)
   {
      for(int ball_idx = 0; ball_idx < 3; ball_idx++)
      {
         // x,y values between -40 and 40, z values between 25 and 75
         float x = -40.0f + (std::rand() / (float)RAND_MAX) * 80.0f;  // -40 to +40
         float y = -40.0f + (std::rand() / (float)RAND_MAX) * 80.0f;  // -40 to +40
         float z = 25.0f + (std::rand() / (float)RAND_MAX) * 50.0f;   // 25 to 75
            
         cg::Point3 start_pos(x, y, z);
         
         // Random radius between 3 and 7 units
         float radius = 3.0f + (std::rand() / (float)RAND_MAX) * 4.0f;
         
         // Random unit direction vector
         float dx = (std::rand() / (float)RAND_MAX) * 2.0f - 1.0f;  // -1 to 1
         float dy = (std::rand() / (float)RAND_MAX) * 2.0f - 1.0f;  // -1 to 1
         float dz = (std::rand() / (float)RAND_MAX) * 2.0f - 1.0f;  // -1 to 1
         
         cg::Vector3 direction(dx, dy, dz);
         direction.normalize(); // Ensure unit length

         float speed = 5.0f + (std::rand() / (float)RAND_MAX) * 10.0f;

         //create the ball transform 
         std::shared_ptr<cg::BallTransform> ball_transform = std::make_shared<cg::BallTransform>(
            radius, start_pos, direction, speed
         );

         //create the color node 
          std::shared_ptr<cg::ColorNode> ball_color = std::make_shared<cg::ColorNode>(colors[color_idx]);

         //add them to the scene graph 
         shader->add_child(ball_color);
         ball_color->add_child(ball_transform);
         ball_transform->add_child(unit_sphere);

         //add to global list 
         g_balls.push_back(ball_transform);
      }
   }
}
// Sleep function to help run a reasonable timer
void sleep(int32_t milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

/**
 Display callback. Clears the prior scene and draws a new one.
 */
void display()
{
    // Clear the framebuffer and the depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Init scene state and draw the scene graph
    g_scene_state.init();
    g_scene_root->draw(g_scene_state);

    // Swap buffers
    SDL_GL_SwapWindow(g_sdl_window);
}

/**
 * Keyboard event handler.
 */
bool handle_key_event(const SDL_Event &event)
{
    bool cont_program = true;

    switch(event.key.key)
    {
        case SDLK_ESCAPE: cont_program = false; break;
        default: break;
    }

    return cont_program;
}

/**
 * Reshape function. Load a 2-D orthographic projection matrix using the
 * window width and height so we can directly take window coordinates and
 * send to OpenGL
 * @param  width  Window width
 * @param  height Window height
 */
void reshape(int32_t width, int32_t height)
{
    // Center the world window in the viewport so it does not
    // stretch world window
    float window_aspect = static_cast<float>(width) / static_cast<float>(height);
    if(window_aspect > 1.0f)
    {
        // Window is wider than world
        int32_t w = static_cast<int32_t>(height);
        int32_t start_x = static_cast<int32_t>((width - w) * 0.5f);
        glViewport(start_x, 0, w, height);
    }
    else
    {
        // Window is taller than world
        int32_t h = static_cast<int32_t>(width);
        int32_t start_y = static_cast<int32_t>((height - h) * 0.5f);
        glViewport(0, start_y, width, h);
    }
}

/**
 * Window event handler.
 */
bool handle_window_event(const SDL_Event &event)
{
    bool cont_program = true;

    switch(event.type)
    {
        case SDL_EVENT_WINDOW_RESIZED:
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            reshape(event.window.data1, event.window.data2);
            break;
        default: break;
    }

    return cont_program;
}

/**
 * Handle Events function.
 */
bool handle_events()
{
    SDL_Event e;
    bool      cont_program = true;
    while(SDL_PollEvent(&e))
    {
        switch(e.type)
        {
            case SDL_EVENT_QUIT:
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED: cont_program = false; break;

            case SDL_EVENT_WINDOW_RESIZED:
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: cont_program = handle_window_event(e); break;

            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP: cont_program = handle_key_event(e); break;
            default: break;
        }
    }
    return cont_program;
}

/**
 * Construct a unit box with outward facing normals.
 * @param  unit_square  Geometry node to use
 */
std::shared_ptr<cg::SceneNode> construct_unit_box(std::shared_ptr<cg::UnitSquare> unit_square)
{
    // Contruct transform nodes for the sides of the box.
    // Perform rotations so the sides face outwards

    // Bottom is rotated 180 degrees so it faces outwards
    auto bottom_transform = std::make_shared<cg::TransformNode>();
    bottom_transform->translate(0.0f, 0.0f, -0.5f);
    bottom_transform->rotate_x(180.0f);

    // Back is rotated -90 degrees about x: (z -> y)
    auto back_transform = std::make_shared<cg::TransformNode>();
    back_transform->translate(0.0f, 0.5f, 0.0f);
    back_transform->rotate_x(-90.0f);

    // Front wall is rotated 90 degrees about x: (y -> z)
    auto front_transform = std::make_shared<cg::TransformNode>();
    front_transform->translate(0.0f, -0.5f, 0.0f);
    front_transform->rotate_x(90.0f);

    // Left wall is rotated -90 about y: (z -> -x)
    auto left_transform = std::make_shared<cg::TransformNode>();
    left_transform->translate(-0.5f, 0.0f, 00.0f);
    left_transform->rotate_y(-90.0f);

    // Right wall is rotated 90 degrees about y: (z -> x)
    auto right_transform = std::make_shared<cg::TransformNode>();
    right_transform->translate(0.5f, 0.0f, 0.0f);
    right_transform->rotate_y(90.0f);

    // Top
    auto top_transform = std::make_shared<cg::TransformNode>();
    top_transform->translate(0.0f, 0.0f, 0.50f);

    // Create a SceneNode and add the 6 sides of the box.
    auto box = std::make_shared<cg::SceneNode>();
    box->add_child(back_transform);
    back_transform->add_child(unit_square);
    box->add_child(left_transform);
    left_transform->add_child(unit_square);
    box->add_child(right_transform);
    right_transform->add_child(unit_square);
    box->add_child(front_transform);
    front_transform->add_child(unit_square);
    box->add_child(bottom_transform);
    bottom_transform->add_child(unit_square);
    box->add_child(top_transform);
    top_transform->add_child(unit_square);

    return box;
}

void check_ball_plane_collisions(std::shared_ptr<cg::BallTransform> ball)
{
   cg::Point3 position = ball->getPosition();
   float radius  = ball->getRadius();
   cg::Vector3 velocity = ball->getVelocity();

   //assume frame time 
   float frame_time = 1.0f / 72.0f;

   float closest_collision_time = frame_time;
   cg::Plane* collision_plane = nullptr;

   for(auto& plane : g_bounding_planes)
   {
      //calculate signed distance from ball center to plane 
      float signed_dist = plane.solve(position);

      //if ball is moving toward plane
      float velocity_toward_plane = -velocity.dot(plane.get_normal());

      if(velocity_toward_plane > 0)
      {
         //calculate distance to collision 
         float collisison_distance = signed_dist - radius;

         if(collisison_distance <= velocity_toward_plane * frame_time 
            && collisison_distance > 0)
         {
            float collision_time = collisison_distance / velocity_toward_plane;
            if(collision_time < closest_collision_time 
               &&collision_time >= 0)
            {
               closest_collision_time = collision_time;
               collision_plane = &plane;
            }
         }
      }
   }

   //set collision data in the ball transform 
   if(collision_plane != nullptr)
      ball->setCollision(closest_collision_time, collision_plane);
   else 
      ball->clearCollision();
}

bool check_ball_ball_collision(std::shared_ptr<cg::BallTransform> ball1,
                               std::shared_ptr<cg::BallTransform> ball2)
{
   cg::Point3 pos1 = ball1->getPosition();
   cg::Point3 pos2 = ball2->getPosition();
   cg::Vector3 vel1 = ball1->getVelocity();
   cg::Vector3 vel2 = ball2->getVelocity();

   float radius1 = ball1->getRadius();
   float radius2 = ball2->getRadius();

   //see if they're stuck together
   cg::Vector3 separation = pos2 - pos1;
   float distance = separation.norm();
   float combined_radius = radius1 + radius2;

   //already intersecting reposition and move on 
   if(distance < combined_radius)
   {
      if(distance > 0.001f) //avoid a zero div 
      {
         cg::Vector3 separation_unit = separation;
         separation_unit.normalize();
         float overlap = combined_radius - distance;

         cg::Point3 new_pos1 = pos1 - separation_unit * (overlap * 0.5f);
         cg::Point3 new_pos2 = pos2 + separation_unit * (overlap * 0.5f);

         ball1->setPosition(new_pos1);
         ball2->setPosition(new_pos2);
         // Reflect velocities off the collision plane
         cg::Vector3 new_vel1 = vel1.reflect(separation_unit);
         cg::Vector3 new_vel2 = vel2.reflect(separation_unit * -1);
            
         ball1->setVelocity(new_vel1);
         ball2->setVelocity(new_vel2);
      }
      return true;
   }

   cg::Vector3 relative_velocity = vel1 - vel2;
   //no relative motion
   if(relative_velocity.norm() < 0.001f) 
      return false;
   
   cg::Ray3 ray(pos1, relative_velocity.normalize());
   cg::BoundingSphere sphere(pos2, combined_radius);

   float frame_time = 1.0f / 72.0f;

   auto intersection = ray.intersect(sphere);

   if(intersection.intersects && intersection.distance >= 0 &&
      intersection.distance <= relative_velocity.norm() * frame_time)
   {
      //calculate collision point and normal 
      cg::Point3 collision_point = ray.intersect(intersection.distance);
      cg:: Vector3 collision_normal = (collision_point - pos2).normalize();

      //reflect the velocity 
      cg::Vector3 new_vel1 = vel1.reflect(collision_normal);
      cg::Vector3 new_vel2 = vel2.reflect(collision_normal * -1);

      ball1->setVelocity(new_vel1);
      ball2->setVelocity(new_vel2);

      return true;
   }
   return false;
}
// Initialize bounding planes for the room
void initialize_bounding_planes()
{
    g_bounding_planes.clear();
    
    // Floor (z = 0, normal pointing up)
    g_bounding_planes.push_back(cg::Plane(cg::Point3(0, 0, 0), cg::Vector3(0, 0, 1)));
    
    // Ceiling (z = 100, normal pointing down)  
    g_bounding_planes.push_back(cg::Plane(cg::Point3(0, 0, 100), cg::Vector3(0, 0, -1)));
    
    // Left wall (x = -50, normal pointing right)
    g_bounding_planes.push_back(cg::Plane(cg::Point3(-50, 0, 0), cg::Vector3(1, 0, 0)));
    
    // Right wall (x = 50, normal pointing left)
    g_bounding_planes.push_back(cg::Plane(cg::Point3(50, 0, 0), cg::Vector3(-1, 0, 0)));
    
    // Back wall (y = 50, normal pointing toward camera)
    g_bounding_planes.push_back(cg::Plane(cg::Point3(0, 50, 0), cg::Vector3(0, -1, 0)));
    
    // Front wall (y = -50, normal pointing away from camera) 
    g_bounding_planes.push_back(cg::Plane(cg::Point3(0, -50, 0), cg::Vector3(0, 1, 0)));
}

void detect_collisions()
{
   //see if we hit a wall
   for(auto& ball : g_balls)
   {
      check_ball_plane_collisions(ball);
   }

   //see if we hit our neighbors...this has to be so slow
   for(size_t i=0; i < g_balls.size(); i++)
   {
      for(size_t j = i + 1; j < g_balls.size(); j++)
      {
         check_ball_ball_collision(g_balls[i], g_balls[j]);
      }
   }
}
// Updated construct_scene function
void construct_scene()
{
    // Shader node
    auto shader = std::make_shared<cg::LightingShaderNode>();
    if(!shader->create("Module5/simple_light.vert", "Module5/simple_light.frag") ||
       !shader->get_locations())
    {
        exit(-1);
    }

    auto unit_square =
        std::make_shared<cg::UnitSquare>(shader->get_position_loc(), shader->get_normal_loc());

    // Wall transforms (same as before)
    auto floor_transform = std::make_shared<cg::TransformNode>();
    floor_transform->scale(100.0f, 100.0f, 100.0f);

    auto back_wall_transform = std::make_shared<cg::TransformNode>();
    back_wall_transform->translate(0.0f, 50.0f, 50.0f);
    back_wall_transform->rotate_x(90.0f);
    back_wall_transform->scale(100.0f, 100.0f, 1.0f);

    auto left_wall_transform = std::make_shared<cg::TransformNode>();
    left_wall_transform->translate(-50.0f, 0.0f, 50.0f);
    left_wall_transform->rotate_y(90.0f);
    left_wall_transform->scale(100.0f, 100.0f, 1.0f);

    auto right_wall_transform = std::make_shared<cg::TransformNode>();
    right_wall_transform->translate(50.0f, 0.0f, 50.0f);
    right_wall_transform->rotate_y(-90.0f);
    right_wall_transform->scale(100.0f, 100.0f, 1.0f);

    auto ceiling_transform = std::make_shared<cg::TransformNode>();
    ceiling_transform->translate(0.0f, 0.0f, 100.0f);
    ceiling_transform->rotate_x(180.0f);
    ceiling_transform->scale(100.0f, 100.0f, 1.0f);

    // Wall colors
    auto floor_color = std::make_shared<cg::ColorNode>(cg::Color4(0.6f, 0.5f, 0.2f));
    auto back_wall_color = std::make_shared<cg::ColorNode>(cg::Color4(0.9f, 0.7f, 0.5f));
    auto wall_color = std::make_shared<cg::ColorNode>(cg::Color4(1.0f, 1.0f, 1.0f));
    auto ceiling_color = std::make_shared<cg::ColorNode>(cg::Color4(0.1f, 0.4f, 1.0f));

    // Create unit sphere for all balls
    auto unit_sphere = std::make_shared<cg::UnitSphere>(
        shader->get_position_loc(), shader->get_normal_loc());

    // Construct the scene layout
    g_scene_root = std::make_shared<cg::SceneNode>();
    g_scene_root->add_child(shader);

    // Add walls to scene
    shader->add_child(back_wall_color);
    back_wall_color->add_child(back_wall_transform);
    back_wall_transform->add_child(unit_square);
    
    shader->add_child(wall_color);
    wall_color->add_child(left_wall_transform);
    left_wall_transform->add_child(unit_square);
    wall_color->add_child(right_wall_transform);
    right_wall_transform->add_child(unit_square);
    
    shader->add_child(floor_color);
    floor_color->add_child(floor_transform);
    floor_transform->add_child(unit_square);
    
    shader->add_child(ceiling_color);
    ceiling_color->add_child(ceiling_transform);
    ceiling_transform->add_child(unit_square);

    // Create all 9 balls and add them to scene
    create_balls(unit_sphere, shader);
    
    // Initialize bounding planes for collision detection
    initialize_bounding_planes();
}

/**
 * Main
 */
int main(int argc, char **argv)
{
    cg::set_root_paths(argv[0]);

    // Initialize SDL
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cout << "Error initializing SDL: " << SDL_GetError() << '\n';
        exit(1);
    }

    // Initialize display mode and window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_PropertiesID props = SDL_CreateProperties();
    if(props == 0)
    {
        std::cout << "Error creating SDL Window Properties: " << SDL_GetError() << '\n';
        exit(1);
    }

    SDL_SetStringProperty(
        props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "Simple Animation by Brian Russin");
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, 800);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, 800);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, 200);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, 200);

    g_sdl_window = SDL_CreateWindowWithProperties(props);
    if(g_sdl_window == nullptr)
    {
        std::cout << "Error initializing SDL Window" << SDL_GetError() << '\n';
        exit(1);
    }

    // Initialize OpenGL
    g_gl_context = SDL_GL_CreateContext(g_sdl_window);

    std::cout << "OpenGL  " << glGetString(GL_VERSION) << ", GLSL "
              << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';

#if BUILD_WINDOWS
    int32_t glew_init_result = glewInit();
    if(GLEW_OK != glew_init_result)
    {
        std::cout << "GLEW Error: " << glewGetErrorString(glew_init_result) << '\n';
        exit(EXIT_FAILURE);
    }
#endif

    // Set the clear color to black. Any part of the window outside the
    // viewport should appear black
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Construct scene.
    construct_scene();

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Enable back face polygon removal
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    // Set a fixed perspective projection. fov = 70, aspect = 1.0, near = 1.0 far = 200.
    // We are hard-coding viewing and projection matrices since they do not change in
    // this application.
    cg::Matrix4x4 projection;
    projection.m00() = 1.428f;
    projection.m11() = 1.428f;
    projection.m22() = -1.010f;
    projection.m23() = -2.010f;
    projection.m32() = -1.0f;
    projection.m33() = 0.0f;

    // Set a fixed camera outside the center of the front wall (imagine
    // it being a window) looking parallel to the floor
    cg::Matrix4x4 view;
    view.m00() = 1.0f;
    view.m11() = 0.0f;
    view.m12() = 1.0f;
    view.m13() = -50.0f;
    view.m21() = -1.0f;
    view.m22() = 0.0f;
    view.m23() = -90.0f;
    view.m33() = 1.0f;

    // Set the composite projection and viewing matrix
    // These remain fixed.
    g_scene_state.pv = projection * view;

    glViewport(0, 0, 800, 800);

    // Main loop
    while(handle_events())
    {
        //detect collisions 
        detect_collisions();

        g_scene_root->update(g_scene_state); 
        display();
        sleep(DRAW_INTERVAL_MILLIS);
    }

    // Destroy OpenGL Context, SDL Window and SDL
    SDL_GL_DestroyContext(g_gl_context);
    SDL_DestroyWindow(g_sdl_window);
    SDL_Quit();

    return 0;
}
