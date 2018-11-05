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

Mesh Model::ProcessMesh( aiMesh* mesh, const aiScene* scene )
{
   std::vector<Vertex> vertices;
   std::vector<unsigned int> indices;
   std::vector<Texture> textures;

   // Process vertices
   for ( unsigned int idx = 0; idx < mesh->mNumVertices; ++idx )
   {
      Vertex vertex;
      vertex.Position = glm::vec3( mesh->mVertices[ idx ].x,
                                   mesh->mVertices[ idx ].y,
                                   mesh->mVertices[ idx ].z );
      vertex.Normal = glm::vec3( mesh->mNormals[ idx ].x,
                                 mesh->mNormals[ idx ].y,
                                 mesh->mNormals[ idx ].z );

      if ( mesh->mTextureCoords[ 0 ] != nullptr )
      {
         vertex.TexCoords = glm::vec2( mesh->mTextureCoords[ 0 ][ idx ].x,
                                       mesh->mTextureCoords[ 0 ][ idx ].y );
      }
      else
      {
         vertex.TexCoords = glm::vec2( 0.0f, 0.0f );
      }

      vertices.push_back( vertex );
   }

   // Process indices
   for ( unsigned int idx = 0; idx < mesh->mNumFaces; ++idx )
   {
      aiFace face = mesh->mFaces[ idx ];
      for ( unsigned int j = 0; j < face.mNumIndices; ++j )
      {
         indices.push_back( face.mIndices[ j ] );
      }
   }

   // Process material
   if ( mesh->mMaterialIndex >= 0 )
   {
      aiMaterial* material = scene->mMaterials[ mesh->mMaterialIndex ];
      std::vector<Texture> diffuseMaps = LoadMaterialTextures( material,
                                                               aiTextureType_DIFFUSE,
                                                               "texture_diffuse" );
      textures.insert( textures.end( ), diffuseMaps.begin( ), diffuseMaps.end( ) );

      std::vector<Texture> specularMaps = LoadMaterialTextures( material,
                                                                aiTextureType_SPECULAR,
                                                                "texture_specular" );
      textures.insert( textures.end( ), specularMaps.begin( ), specularMaps.end( ) );
   }

   return Mesh( vertices, indices, textures );
}

std::vector<Texture> Model::LoadMaterialTextures( aiMaterial* mat, aiTextureType type,
                                                  const std::string& typeName )
{
   std::vector<Texture> textures;
   for ( unsigned int idx = 0; idx < mat->GetTextureCount( type ); ++idx )
   {
      aiString str;
      mat->GetTexture( type, idx, &str );
      bool skip = false;
      for ( unsigned int j = 0; j < m_loadedTextures.size( ); ++j )
      {
         if ( std::strcmp( m_loadedTextures[ j ].path.data, str.C_Str( ) ) == 0 )
         {
            textures.push_back( m_loadedTextures[ j ] );
            skip = true;
         }
      }

      if ( !skip )
      {
         Texture texture;
         texture.id = TextureFromFile( str.C_Str( ), m_directory );
         texture.type = typeName;
         texture.path = str;
         textures.push_back( texture );
         m_loadedTextures.push_back( texture );
      }
   }

   return textures;
}