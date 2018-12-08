#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
   Shader( const std::string& vertexPath, const std::string& fragmentPath );
   Shader( const std::string& vertexPath, const std::string& fargmentPath, const std::string& geometryPath );

   void Use( );

   int GetID( ) const { return m_id; }

   void SetBool( const std::string& name, bool value ) const;
   void SetInt( const std::string& name, int value ) const;
   void SetFloat( const std::string& name, float value ) const;
   void SetVec2f( const std::string& name, float x, float y ) const;
   inline void SetVec2f( const std::string& name, const glm::vec2& vec ) const
   {
      SetVec2f(  name, vec.x, vec.y );
   }
   void SetVec3f( const std::string& name, float x, float y, float z ) const;
   inline void SetVec3f( const std::string& name, const glm::vec3& vec ) const
   {
      SetVec3f( name, vec.x, vec.y, vec.z );
   }
   void SetVec4f( const std::string& name, float x, float y, float z, float w ) const;
   inline void SetVec4f( const std::string& name, const glm::vec4& vec ) const
   {
      SetVec4f( name, vec.x, vec.y, vec.z, vec.w );
   }
   void SetMat4f( const std::string& name, const glm::mat4& mat ) const;

private:
   unsigned int m_id;

};