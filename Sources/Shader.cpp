#include "Shader.h"

Shader::Shader( const std::string& vertexPath, const std::string& fragmentPath ) 
{
   std::string vertexSrc;
   std::string fragmentSrc;
   std::ifstream vertexStream;
   std::ifstream fragmentStream;

   vertexStream.exceptions( std::ifstream::failbit | std::ifstream::badbit );
   fragmentStream.exceptions( std::ifstream::failbit | std::ifstream::badbit );
   try
   {
      vertexStream.open( vertexPath );
      fragmentStream.open( fragmentPath );

      std::stringstream vertexSS;
      std::stringstream fragSS;

      vertexSS << vertexStream.rdbuf( );
      fragSS << fragmentStream.rdbuf( );

      vertexStream.close( );
      fragmentStream.close( );

      vertexSrc = vertexSS.str( );
      fragmentSrc = fragSS.str( );
   }
   catch ( std::ifstream::failure e )
   {
      std::cout << "Error: File not succesfully readed in shader!" << std::endl;
   }

   const char* vShaderCode = vertexSrc.c_str( );
   const char* fShaderCode = fragmentSrc.c_str( );

   unsigned int vertex;
   unsigned int fragment;
   int success = 0;
   char infoLog[ 512 ];

   vertex = glCreateShader( GL_VERTEX_SHADER );
   glShaderSource( vertex, 1, &vShaderCode, nullptr );
   glCompileShader( vertex );
   glGetShaderiv( vertex, GL_COMPILE_STATUS, &success );
   if ( !success )
   {
      glGetShaderInfoLog( vertex, 512, nullptr, infoLog );
      std::cout << "Vertex shader compilation failed: " << infoLog << std::endl;
   }

   fragment = glCreateShader( GL_FRAGMENT_SHADER );
   glShaderSource( fragment, 1, &fShaderCode, nullptr );
   glCompileShader( fragment );
   glGetShaderiv( fragment, GL_COMPILE_STATUS, &success );
   if ( !success )
   {
      glGetShaderInfoLog( fragment, 512, nullptr, infoLog );
      std::cout << "Fragment shader complilation failed: " << infoLog << std::endl;
   }

   m_id = glCreateProgram( );
   glAttachShader( m_id, vertex );
   glAttachShader( m_id, fragment );
   glLinkProgram( m_id );

   glGetProgramiv( m_id, GL_LINK_STATUS, &success );
   if ( !success )
   {
      glGetProgramInfoLog( m_id, 512, nullptr, infoLog );
      std::cout << "Failed to linking program : " << infoLog << std::endl;
   }

   glDeleteShader( vertex );
   glDeleteShader( fragment );
}

void Shader::Use( )
{
   glUseProgram( m_id );
}

void Shader::SetBool( const std::string& name, bool value ) const
{
   glUniform1i( glGetUniformLocation( m_id, name.c_str( ) ), value );
}

void Shader::SetInt( const std::string& name, int value ) const
{
   glUniform1i( glGetUniformLocation( m_id, name.c_str( ) ), value );
}

void Shader::SetFloat( const std::string& name, float value ) const
{
   glUniform1f( glGetUniformLocation( m_id, name.c_str( ) ), value );
}

void Shader::SetVec3f( const std::string& name, float x, float y, float z ) const
{
   glUniform3f( glGetUniformLocation( m_id, name.c_str( ) ), x, y, z );
}

void Shader::SetVec4f( const std::string& name, float x, float y, float z, float w ) const
{
   glUniform4f( glGetUniformLocation( m_id, name.c_str( ) ), x, y, z, w );
}

void Shader::SetMat4f( const std::string& name, const glm::mat4& mat ) const
{
   glUniformMatrix4fv( glGetUniformLocation( m_id, name.c_str( ) ), 1, GL_FALSE, glm::value_ptr( mat ) );
}