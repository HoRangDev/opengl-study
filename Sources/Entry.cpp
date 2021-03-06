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

void renderScene( const Shader& shader );
void renderCube( );
void renderQuad( );

Camera camera{ glm::vec3( 0.0f, 0.0f, 3.0f ) };
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
float exposure = 0.1f;
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
   glViewport( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );

   Shader shader{ "../Resources/Shaders/Bloom.vs", "../Resources/Shaders/Bloom.fs" };
   Shader lightBoxShader{ "../Resources/Shaders/BloomLightBox.vs", "../Resources/Shaders/BloomLightBox.fs" };
   Shader blurShader{ "../Resources/Shaders/Blur.vs", "../Resources/Shaders/Blur.fs" };
   Shader bloomFinalPass{ "../Resources/Shaders/BloomFinal.vs", "../Resources/Shaders/BloomFinal.fs" };

   unsigned int woodTexture = LoadTexture( "../Resources/Textures/wood.png" );

   unsigned int hdrFBO;
   glGenFramebuffers( 1, &hdrFBO );

   unsigned int colorBuffer[2];
   glGenTextures( 2, colorBuffer );

   glBindFramebuffer( GL_FRAMEBUFFER, hdrFBO );
   for ( int idx = 0; idx < 2; ++idx )
   {
      glBindTexture( GL_TEXTURE_2D, colorBuffer[ idx ] );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
      glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + idx, GL_TEXTURE_2D, colorBuffer[idx], 0 );
   }

   unsigned int attachments[ 2 ] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
   glDrawBuffers( 2, attachments );

   unsigned int rboDepth;
   glGenRenderbuffers( 1, &rboDepth );
   glBindRenderbuffer( GL_RENDERBUFFER, rboDepth );
   glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT );

   glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth );
   if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
   {
      std::cout << "Framebuffer not completed!" << std::endl;
   }
   glBindFramebuffer( GL_FRAMEBUFFER, 0 );

   unsigned int pingpongFBO[ 2 ];
   unsigned int pingpongBuffer[ 2 ];
   glGenFramebuffers( 2, pingpongFBO );
   glGenTextures( 2, pingpongBuffer );
   for ( unsigned int idx = 0; idx < 2; ++idx )
   {
      glBindFramebuffer( GL_FRAMEBUFFER, pingpongFBO[ idx ] );
      glBindTexture( GL_TEXTURE_2D, pingpongBuffer[ idx ] );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
      glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[ idx ], 0 );
      if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
      {
         std::cout << "Framebuffer not complete!" << std::endl;
      }
   }

   shader.Use( );
   shader.SetInt( "diffuseMap", 0 );

   blurShader.Use( );
   blurShader.SetInt( "image", 0 );

   bloomFinalPass.Use( );
   bloomFinalPass.SetInt( "scene", 0 );
   bloomFinalPass.SetInt( "bloomBlur", 1 );
   bloomFinalPass.SetFloat( "exposure", exposure );

   std::vector<glm::vec3> lightPositions{
      glm::vec3{ 10.f, 5.0, -10.0f },
      glm::vec3{ 0.5f, 1.0f, -1.0f },
      glm::vec3 {0.7f, 0.3f, 2.0f }
   };

   std::vector<glm::vec3> lightColors{
      glm::vec3{ 2.5f, 2.5f, 2.5f },
      glm::vec3{ 15.f, 15.f, 15.f },
      glm::vec3{0.2f, 0.3f, 0.5f }
   };

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
      glBindFramebuffer( GL_FRAMEBUFFER, hdrFBO );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      // Render Objects
      shader.Use( );
      glm::mat4 projection = glm::perspective( glm::radians( camera.Zoom ), ( float ) SCREEN_WIDTH / ( float ) SCREEN_HEIGHT, 0.1f, 100.0f );
      glm::mat4 view = camera.GetViewMatrix( );
      shader.SetMat4f( "projection", projection );
      shader.SetMat4f( "view", view );
      shader.SetVec3f( "viewPos", camera.Position );
      for ( int idx = 0; idx < lightPositions.size( ); ++idx )
      {
         shader.SetVec3f( "lights[" + std::to_string( idx ) + "].position", lightPositions[ idx ] );
         shader.SetVec3f( "lights[" + std::to_string( idx ) + "].color", lightColors[ idx ] );
      }

      shader.SetMat4f( "model", glm::scale( glm::mat4( ), glm::vec3( 0.6f ) ) );
      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, woodTexture );
      renderCube( );

      // Render Light Sources
      lightBoxShader.Use( );
      lightBoxShader.SetMat4f( "projection", projection );
      lightBoxShader.SetMat4f( "view", view );
      for ( int idx = 0; idx < lightPositions.size( ); ++idx )
      {
         lightBoxShader.SetVec3f( "lightColor", lightColors[ idx ] );

         glm::mat4 boxModel;
         boxModel = glm::translate( boxModel, lightPositions[ idx ] );
         boxModel = glm::scale( boxModel, glm::vec3( 0.25f ) );
         lightBoxShader.SetMat4f( "model", boxModel );
         renderCube( );
      }
      glBindFramebuffer( GL_FRAMEBUFFER, 0 );

      blurShader.Use( );
      bool horizontal = true;
      bool firstItr = true;
      unsigned int amount = 10;
      for ( unsigned int idx = 0; idx < amount; ++idx )
      {
         glBindFramebuffer( GL_FRAMEBUFFER, pingpongFBO[ horizontal ] );
         blurShader.SetInt( "horizontal", horizontal );
         glActiveTexture( GL_TEXTURE0 );
         glBindTexture( GL_TEXTURE_2D, firstItr ? colorBuffer[ 1 ] : pingpongBuffer[ !horizontal ] );
         renderQuad( );
         horizontal = !horizontal;
         if ( firstItr )
         {
            firstItr = false;
         }
      }
      glBindFramebuffer( GL_FRAMEBUFFER, 0 );

      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      bloomFinalPass.Use( );
      bloomFinalPass.SetFloat( "exposure", exposure );
      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, colorBuffer[0] );
      glActiveTexture( GL_TEXTURE1 );
      glBindTexture( GL_TEXTURE_2D, pingpongFBO[ !horizontal ] );
      renderQuad( );

#pragma endregion

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
   if ( glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS )
   {
      exposure -= 0.1f;
      std::cout << "exposure: " << exposure << std::endl;
   }
   if ( glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS )
   {
      exposure += 0.1f;
      std::cout << "exposure: " << exposure << std::endl;
   }
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
      float quadVertices[ ] = {
         // positions        // texture Coords
         -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
         -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
      };
      // setup plane VAO
      glGenVertexArrays( 1, &quadVAO );
      glGenBuffers( 1, &quadVBO );
      glBindVertexArray( quadVAO );
      glBindBuffer( GL_ARRAY_BUFFER, quadVBO );
      glBufferData( GL_ARRAY_BUFFER, sizeof( quadVertices ), &quadVertices, GL_STATIC_DRAW );
      glEnableVertexAttribArray( 0 );
      glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), ( void* ) 0 );
      glEnableVertexAttribArray( 1 );
      glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), ( void* ) ( 3 * sizeof( float ) ) );
   }
   glBindVertexArray( quadVAO );
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
   glBindVertexArray( 0 );
}


// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube( )
{
   // initialize (if necessary)
   if ( cubeVAO == 0 )
   {
      float vertices[ ] = {
         // back face
         -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
         1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
         -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
         -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
                                                               // front face
                                                               -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                                                               1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
                                                               1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                                                               1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                                                               -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
                                                               -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                                                                                                                     // left face
                                                                                                                     -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                                                                                                                     -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
                                                                                                                     -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                                                                                                                     -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                                                                                                                     -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                                                                                                                     -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                                                                                                                                                                           // right face
                                                                                                                                                                           1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                                                                                                                                                                           1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                                                                                                                                                                           1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
                                                                                                                                                                           1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                                                                                                                                                                           1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                                                                                                                                                                           1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
                                                                                                                                                                                                                                // bottom face
                                                                                                                                                                                                                                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                                                                                                                                                                                                                                1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
                                                                                                                                                                                                                                1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                                                                                                                                                                                                                                1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                                                                                                                                                                                                                                -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                                                                                                                                                                                                                                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                                                                                                                                                                                                                                                                                      // top face
                                                                                                                                                                                                                                                                                      -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                                                                                                                                                                                                                                                                                      1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                                                                                                                                                                                                                                                                                      1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
                                                                                                                                                                                                                                                                                      1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                                                                                                                                                                                                                                                                                      -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                                                                                                                                                                                                                                                                                      -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
      };
      glGenVertexArrays( 1, &cubeVAO );
      glGenBuffers( 1, &cubeVBO );
      // fill buffer
      glBindBuffer( GL_ARRAY_BUFFER, cubeVBO );
      glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );
      // link vertex attributes
      glBindVertexArray( cubeVAO );
      glEnableVertexAttribArray( 0 );
      glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), ( void* ) 0 );
      glEnableVertexAttribArray( 1 );
      glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), ( void* ) ( 3 * sizeof( float ) ) );
      glEnableVertexAttribArray( 2 );
      glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), ( void* ) ( 6 * sizeof( float ) ) );
      glBindBuffer( GL_ARRAY_BUFFER, 0 );
      glBindVertexArray( 0 );
   }
   // render Cube
   glBindVertexArray( cubeVAO );
   glDrawArrays( GL_TRIANGLES, 0, 36 );
   glBindVertexArray( 0 );
}

void renderScene( const Shader& shader )
{
   glm::mat4 model;
   model = glm::rotate( model, glm::radians( ( float ) glfwGetTime( ) * -10.0f ), glm::normalize( glm::vec3( 1.0, 0.0, 1.0 ) ) );
   shader.SetMat4f( "model", model );
   renderCube( );
}