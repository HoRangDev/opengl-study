#include "Model.h"

void Model::Draw(Shader shader)
{
   for (unsigned int idx = 0; idx < m_meshes.size(); ++idx)
   {
      m_meshes[idx].Draw(shader, m_instAmount);
   }
}

void Model::LoadModel(const std::string& path)
{
   Assimp::Importer importer;
   const aiScene* scene = importer.ReadFile( path,
                                             aiProcess_Triangulate |
                                             aiProcess_FlipUVs | aiProcess_GenSmoothNormals );
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

      std::vector<Texture> ambientMaps = LoadMaterialTextures( material,
                                                               aiTextureType_AMBIENT,
                                                               "texture_ambient" );
      textures.insert( textures.end( ), ambientMaps.begin( ), ambientMaps.end( ) );
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

void Model::SetupMeshes( glm::mat4* worldMatrices )
{
   glGenBuffers( 1, &m_instVBO );
   glBindBuffer( GL_ARRAY_BUFFER, m_instVBO );
   glBufferData( GL_ARRAY_BUFFER, m_instAmount * sizeof( glm::mat4 ), worldMatrices, GL_STATIC_DRAW );

   GLsizei vec4Size = sizeof( glm::vec4 );
   for ( unsigned int idx = 0; idx < m_meshes.size( ); ++idx )
   {
      unsigned int meshVAO = m_meshes[ idx ].GetVAO( );
      glBindVertexArray( meshVAO );

      glEnableVertexAttribArray( 3 );
      glVertexAttribPointer( 3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, ( void* ) 0 );
      glEnableVertexAttribArray( 4 );
      glVertexAttribPointer( 4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, ( void* ) (vec4Size) );
      glEnableVertexAttribArray( 5 );
      glVertexAttribPointer( 5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, ( void* ) (vec4Size*2) );
      glEnableVertexAttribArray( 6 );
      glVertexAttribPointer( 6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, ( void* ) (vec4Size*3) );

      glVertexAttribDivisor( 3, 1 );
      glVertexAttribDivisor( 4, 1 );
      glVertexAttribDivisor( 5, 1 );
      glVertexAttribDivisor( 6, 1 );   

      glBindVertexArray( 0 );
   }
}