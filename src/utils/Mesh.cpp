#include <utils/Mesh.hpp>

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices)
{
	GLuint buffers[2];
	glGenBuffers(2, buffers);
	_vbo = buffers[0];
	_ebo = buffers[1];
	glGenVertexArrays(1, &_vao);

	glBindVertexArray(_vao);
	
	// VBO
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	// EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
	_ebo_length = indices.size();

	// VAO attributes
	// Position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position));
	// Normal
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));
	// Texture coordinates
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, tex_coords));
	// Base vertex color
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, base_color));

	glBindVertexArray(0);
}

// Move constructor
Mesh::Mesh(Mesh&& other) noexcept
{
	_vao = other._vao;
	_vbo = other._vbo;
	_ebo = other._ebo;
	_ebo_length = other._ebo_length;
	diffuse_tex = other.diffuse_tex;

	other._vao = 0;
	other._vbo = 0;
	other._ebo = 0;
	other._ebo_length = 0;
	other.diffuse_tex = nullptr;	// Multiple Meshes CAN point to the same Texture. But it makes no sense with no vbo, so reset that as well.
}

// Move assignment
Mesh& Mesh::operator=(Mesh&& other) noexcept
{
	if (this != &other) {	// Forbid self-move
		cleanup();			// Clean up this before moving data to it, so as to not leave resources dangling
		_vao = other._vao;
		_vbo = other._vbo;
		_ebo = other._ebo;
		_ebo_length = other._ebo_length;
		diffuse_tex = other.diffuse_tex;

		other._vao = 0;
		other._vbo = 0;
		other._ebo = 0;
		other._ebo_length = 0;		
		other.diffuse_tex = nullptr;	// Multiple Meshes CAN point to the same Texture. But it makes no sense with no vbo, so reset that as well.
	}
	return *this;
}

Mesh::~Mesh() { cleanup(); }

void Mesh::cleanup()
{
	glDeleteVertexArrays(1, &_vao);
	_vao = 0;
	
	GLuint buffers[] = {_vbo, _ebo};
	glDeleteBuffers(2, buffers);
	_vbo = 0;
	_ebo = 0;
	_ebo_length = 0;

	diffuse_tex = nullptr;
}

GLuint Mesh::get_VBO() const { return _vbo; }
GLuint Mesh::get_EBO() const { return _ebo; }
GLuint Mesh::get_EBO_length() const { return _ebo_length; }
GLuint Mesh::get_VAO() const { return _vao; }