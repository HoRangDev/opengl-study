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

Camera camera{ glm::vec3( 0.0f, 0.0f, 3.0f ) };
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastTime = 0.0f;

int main( )
{
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

   glEnable( GL_DEPTH_TEST );
   glEnable( GL_STENCIL_TEST );
   glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );

   Shader lightShader( "../Resources/Shaders/SimpleLightVS.glsl", "../Resources/Shaders/SimpleLightPS.glsl" );
   Shader depthVisualizeShader( "../Resources/Shaders/SimpleLightVS.glsl", "../Resources/Shaders/DepthVisualizePS.glsl" );
   Shader outlineShader( "../Resources/Shaders/OutlineVS.glsl", "../Resources/Shaders/OutlinePS.glsl" );
   Shader lightSourceVisualizeShader( "../Resources/Shaders/SimpleLightVS.glsl", "../Resources/Shaders/SimpleLampPS.glsl" );
   Shader transparencyShader( "../Resources/Shaders/SimpleLightVS.glsl", "../Resources/Shaders/TransparencyPS.glsl" );
   //Model nanosuit( "../Resources/Models/Nanosuit/nanosuit.obj" );
   Model sponza( "../Resources/Models/Sponza/sponza.obj" );
   Model dragon( "../Resources/Models/Dragon/dragon.obj" );
   Model bunny( "../Resources/Models/Bunny/bunny.obj" );
   Model sphere( "../Resources/Models/Sphere/Sphere.obj" );
   Mesh grassMesh = Mesh::CreateQuad( );
   auto grassTexture = LoadTexture( "../Resources/Textures/grass.png" );

   Shader* usingShader = &lightShader;
   glm::vec3 pointLightPositions[ ] = {
      glm::vec3( 5.0f,  9.5f,  0.0f ),
      glm::vec3( 5.0f, 1.75f, 0.0f ),
      glm::vec3( -15.0f,  1.75f, 0.0f ),
      glm::vec3( -15.0f,  9.5f, 0.0f )
   };

   glm::vec3 vegetationPos[ ] = {
      glm::vec3( 3.0f, 1.5f, 0.0f )
   };

   while ( !glfwWindowShouldClose( window ) )
   {
      float currentFrame = glfwGetTime( );
      deltaTime = currentFrame - lastTime;
      lastTime = currentFrame;

      processInput( window );

      glClearColor( 0.05f, 0.05f, 0.05f, 1.0f );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

      glm::mat4 proj = glm::perspective( glm::radians( camera.Zoom ), ( float ) SCREEN_WIDTH / ( float ) SCREEN_HEIGHT, 
                                         0.1f, 100.0f );
      glm::mat4 view = camera.GetViewMatrix( );

      glStencilMask( 0x00 );
      transparencyShader.Use( );
      transparencyShader.SetMat4f( "projection", proj );
      transparencyShader.SetMat4f( "view", view );
      for ( int count = 0; count < 1; ++count )
      {
         glActiveTexture( GL_TEXTURE0 );
         glBindTexture( GL_TEXTURE_2D, grassTexture );
         transparencyShader.SetInt( "texture1", 0 );
         transparencyShader.SetMat4f( "model", glm::translate( glm::mat4(), vegetationPos[count] ));
         grassMesh.Draw( transparencyShader );
      }

      lightSourceVisualizeShader.Use( );
      lightSourceVisualizeShader.SetMat4f( "projection", proj );
      lightSourceVisualizeShader.SetMat4f( "view", view );
      for ( int count = 0; count < 4; ++count )
      {
         lightSourceVisualizeShader.SetMat4f( "model", glm::scale(
            glm::translate( glm::mat4( ), pointLightPositions[ count ] ),
            glm::vec3( 0.01f ) ) );
         lightSourceVisualizeShader.SetVec3f( "lightColor", glm::vec3( 1.0f ) );
         sphere.Draw( lightSourceVisualizeShader );
      }

      usingShader->Use( );
      usingShader->SetMat4f( "projection", proj );
      usingShader->SetMat4f( "view", view );

      //usingShader->SetVec3f( "dirLight.direction", glm::vec3( -0.2f, -1.0f, -0.3f ) );
      //usingShader->SetVec3f( "dirLight.ambient", glm::vec3( 0.2f, 0.2f, 0.2f ) );
      //usingShader->SetVec3f( "dirLight.diffuse", glm::vec3( 0.75f, 0.75f, 0.75f ) );
      //usingShader->SetVec3f( "dirLight.specular", glm::vec3( 0.4f, 0.4f, 0.4f ) );

      for ( int count = 0; count < 4; ++count )
      {
         auto countStr = std::to_string( count );
         auto prefix = "pointLights[" + std::to_string( count ) + "]";
         usingShader->SetVec3f( prefix + ".position", pointLightPositions[ count ] );
         usingShader->SetFloat( prefix + ".constant", 1.0f );
         usingShader->SetFloat( prefix + ".linear", 0.045f );
         usingShader->SetFloat( prefix + ".quadratic", 0.0075f );
         usingShader->SetVec3f( prefix + ".ambient", glm::vec3( 0.0f ) );
         usingShader->SetVec3f( prefix + ".diffuse", glm::vec3( 0.6f ) );
         usingShader->SetVec3f( prefix + ".specular", 1.f, 1.f, 1.f );
      }
      usingShader->SetFloat( "material.shininess", 32.0f );

      glm::mat4 roomWorldMat;
      roomWorldMat = glm::scale( roomWorldMat, glm::vec3( 0.02f ) );
      usingShader->SetMat4f( "model", roomWorldMat );
      usingShader->SetVec3f( "material.baseColor", glm::vec3( 0.0f ) );

      glStencilMask( 0x00 );
      sponza.Draw( *usingShader );

      usingShader->SetVec3f( "material.baseColor", glm::vec3( 1.0f ) );

      glStencilFunc( GL_ALWAYS, 1, 0xFF );
      glStencilMask( 0xFF );
      glm::mat4 dragonWorldMat = glm::translate( glm::mat4( ), glm::vec3( 0.0f, 0.35f, 0.0f ) );
      dragonWorldMat = glm::scale( dragonWorldMat, glm::vec3( 0.2f ) );
      usingShader->SetMat4f( "model", dragonWorldMat );
      dragon.Draw( *usingShader );

      glm::mat4 bunnyWorldMat = glm::translate( glm::mat4( ), glm::vec3( 0.0f, 0.35f, 1.8f ) );
      bunnyWorldMat = glm::scale( bunnyWorldMat, glm::vec3( 0.8f ) );
      usingShader->SetMat4f( "model", bunnyWorldMat );
      bunny.Draw( *usingShader );

      outlineShader.Use( );
      outlineShader.SetVec3f( "outlineColor", glm::vec3( 0.607f, 0.862f, 0.913f ) );
      outlineShader.SetMat4f( "projection", proj );
      outlineShader.SetMat4f( "view", view );

      glStencilFunc( GL_NOTEQUAL, 1, 0xFF );
      glStencilMask( 0x00 );
      outlineShader.SetMat4f( "model", dragonWorldMat );
      dragon.Draw( outlineShader );

      outlineShader.SetMat4f( "model", bunnyWorldMat );
      bunny.Draw( outlineShader );
      glStencilMask( 0xFF ); /// Stencil buffer write enable

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
   stbi_set_flip_vertically_on_load( false );
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
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData );
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