#include <Camera.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

void Camera::_update_view_mat() { _view_mat = glm::lookAt(_position, _position + _forward, _up); }
void Camera::_update_projection_mat() { _projection_mat = glm::perspectiveFov(_vert_fov, (float) _width, (float) _height, _z_near, _z_far); }

// TODO: double check default parameters
Camera::Camera(unsigned int width, unsigned int height, float vFOV, float zNear, float zFar, glm::vec3 position, glm::vec3 worldUp, glm::vec3 cameraForward, float speed, float sensitivity) :
	_width(width),
	_height(height),
	_vert_fov(vFOV), 
	_z_near(zNear), 
	_z_far(zFar), 
	_position(position), 
	_world_up(glm::normalize(worldUp)), 
	_forward(glm::normalize(cameraForward)),
	_right(glm::normalize(glm::cross(_forward, _world_up))), 
	_up(glm::normalize(glm::cross(_right, _forward))), 
	speed(speed), 
	sensitivity(sensitivity)
{
	_yaw = atan2(_forward.z, _forward.x);
	_pitch = asin(_forward.y);	// For some reason gives NaN in initializer list
	_update_view_mat();
	_update_projection_mat();
}

void Camera::move(glm::vec3 direction, bool fasterMovement, float deltaTime) 
{
	direction *= speed * deltaTime;
	if (fasterMovement) direction *= 2.0f;
	_position += direction.x * _right;
	_position += direction.y * _up;
	_position += direction.z * _forward;
	
	_update_view_mat();
};

void Camera::rotate(float yawOffset, float pitchOffset)
{
	_yaw += yawOffset * sensitivity;
	_pitch -= pitchOffset * sensitivity;
	// Rotation constraints (in radians) to avoid upside-down Camera
	if (_pitch > 1.57f) _pitch = 1.57f;
	if (_pitch < -1.57f) _pitch = -1.57f;

	glm::vec3 newForward;
	newForward.x = cos(_yaw) * cos(_pitch);
	newForward.y = sin(_pitch);
	newForward.z = sin(_yaw) * cos(_pitch);

	_forward = glm::normalize(newForward);
	_right = glm::normalize(glm::cross(_forward, _world_up));
	_up = glm::normalize(glm::cross(_right, _forward));

	_update_view_mat();
};

unsigned int Camera::get_width() const { return _width; };
unsigned int Camera::get_height() const { return _height; };
void Camera::set_viewport_size(unsigned int newWidth, unsigned int newHeight)
{
	_width = newWidth;
	_height = newHeight;
	_update_projection_mat();
};

float Camera::get_vert_fov() const { return _vert_fov; };
void Camera::set_vert_fov(float newValue) 
{ 
	if (newValue < 0.01f) newValue = 0.01f;	// ~1°
	if (newValue > 6.28f) newValue = 6.28f;	// ~359°
	_vert_fov = newValue;
	_update_projection_mat();
};

float Camera::get_z_near() const { return _z_near; };
void Camera::set_z_near(float newValue) { _z_near = newValue; _update_projection_mat(); };

float Camera::get_z_far() const { return _z_far; };
void Camera::set_z_far(float newValue) { _z_far = newValue; _update_projection_mat(); };

const glm::vec3& Camera::get_position() const { return _position; };
void Camera::set_position(glm::vec3 newPosition) { _position = newPosition; _update_view_mat(); };

const glm::mat4& Camera::get_view_mat() const{ return _view_mat; }
const glm::mat4 Camera::get_view_rotation_mat() const { return glm::mat4(glm::mat3(_view_mat)); }	// "Cuts" the translation part of the viewMat
const glm::mat4& Camera::get_projection_mat() const{ return _projection_mat; }

float Camera::get_yaw() const { return _yaw; };
float Camera::get_pitch() const { return _pitch; };