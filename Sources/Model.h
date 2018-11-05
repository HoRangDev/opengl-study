#pragma once
#include <stb_image.h>

#include "Mesh.h"

static unsigned int TextureFromFile( const std::string& textureName, const std::string& directory )
{
   std::string fileName = std::string( textureName );
   fileName = directory + '/' + fileName;

   unsigned int texture;
   glGenTextures( 1, &texture );

   int width, height, nrChannels;
   //stbi_set_flip_vertically_on_load( true );
   unsigned char* data = stbi_load( fileName.c_str( ), &width, &height, &nrChannels, 0 );
   if ( data != nullptr )
   {
      GLenum format;
      switch ( nrChannels )
      {
      case 1:
         format = GL_RED;
         break;

      case 3:
         format = GL_RGB;
         break;

      case 4:
         format = GL_RGBA;
         break;
      }

      glBindTexture( GL_TEXTURE_2D, texture );
      glTexImage2D( GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data );
      glGenerateMipmap( GL_TEXTURE_2D );

      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ); // Up scailing => Use Linear (Mag)
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR ); // Down scailing => Use Nearest filter (Min)
   }
   else
   {
      std::cout << "Failed to load texture from : " << fileName << std::endl;
   }

   stbi_image_free( data );

   return texture;
}

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
   Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

   std::vector<Texture> LoadMaterialTextures(aiMaterial* mat,
      aiTextureType type,
      const std::string& typeName);

private:
   std::vector<Texture> m_loadedTextures;
   std::vector<Mesh> m_meshes;
   std::string       m_directory;

};
