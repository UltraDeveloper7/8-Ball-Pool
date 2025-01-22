#ifndef DRAWABLE_HPP
#define DRAWABLE_HPP

#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <map>
#include <glm/glm.hpp>

static std::vector<unsigned int> VEC_UINT_DEFAUTL_VALUE{};
static std::vector<glm::vec3> VEC_VEC3_DEFAUTL_VALUE{};
static std::vector<glm::vec2> VEC_VEC2_DEFAUTL_VALUE{};
static std::map<std::string, GLuint> MAP_STRING_GLUINT_DEFAULT_VALUE{};

class Drawable {
public:
    Drawable(std::string path);

    Drawable(
        const std::vector<glm::vec3>& vertices,
        const std::vector<glm::vec2>& uvs = VEC_VEC2_DEFAUTL_VALUE,
        const std::vector<glm::vec3>& normals = VEC_VEC3_DEFAUTL_VALUE);

    ~Drawable();

    void bind();

    /* Bind VAO before calling draw */
    void draw(int mode = GL_TRIANGLES);

public:
    std::vector<glm::vec3> vertices, normals, indexedVertices, indexedNormals;
    std::vector<glm::vec2> uvs, indexedUVS;
    std::vector<unsigned int> indices;

    GLuint VAO, verticesVBO, uvsVBO, normalsVBO, elementVBO;

private:
    void createContext();
};

// Declare the function here
void loadOBJWithTiny(const char* path, std::vector<glm::vec3>& vertices, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals, std::vector<unsigned int>& indices);
void loadVTP(const char* path, std::vector<glm::vec3>& vertices, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals, std::vector<unsigned int>& indices);
void indexVBO(const std::vector<glm::vec3>& vertices, const std::vector<glm::vec2>& uvs, const std::vector<glm::vec3>& normals, std::vector<unsigned int>& indices, std::vector<glm::vec3>& indexedVertices, std::vector<glm::vec2>& indexedUVS, std::vector<glm::vec3>& indexedNormals);

extern Drawable* lightbulb;

#endif // DRAWABLE_HPP
