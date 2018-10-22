#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <fstream>
#include <sstream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "stb_image.h"

#include "Shader.h"

void FrameBufferSizeCallBcak( GLFWwindow* window, int width, int height )
{
   glViewport( 0, 0, width, height );
}

void ProcessInput( GLFWwindow* window )
{
   if ( glfwGetKey( window, GLFW_KEY_ESCAPE ) )
   {
      glfwSetWindowShouldClose( window, true );
   }
}

int main( )
{
   glfwInit( );
   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

   GLFWwindow* window = glfwCreateWindow( 800, 600, "TEST", nullptr, nullptr );
   if ( window == nullptr )
   {
      std::cout << "Failed to init GLFW" << std::endl;
      glfwTerminate( );
      return -1;
   }

   glfwMakeContextCurrent( window );

   if ( !gladLoadGLLoader( ( GLADloadproc ) glfwGetProcAddress ) )
   {
      std::cout << "Failed to init GLAD" << std::endl;
      return -1;
   }

   glViewport( 0, 0, 800, 600 );
   glfwSetFramebufferSizeCallback( window, FrameBufferSizeCallBcak );

   // Pos / Color / TexCoord
   float vertices[ ] = {
      0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
      0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
      -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
      -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f
   };

   unsigned int indices[ ] = {
      0, 3, 1,
      1, 2, 3
   };

   unsigned int VAO{ 0 };
   glGenVertexArrays( 1, &VAO );
   glBindVertexArray( VAO );

   unsigned int VBO{ 0 };
   glGenBuffers( 1, &VBO );
   glBindBuffer( GL_ARRAY_BUFFER, VBO );
   glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), &vertices, GL_STATIC_DRAW );

   // location, Number of elements, type, normalize?, stride, offset
   glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( GL_FLOAT ) * 8, ( void* ) 0 );
   glEnableVertexAttribArray( 0 );

   glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( GL_FLOAT ) * 8, ( void* ) ( 3 * sizeof( float ) ) );
   glEnableVertexAttribArray( 1 );

   glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof( GL_FLOAT ) * 8, ( void* ) ( 6 * sizeof( float ) ) );
   glEnableVertexAttribArray( 2 );

   unsigned int EBO{ 0 };
   glGenBuffers( 1, &EBO );
   glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, EBO );
   glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indices ), indices, GL_STATIC_DRAW );

   // Load Shader program
   Shader simpleShader{ "../Resources/Shaders/BasicVS.glsl", "../Resources/Shaders/BasicPS.glsl" };

   unsigned int texture{ 0 };
   glGenTextures( 1, &texture );
   glBindTexture( GL_TEXTURE_2D, texture );

   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ); // Up scailing => Use Linear (Mag)
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); // Down scailing => Use Nearest filter (Min)

                                                                        // Texture Down Scailing => Use Mipmaps! (effective memory usage!)
                                                                        // glGenerateMipmaps..
                                                                        // glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR..NEAREST )


                                                                        // Load Textures
   int width, height, nrChannels;
   unsigned char* data = stbi_load( "../Resources/Textures/wooden_container.jpg", &width, &height, &nrChannels, 0 );
   if ( data != nullptr )
   {
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data );
      glGenerateMipmap( GL_TEXTURE_2D );
   }
   else
   {
      std::cout << "Failed to load texture" << std::endl;
   }
   stbi_image_free( data );

   while ( !glfwWindowShouldClose( window ) )
   {
      ProcessInput( window );

      glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
      glClear( GL_COLOR_BUFFER_BIT );

      simpleShader.Use( );

      float timeValue = glfwGetTime( );
      float greenValue = ( sin( timeValue ) / 2.0f ) + 0.5f;
      simpleShader.SetVec3f( "ourColor", 0.0f, greenValue, 0.0f );
      //int vertexColorLocation = glGetUniformLocation( shaderProgram, "ourColor" );
      //glUniform4f( vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f );

      glBindTexture( GL_TEXTURE_2D, texture );
      glBindVertexArray( VAO );
      glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, EBO );
      glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );

      glfwSwapBuffers( window );
      glfwPollEvents( );
   }

   glfwTerminate( );
   return 0;
}