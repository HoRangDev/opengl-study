#include "Mesh.h"

Mesh::Mesh(const std::vector<Vertex>& vertices,
   const std::vector<unsigned int>& indices,
   const std::vector<Texture>& textures) :
   m_vertices(vertices),
   m_indices(indices),
   m_textures(textures)
{
   SetupMesh();
}

void Mesh::SetupMesh()
{
   glGenVertexArrays(1, &VAO);
   glGenBuffers(1, &VBO);
   glGenBuffers(1, &EBO);

   glBindVertexArray(VAO);
   glBindBuffer(GL_ARRAY_BUFFER, VBO);

   glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex),
      m_vertices.data(), GL_STATIC_DRAW);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int),
      m_indices.data(), GL_STATIC_DRAW);

   // Position
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

   // Normal
   glEnableVertexAttribArray(1);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

   // Texture coordinates
   glEnableVertexAttribArray(2);
   glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

   glBindVertexArray(0);
}

void Mesh::Draw(Shader shader)
{
   unsigned int diffuseNr = 0;
   unsigned int specularNr = 0;
   for (unsigned int idx = 0; idx < m_textures.size(); ++idx)
   {
      glActiveTexture(GL_TEXTURE0 + idx);
      std::string number;
      std::string name = m_textures[idx].type;
      if (name == "texture_diffuse")
      {
         number = std::to_string(++diffuseNr);
      }
      else if (name == "texture_specular")
      {
         number = std::to_string(++specularNr);
      }

      shader.SetInt(("material." + name + number).c_str(), idx);
      glBindTexture(GL_TEXTURE_2D, m_textures[idx].id);
   }
   glActiveTexture(GL_TEXTURE0);

   glBindVertexArray(VAO);
   glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
   glBindVertexArray(0);
}

Mesh Mesh::CreateQuad( )
{
   std::vector<Vertex> vertices{ 
      { glm::vec3( 0.0f, 1.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ),  glm::vec2( 0.0f, 1.0f ) },
   { glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ),  glm::vec2( 0.0f, 0.0f ) },
   { glm::vec3( 1.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ),  glm::vec2( 1.0f, 0.0f ) },
   { glm::vec3( 1.0f, 1.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ),  glm::vec2( 1.0f, 1.0f ) }
   };

   std::vector<unsigned int> indices{
      0,1,2,
      2,3,0
   };

   Mesh mesh{ vertices, indices, std::vector<Texture>( ) };
   return mesh;
}