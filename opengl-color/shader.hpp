#ifndef SHADER_HPP
#define SHADER_HPP
#include <glm/glm.hpp>

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);
void setMat4(const GLuint &ProgramID, const std::string &name, const glm::mat4 &mat);
void setVec3(const GLuint &ProgramID, const std::string &name, float x, float y, float z);

#endif
