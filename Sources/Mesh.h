#pragma once
#include <string>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "glm/glm.hpp"
#include "Shader.h"

struct Vertex
{
   glm::vec3 Position;
   glm::vec3 Normal;
   glm::vec2 TexCoords;
};

struct Texture
{
   unsigned int id;
   std::string type;
   aiString path;
};

class Mesh
{
public:
   Mesh(const std::vector<Vertex>& vertices,
      const std::vector<unsigned int>& indices,
      const std::vector<Texture>& textures);

   void Draw(Shader shader);

public:
   static Mesh CreateQuad( );

private:
   void SetupMesh();

public:
   std::vector<Vertex> m_vertices;
   std::vector<unsigned int> m_indices;
   std::vector<Texture> m_textures;

private:
   unsigned int VAO;
   unsigned int VBO;
   unsigned int EBO;

};