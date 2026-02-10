#include <Model.hpp>
#include <glm/ext/matrix_transform.hpp>

const glm::vec3 X_AXIS (1.0f, 0.0f, 0.0f);
const glm::vec3 Y_AXIS (0.0f, 1.0f, 0.0f);
const glm::vec3 Z_AXIS (0.0f, 0.0f, 1.0f);

void Model::_update_model_mat()
{
	_model_mat = glm::mat4(1.0f);
	_model_mat = glm::translate(_model_mat, _transform.position);
	_model_mat = glm::rotate(_model_mat, _transform.rotation.x, X_AXIS);
	_model_mat = glm::rotate(_model_mat, _transform.rotation.y, Y_AXIS);
	_model_mat = glm::rotate(_model_mat, _transform.rotation.z, Z_AXIS);
	_model_mat = glm::scale(_model_mat, _transform.scale);
	_update_normal_mat();
}

void Model::_update_normal_mat()
{
	_normal_mat = glm::mat3(glm::transpose(glm::inverse(_model_mat)));
}

Model::Model(const std::string& modelPath)
{
	// TODO: Check for duplicate Textures in Assimp. Somehow. (Keep cache of Tex data on load, and skip if equal?)
	// TODO
}

Model::Model(std::vector<Mesh>&& meshes) :
	meshes(std::move(meshes))
{
	_update_model_mat();
}

Model::~Model() { cleanup(); }

void Model::cleanup() { for (Mesh& mesh : meshes) mesh.cleanup(); }

glm::vec3 Model::get_scale() const { return _transform.scale; }
void Model::set_scale(const glm::vec3 newScale)
{
	_transform.scale = newScale;
	_update_model_mat();
}

glm::vec3 Model::get_position() const { return _transform.position; }
void Model::set_position(const glm::vec3 newPosition)
{
	_transform.position = newPosition;
	_update_model_mat();
}

/// @return Axis rotations in radians.
glm::vec3 Model::get_rotation() const { return _transform.rotation; }
/// @param newRotation Axis rotations in radians.
void Model::set_rotation(const glm::vec3 newRotation)
{
	_transform.rotation = newRotation;
	_update_model_mat();
}

const glm::mat4& Model::get_model_mat() const { return _model_mat; }

const glm::mat3 & Model::get_normal_mat() const { return _normal_mat; } 
