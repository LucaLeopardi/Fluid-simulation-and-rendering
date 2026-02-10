#pragma once

#include <string>
#include <vector>
#include <glad/glad.h>
#include <utils/Mesh.hpp>
#include <utils/Vertex.hpp>
#include <utils/Transform.hpp>
#include <utils/Texture.hpp>

class Model
{
private:

	Transform _transform;
	glm::mat4 _model_mat;
	glm::mat3 _normal_mat;

	void _update_model_mat();
	void _update_normal_mat();

public:

	// TODO: Model name?
	// Models own Meshes and Textures. Meshes point to a Texture from their Model, or a standalone one.
	std::vector<Mesh> meshes;
	std::vector<Texture> textures;

	Model(const std::string& model_path);				// Model path constructor (Assimp import)
	
	Model(std::vector<Mesh>&& meshes);					// Constructor from rvalue vector of Meshes (as they're move-only)

	// Copy constructor/assignment are forbidden, as they are for Mesh objects
	Model(const Model& other) = delete;					// Copy constructor
	Model& operator=(const Model& other) = delete;		// Copy assignment

	Model(Model && other) noexcept = default;			// Move constructor
	Model& operator=(Model&& other) noexcept = default;	// Move assignment
	
	~Model();

	void cleanup();
	
	glm::vec3 get_scale() const;
	void set_scale(const glm::vec3 new_scale);

	glm::vec3 get_position() const;
	void set_position(const glm::vec3 new_position);
	
	glm::vec3 get_rotation() const;
	void set_rotation(const glm::vec3 new_rotation);
	
	const glm::mat4& get_model_mat() const;

	const glm::mat3& get_normal_mat() const;
};