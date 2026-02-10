#include <Shader.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

Shader::Shader(const std::string vertex_source_path, const std::string fragment_source_path)
{
	// Open Shader Vertex and Fragment code and copy as c_str
	std::ifstream vertex_file;
	std::ifstream fragment_file;
	std::string vertex_string;
	std::string fragment_string;
	const char* vertex_code;
	const char* fragment_code;
	try
	{
		vertex_file.open(vertex_source_path);
		if (!vertex_file.is_open()) {
			std::cerr << "ERROR::SHADER::CONSTRUCTOR::VERTEX_FILE_NOT_FOUND: " << vertex_source_path << std::endl;
			return;
		}
		fragment_file.open(fragment_source_path);
		if (!fragment_file.is_open()) {
			std::cerr << "ERROR::SHADER::CONSTRUCTOR::FRAGMENT_FILE_NOT_FOUND: " << fragment_source_path << std::endl;
			return;
		}
		std::stringstream vertex_stream, fragment_stream;
		vertex_stream << vertex_file.rdbuf();
		fragment_stream << fragment_file.rdbuf();
		vertex_file.close();
		fragment_file.close();
		vertex_string = vertex_stream.str();
		fragment_string = fragment_stream.str();
	}
	catch(const std::ifstream::failure e)
	{
		std::cerr << "ERROR::SHADER::CONSTRUCTOR::FILE_READ_FAILED\n" << e.what() << std::endl;
		return;
	}
	vertex_code = vertex_string.c_str();
	fragment_code = fragment_string.c_str();
	
	// Shader compilation
	
	GLuint vertex_shader, fragment_shader;
	int success;
	char info_log[512];

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_code, NULL);
	glCompileShader(vertex_shader);
	
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
		std:: cerr << "ERROR::SHADER::CONSTRUCTOR::VERTEX_COMPILATION_FAILED\n" << info_log << std::endl;
		return;
	}

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_code, NULL);
	glCompileShader(fragment_shader);
	
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
		std:: cerr << "ERROR::SHADER::CONSTRUCTOR::FRAGMENT_COMPILATION_FAILED\n" << info_log << std::endl;
		return;
	}

	// Shader linking and cleanup
	
	_id = glCreateProgram();
	glAttachShader(_id, vertex_shader);
	glAttachShader(_id, fragment_shader);
	glLinkProgram(_id);

	glGetProgramiv(_id, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(_id, 512, NULL, info_log);
		std:: cerr << "ERROR::SHADER::CONSTRUCTOR::PROGRAM_LINKING_FAILED\n" << info_log << std::endl;
		return;
	}
	
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
}

// Move constructor
Shader::Shader(Shader&& other) noexcept
{
	_id = other._id;
	other._id = 0;
}

Shader & Shader::operator=(Shader&& other) noexcept
{
	if (this != &other) {
		glDeleteProgram(_id);	// Leave no dangling OpenGL resources
		_id = other._id;
		other._id = 0;
	}
	return *this;
}

Shader::~Shader() { glDeleteProgram(_id); }

GLuint Shader::get_ID() const { return _id; }

GLint Shader::get_uniform_location(const GLchar* name)
{
	// Check if location is already in cache
	if (_uniform_locations_cache.find(name) != _uniform_locations_cache.end())
		return _uniform_locations_cache[name];
	// Otherwise, get it from OpenGL
	GLint location = glGetUniformLocation(_id, name);
	if (location == -1)
		std::wcerr << "WARNING::SHADER::" << _id << "::UNIFORM\n" << "Trying to get unset location for uniform: " << name << std::endl;
	else
		_uniform_locations_cache[name] = location;
	return location;
}

void Shader::set_uniform_1b(const std::string& name, const bool value) { glUniform1i(get_uniform_location(name.c_str()), (GLint) value); }

void Shader::set_uniform_1i(const std::string& name, const int value) { glUniform1i(get_uniform_location(name.c_str()), value); }

void Shader::set_uniform_1f(const std::string& name, const float value) { glUniform1f(get_uniform_location(name.c_str()), value); }

void Shader::set_uniform_1fv(const std::string& name, const int count, const float* first_value) { glUniform1fv(get_uniform_location(name.c_str()), count, first_value); }

void Shader::set_uniform_vec2(const std::string& name, const glm::vec2& vector) 
{ 
	glUniform2fv(get_uniform_location(name.c_str()), 1, glm::value_ptr(vector)); 
}

void Shader::set_uniform_vec3(const std::string& name, const glm::vec3& vector) 
{ 
	glUniform3fv(get_uniform_location(name.c_str()), 1, glm::value_ptr(vector)); 
}

void Shader::set_uniform_vec3v(const std::string & name, const glm::vec3* vectors, const GLsizei count)
{
	glUniform3fv(get_uniform_location(name.c_str()), count, glm::value_ptr(vectors[0])); 
}

void Shader::set_uniform_uvec3(const std::string& name, const glm::uvec3& vector) 
{ 
	glUniform3uiv(get_uniform_location(name.c_str()), 1, glm::value_ptr(vector)); 
}

void Shader::set_uniform_mat3(const std::string& name, const glm::mat3& matrix) 
{ 
	glUniformMatrix3fv(get_uniform_location(name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix)); 
}

void Shader::set_uniform_mat4(const std::string& name, const glm::mat4& matrix) 
{ 
	glUniformMatrix4fv(get_uniform_location(name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
}