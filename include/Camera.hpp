#pragma once

#include <glm/glm.hpp>

class Camera
{
private:

	float _width, _height, _vert_fov, _z_near, _z_far, _yaw, _pitch;
	glm::vec3 _position, _world_up, _forward, _right, _up;

	glm::mat4 _view_mat;
	glm::mat4 _inverse_view_mat;
	glm::mat4 _projection_mat;
	glm::mat4 _inverse_projection_mat;
	void _update_view_mat();
	void _update_projection_mat();

public:

	float speed, sensitivity;

	/// @brief 
	/// @param width Viewport width, in pixels.
	/// @param height VIewport height, in pixels.
	/// @param vFOV Vertical FOV, in radians.
	/// @param zNear Near clipping plane distance, in units.
	/// @param zFar Far clipping plan distance, in units.
	/// @param position Position Vector where to create the Camera.
	/// @param world_up Upward world vector.
	/// @param camera_forward Camera's forward direction vector.
	/// @param speed Camera's movement speed.
	/// @param sensitivity Camera's rotation speed.
	/// @param constrained Whether the Camera's pitch rotation is constrained to (-90°, +90°).
	Camera(
		unsigned int width,
		unsigned int height,
		float vFOV = 1.2f,	// Radians
		float zNear = 0.1f,
		float zFar = 5000.0f,
		glm::vec3 position = glm::vec3(0.0f),
		glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3 camera_forward = glm::vec3(0.0f, 0.0f, -1.0f),
		float speed = 10.0f,
		float sensitivity = 0.0015f,	// Very small because mouse offset (from main) is used as rotation radians directly
		bool constrained = true
	);

	void move(glm::vec3 direction, bool fasterMovement, float deltaTime);

	void rotate(float yaw, float pitch);

	void set_direction(glm::vec3 forward, glm::vec3 up);

	unsigned int get_width() const;
	unsigned int get_height() const;
	void set_viewport_size(unsigned int new_width, unsigned int new_height);

	/// @return The current vertical FOV expressed in radians.
	float get_vert_fov() const;
	/// @param new_value Desired vertical FOV expressed in radians. 
	void set_vert_fov(float new_value);

	float get_z_near() const;
	void set_z_near(float new_value);

	float get_z_far() const;
	void set_z_far(float new_value);

	const glm::vec3& get_position() const;
	void set_position(glm::vec3 new_position);

	const glm::mat4& get_view_mat() const;
	const glm::mat4& get_inverse_view_mat() const;
	const glm::mat4 get_view_rotation_mat() const;
	const glm::mat4& get_projection_mat() const;
	const glm::mat4& get_inverse_projection_mat() const;

	float get_yaw() const;
	float get_pitch() const;

	float get_horizontal_scale() const;
};