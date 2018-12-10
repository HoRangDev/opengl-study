#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Shader.h>
#include <Camera.h>
#include <Model.h>

#include <iostream>
const unsigned int SCREEN_WIDTH = 1280;
const unsigned int SCREEN_HEIGHT = 720;

void processInput( GLFWwindow *window );
void framebuffer_size_callback( GLFWwindow* window, int width, int height );
void mouse_callback( GLFWwindow* window, double xpos, double ypos );
void scroll_callback( GLFWwindow* window, double xoffset, double yoffset );
unsigned int LoadTexture( const std::string& path );
unsigned int LoadCubeMap( const std::vector<std::string>& faces );

unsigned int loadTexture( char const * path )
{
   unsigned int textureID;
   glGenTextures( 1, &textureID );

   int width, height, nrComponents;
   unsigned char *data = stbi_load( path, &width, &height, &nrComponents, 0 );
   if ( data )
   {
      GLenum format;
      if ( nrComponents == 1 )
         format = GL_RED;
      else if ( nrComponents == 3 )
         format = GL_RGB;
      else if ( nrComponents == 4 )
         format = GL_RGBA;

      glBindTexture( GL_TEXTURE_2D, textureID );
      glTexImage2D( GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data );
      glGenerateMipmap( GL_TEXTURE_2D );

      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT ); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

      stbi_image_free( data );
   }
   else
   {
      std::cout << "Texture failed to load at path: " << path << std::endl;
      stbi_image_free( data );
   }

   return textureID;
}

void renderScene( const Shader& shader );
void renderCube( );
void renderQuad( );

Camera camera{ glm::vec3( 0.0f, 0.0f, 3.0f ) };
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastTime = 0.0f;

// Meshes
unsigned int planeVAO;

enum RenderPass
{
   Shadow = 0,
   Render,
   EnumMax
};

int main( )
{
   unsigned int frameCount = 0;
   float elasedTime = 0.0;
   constexpr float UnitTime = 1.0f;

   constexpr unsigned int Samples = 4;

   glfwInit( );
   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

   GLFWwindow* window = glfwCreateWindow( SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL", nullptr, nullptr );
   if ( window == NULL )
   {
      std::cout << "Failed to create window" << std::endl;
   }
   glfwMakeContextCurrent( window );
   glfwSetFramebufferSizeCallback( window, framebuffer_size_callback );
   glfwSetCursorPosCallback( window, mouse_callback );
   glfwSetScrollCallback( window, scroll_callback );

   glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );

   if ( !gladLoadGLLoader( ( GLADloadproc ) glfwGetProcAddress ) )
   {
      std::cout << "Failed to init glad" << std::endl;
      return -1;
   }

#pragma region Initialize Region
   glEnable( GL_DEPTH_TEST );

   Shader shader{ "../Resources/Shaders/NormalMapping.vs", "../Resources/Shaders/NormalMapping.fs" };

   unsigned int brickTexture = LoadTexture( "../Resources/Textures/brickwall.jpg" );
   unsigned int normalMap = LoadTexture( "../Resources/Textures/brickwall_normal.jpg" );

   shader.Use( );
   // Bind Diffuse Texture to GL_TEXTURE0
   // Bind Normal Map to GL_TEXTURE1
   shader.SetInt( "diffuseMap", 0 );
   shader.SetInt( "normalMap", 1 );

   glm::vec3 lightPos{ 0.5f, 1.0f, 0.3f };

#pragma endregion

   while ( !glfwWindowShouldClose( window ) )
   {
      float currentFrame = glfwGetTime( );
      deltaTime = currentFrame - lastTime;
      lastTime = currentFrame;
      ++frameCount;
      elasedTime += deltaTime;
      if ( elasedTime >= UnitTime )
      {
         printf( "FPS: %u\n", frameCount );
         frameCount = 0;
         elasedTime = 0.0f;
      }

      processInput( window );

#pragma region Render
      glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

      glViewport( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );
      shader.Use( );
      glm::mat4 projection = glm::perspective( glm::radians( camera.Zoom ), ( float ) SCREEN_WIDTH / ( float ) SCREEN_HEIGHT, 0.1f, 100.0f );
      glm::mat4 view = camera.GetViewMatrix( );
      shader.SetMat4f( "projection", projection );
      shader.SetMat4f( "view", view );

      shader.SetVec3f( "viewPos", camera.Position );
      shader.SetVec3f( "lightPos", lightPos );
      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, brickTexture );
      glActiveTexture( GL_TEXTURE1 );
      glBindTexture( GL_TEXTURE_2D, normalMap );
      renderScene( shader );
#pragma endregion

      lightPos = glm::vec3{ 1.5 * std::cos( lastTime ), 0.0f, 1.5f * std::sin( lastTime ) };

      glfwSwapBuffers( window );
      glfwPollEvents( );
   }

   glfwTerminate( );
   return 0;
}

void processInput( GLFWwindow *window )
{
   if ( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS )
      glfwSetWindowShouldClose( window, true );

   if ( glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS )
      camera.ProcessKeyboard( FORWARD, deltaTime );
   if ( glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS )
      camera.ProcessKeyboard( BACKWARD, deltaTime );
   if ( glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS )
      camera.ProcessKeyboard( LEFT, deltaTime );
   if ( glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS )
      camera.ProcessKeyboard( RIGHT, deltaTime );
}

void framebuffer_size_callback( GLFWwindow* window, int width, int height )
{
   glViewport( 0, 0, width, height );
}

void mouse_callback( GLFWwindow* window, double xpos, double ypos )
{
   if ( firstMouse )
   {
      lastX = xpos;
      lastY = ypos;
      firstMouse = false;
   }

   float xoffset = xpos - lastX;
   float yoffset = lastY - ypos;

   lastX = xpos;
   lastY = ypos;

   camera.ProcessMouseMovement( xoffset, yoffset );
}

void scroll_callback( GLFWwindow* window, double xoffset, double yoffset )
{
   camera.ProcessMouseScroll( yoffset );
}

unsigned int LoadTexture( const std::string& path )
{
   unsigned int texture;
   glGenTextures( 1, &texture );
   int imageWidth, imageHeight;
   int imageChannels;
   stbi_set_flip_vertically_on_load( true );
   unsigned char* imageData = stbi_load( path.c_str( ), &imageWidth, &imageHeight, &imageChannels, 0 );
   //stbi_set_flip_vertically_on_load( false );
   if ( imageData != nullptr )
   {
      GLenum format;
      switch ( imageChannels )
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
      glTexImage2D( GL_TEXTURE_2D, 0, format, imageWidth, imageHeight, 0, format, GL_UNSIGNED_BYTE, imageData );
      glGenerateMipmap( GL_TEXTURE_2D );

      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT ); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   }
   else
   {
      std::cout << "Failed to load " << path << std::endl;
   }
   stbi_image_free( imageData );

   return texture;
}

unsigned int LoadCubeMap( const std::vector<std::string>& faces )
{
   unsigned int textureID;
   glGenTextures( 1, &textureID );
   glBindTexture( GL_TEXTURE_CUBE_MAP, textureID );

   int width;
   int height;
   int nrChannels;
   for ( unsigned int idx = 0; idx < faces.size( ); ++idx )
   {
      unsigned char* data = stbi_load( faces[ idx ].c_str( ), &width, &height, &nrChannels, 0 );
      if ( data != nullptr )
      {
         glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + idx,
                       0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data );
      }
      else
      {
         std::cout << "Faeild to load cubemap texture : " << faces[ idx ] << std::endl;
      }
      stbi_image_free( data );
   }

   glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
   glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
   glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
   glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

   return textureID;
}
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad( )
{
   if ( quadVAO == 0 )
   {
      // positions
      glm::vec3 pos1( -1.0f, 1.0f, 0.0f );
      glm::vec3 pos2( -1.0f, -1.0f, 0.0f );
      glm::vec3 pos3( 1.0f, -1.0f, 0.0f );
      glm::vec3 pos4( 1.0f, 1.0f, 0.0f );
      // texture coordinates
      glm::vec2 uv1( 0.0f, 1.0f );
      glm::vec2 uv2( 0.0f, 0.0f );
      glm::vec2 uv3( 1.0f, 0.0f );
      glm::vec2 uv4( 1.0f, 1.0f );
      // normal vector
      glm::vec3 nm( 0.0f, 0.0f, 1.0f );

      // calculate tangent/bitangent vectors of both triangles
      glm::vec3 tangent1, bitangent1;
      glm::vec3 tangent2, bitangent2;
      // triangle 1
      // ----------
      glm::vec3 edge1 = pos2 - pos1;
      glm::vec3 edge2 = pos3 - pos1;
      glm::vec2 deltaUV1 = uv2 - uv1;
      glm::vec2 deltaUV2 = uv3 - uv1;

      GLfloat f = 1.0f / ( deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y );

      tangent1.x = f * ( deltaUV2.y * edge1.x - deltaUV1.y * edge2.x );
      tangent1.y = f * ( deltaUV2.y * edge1.y - deltaUV1.y * edge2.y );
      tangent1.z = f * ( deltaUV2.y * edge1.z - deltaUV1.y * edge2.z );
      tangent1 = glm::normalize( tangent1 );

      bitangent1.x = f * ( -deltaUV2.x * edge1.x + deltaUV1.x * edge2.x );
      bitangent1.y = f * ( -deltaUV2.x * edge1.y + deltaUV1.x * edge2.y );
      bitangent1.z = f * ( -deltaUV2.x * edge1.z + deltaUV1.x * edge2.z );
      bitangent1 = glm::normalize( bitangent1 );

      // triangle 2
      // ----------
      edge1 = pos3 - pos1;
      edge2 = pos4 - pos1;
      deltaUV1 = uv3 - uv1;
      deltaUV2 = uv4 - uv1;

      f = 1.0f / ( deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y );

      tangent2.x = f * ( deltaUV2.y * edge1.x - deltaUV1.y * edge2.x );
      tangent2.y = f * ( deltaUV2.y * edge1.y - deltaUV1.y * edge2.y );
      tangent2.z = f * ( deltaUV2.y * edge1.z - deltaUV1.y * edge2.z );
      tangent2 = glm::normalize( tangent2 );


      bitangent2.x = f * ( -deltaUV2.x * edge1.x + deltaUV1.x * edge2.x );
      bitangent2.y = f * ( -deltaUV2.x * edge1.y + deltaUV1.x * edge2.y );
      bitangent2.z = f * ( -deltaUV2.x * edge1.z + deltaUV1.x * edge2.z );
      bitangent2 = glm::normalize( bitangent2 );


      float quadVertices[ ] = {
         // positions            // normal         // texcoords  // tangent                          // bitangent
         pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
         pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
         pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

         pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
         pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
         pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
      };
      // configure plane VAO
      glGenVertexArrays( 1, &quadVAO );
      glGenBuffers( 1, &quadVBO );
      glBindVertexArray( quadVAO );
      glBindBuffer( GL_ARRAY_BUFFER, quadVBO );
      glBufferData( GL_ARRAY_BUFFER, sizeof( quadVertices ), &quadVertices, GL_STATIC_DRAW );
      glEnableVertexAttribArray( 0 );
      glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof( float ), ( void* ) 0 );
      glEnableVertexAttribArray( 1 );
      glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof( float ), ( void* ) ( 3 * sizeof( float ) ) );
      glEnableVertexAttribArray( 2 );
      glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof( float ), ( void* ) ( 6 * sizeof( float ) ) );
      glEnableVertexAttribArray( 3 );
      glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof( float ), ( void* ) ( 8 * sizeof( float ) ) );
      glEnableVertexAttribArray( 4 );
      glVertexAttribPointer( 4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof( float ), ( void* ) ( 11 * sizeof( float ) ) );
   }
   glBindVertexArray( quadVAO );
   glDrawArrays( GL_TRIANGLES, 0, 6 );
   glBindVertexArray( 0 );
}

void renderScene( const Shader& shader )
{
   glm::mat4 model;
   model = glm::rotate( model, glm::radians( ( float ) glfwGetTime( ) * -10.0f ), glm::normalize( glm::vec3( 1.0, 0.0, 1.0 ) ) );
   shader.SetMat4f( "model", model );
   renderQuad( );
}