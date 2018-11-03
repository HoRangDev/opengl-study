#include "Model.h"

void Model::Draw(Shader shader)
{
   for (unsigned int idx = 0; idx < m_meshes.size(); ++idx)
   {
      m_meshes[idx].Draw(shader);
   }
}

void Model::LoadModel(const std::string& path)
{
   Assimp::Importer importer;
   const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
   if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr)
   {
      std::cout << "ERROR:ASSIMP:" << importer.GetErrorString() << std::endl;
      return;
   }
   m_directory = path.substr(0, path.find_last_of('/'));

   ProcessNode(scene->mRootNode, scene);
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
   // Process all the current node's meshes
   for (unsigned int idx = 0; idx < node->mNumMeshes; ++idx)
   {
      aiMesh* mesh = scene->mMeshes[node->mMeshes[idx]];
      m_meshes.push_back(ProcessMesh(mesh, scene));
   }

   // also call this function recursively for children
   for (unsigned int idx = 0; idx < node->mNumChildren; ++idx)
   {
      ProcessNode(node->mChildren[idx], scene);
   }
}