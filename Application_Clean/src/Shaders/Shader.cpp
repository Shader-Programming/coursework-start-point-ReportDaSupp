
#include <Shaders/shader.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <array>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	// 1. retrieve the vertex/fragment source code from filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;

	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		// open files
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		// read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// close file handlers
		vShaderFile.close();
		fShaderFile.close();
		// convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();


	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();


	// 2. compile shaders
	unsigned int vertex, fragment;
	// vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	checkCompileErrors(vertex, "VERTEX");
	// fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	checkCompileErrors(fragment, "FRAGMENT");


	// shader Program
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");
	// delete the shaders as they're linked into our program now and no longer necessery
	glDeleteShader(vertex);
	glDeleteShader(fragment);



}

Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath)
{

	// 1. retrieve the vertex/fragment source code from filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	std::ifstream gShaderFile;

	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		// open files
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		gShaderFile.open(geometryPath);
		std::stringstream vShaderStream, fShaderStream, gShaderStream;
		// read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		gShaderStream << gShaderFile.rdbuf();
		// close file handlers
		vShaderFile.close();
		fShaderFile.close();
		gShaderFile.close();
		// convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
		geometryCode = gShaderStream.str();


	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	const char* gShaderCode = geometryCode.c_str();


	// 2. compile shaders
	unsigned int vertex, fragment, geometry;
	// vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	checkCompileErrors(vertex, "VERTEX");
	// fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	checkCompileErrors(fragment, "FRAGMENT");
	// geometry Shader
	geometry = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(geometry, 1, &gShaderCode, NULL);
	glCompileShader(geometry);
	checkCompileErrors(geometry, "GEOMETRY");


	// shader Program
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glAttachShader(ID, geometry);
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");
	// delete the shaders as they're linked into our program now and no longer necessery
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	glDeleteShader(geometry);

}

Shader::Shader(const char* glslPath)
{

	enum Region { None = -1, Vertex = 0, Fragment, Geometry, TesselationControl, TesselationEvaluation, Compute };
	bool v = false; bool f = false; bool g = false; bool tc = false; bool te = false; bool c = false;
	std::string line;
	std::array<std::string, Region::Compute + 1> src;
	uint32_t region = Region::None;

	std::fstream handle(glslPath, std::ios::in);

	if (handle.is_open())
	{

		while (getline(handle, line))
		{
			if (line.find("#region Vertex") != std::string::npos) { region = Region::Vertex; v = true; continue; };
			if (line.find("#region Fragment") != std::string::npos) { region = Region::Fragment; f = true; continue; };
			if (line.find("#region Geometry") != std::string::npos) { region = Region::Geometry; g = true; continue; };
			if (line.find("#region TesselationControl") != std::string::npos) { region = Region::TesselationControl; tc = true; continue; };
			if (line.find("#region TesselationEvaluation") != std::string::npos) { region = Region::TesselationEvaluation; te = true; continue; };
			if (line.find("#region Compute") != std::string::npos) { region = Region::Compute; c = true; continue; };
			if (region != Region::None) src[region] += (line + "\n");
		}

	}
	else
	{
		return;
	}
	handle.close();

	const char* vShaderCode = src[Region::Vertex].c_str();
	const char* gShaderCode = src[Region::Geometry].c_str();
	const char* fShaderCode = src[Region::Fragment].c_str();
	const char* tcShaderCode = src[Region::TesselationControl].c_str();
	const char* teShaderCode = src[Region::TesselationEvaluation].c_str();
	const char* cShaderCode = src[Region::Compute].c_str();

	// 2. compile shaders
	unsigned int vertex, fragment, geometry, tesselationControl, tesselationEvaluation, compute;
	if (v)
	{
		// vertex shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		checkCompileErrors(vertex, "VERTEX");
}
	if (f)
	{
		// fragment Shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		checkCompileErrors(fragment, "FRAGMENT");
	}
	if (tc)
	{
		// tesselation control Shader
		tesselationControl = glCreateShader(GL_TESS_CONTROL_SHADER);
		glShaderSource(tesselationControl, 1, &tcShaderCode, NULL);
		glCompileShader(tesselationControl);
		checkCompileErrors(tesselationControl, "TESSELATION_CONTROL");
	}
	if (te)
	{
		// tesselation evaluation Shader
		tesselationEvaluation = glCreateShader(GL_TESS_EVALUATION_SHADER);
		glShaderSource(tesselationEvaluation, 1, &teShaderCode, NULL);
		glCompileShader(tesselationEvaluation);
		checkCompileErrors(tesselationEvaluation, "TESSELATION_EVALUATION");
	}
	if (g)
	{
		// geometry Shader
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &gShaderCode, NULL);
		glCompileShader(geometry);
		checkCompileErrors(geometry, "GEOMETRY");
	}
	if (c)
	{
		// compute Shader
		compute = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(compute, 1, &cShaderCode, NULL);
		glCompileShader(compute);
		checkCompileErrors(compute, "COMPUTE");
	}


	// shader Program
	ID = glCreateProgram();
	if (v) glAttachShader(ID, vertex);
	if (tc) glAttachShader(ID, tesselationControl);
	if (te) glAttachShader(ID, tesselationEvaluation);
	if (g) glAttachShader(ID, geometry);
	if (f) glAttachShader(ID, fragment);
	if (c) glAttachShader(ID, compute);

	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");

	// delete the shaders as they're linked into our program now and no longer necessery
	if (v) glDeleteShader(vertex);
	if (tc) glDeleteShader(tesselationControl);
	if (te) glDeleteShader(tesselationEvaluation);
	if (g) glDeleteShader(geometry);
	if (f) glDeleteShader(fragment);
	if (c) glDeleteShader(compute);
	

}

void Shader::use()
{
	glUseProgram(ID);
}

// ------------------------------------------------------------------------
void Shader::setBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
// ------------------------------------------------------------------------
void Shader::setInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
	glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setVec2(const std::string& name, float x, float y) const
{
	glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}
// ------------------------------------------------------------------------
void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
	glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setVec3(const std::string& name, float x, float y, float z) const
{

	glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}
// ------------------------------------------------------------------------
void Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
	glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setVec4(const std::string& name, float x, float y, float z, float w)
{
	glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}
// ------------------------------------------------------------------------
void Shader::setMat2(const std::string& name, const glm::mat2& mat) const
{
	glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat3(const std::string& name, const glm::mat3& mat) const
{
	glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat4(const std::string& name, const glm::mat4& mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// --------------------------------------------------------------------------
void Shader::setUniformBlock(const std::string& name, unsigned int uniformBlock) const
{
	unsigned int uniformBlockIndex = glGetUniformBlockIndex(ID, name.c_str());
	glUniformBlockBinding(ID, uniformBlockIndex, uniformBlock);
}


void Shader::checkCompileErrors(GLuint shader, std::string type)
{
	GLint success;
	GLchar infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
}



