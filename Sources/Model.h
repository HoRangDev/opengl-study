#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"

class Model
{
public:
   Model(const std::string& path)
   {
      LoadModel(path);
   }

   void Draw(Shader shader);

private:
   void LoadModel(const std::string& path);
   void ProcessNode(aiNode* node, const aiScene* scene);
   Mesh ProcessMesh(aiMesh* node, const aiScene* scene);

   std::vector<Texture> LoadMaterialTextures(aiMaterial* mat,
      aiTextureType type,
      const std::string& typeName);

private:
   std::vector<Mesh> m_meshes;
   std::string       m_directory;

};
