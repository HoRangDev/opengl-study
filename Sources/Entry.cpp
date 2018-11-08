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
   
   // Framebuffer configuration
   unsigned int frameBuffer;
   glGenFramebuffers( 1, &frameBuffer );
   glBindFramebuffer( GL_FRAMEBUFFER, frameBuffer );
   // Texture color attachment
   unsigned int textureColorBuffer;
   glGenTextures( 1, &textureColorBuffer );
   glBindTexture( GL_TEXTURE_2D, textureColorBuffer );
   glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0 );

   unsigned int rbo;
   glGenRenderbuffers( 1, &rbo );
   glBindRenderbuffer( GL_RENDERBUFFER, rbo );
   glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT );
   glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo );
   if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
   {
      std::cout << "Framebuffer is not complete" << std::endl;
   }
   glBindFramebuffer( GL_FRAMEBUFFER, 0 );

   float skyboxVertices[ ] = {
      // positions          
      -1.0f,  1.0f, -1.0f,
      -1.0f, -1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,
      1.0f,  1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f,

      -1.0f, -1.0f,  1.0f,
      -1.0f, -1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f,
      -1.0f,  1.0f,  1.0f,
      -1.0f, -1.0f,  1.0f,

      1.0f, -1.0f, -1.0f,
      1.0f, -1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,
      1.0f,  1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,

      -1.0f, -1.0f,  1.0f,
      -1.0f,  1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,
      1.0f, -1.0f,  1.0f,
      -1.0f, -1.0f,  1.0f,

      -1.0f,  1.0f, -1.0f,
      1.0f,  1.0f, -1.0f,
      1.0f,  1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,
      -1.0f,  1.0f,  1.0f,
      -1.0f,  1.0f, -1.0f,

      -1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f,  1.0f,
      1.0f, -1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f,  1.0f,
      1.0f, -1.0f,  1.0f
   };
   unsigned int skyboxVAO;
   unsigned int skyboxVBO;
   glGenVertexArrays( 1, &skyboxVAO );
   glGenBuffers( 1, &skyboxVBO );
   glBindVertexArray( skyboxVAO );
   glBindBuffer( GL_ARRAY_BUFFER, skyboxVBO );
   glBufferData( GL_ARRAY_BUFFER, sizeof( skyboxVertices ), skyboxVertices, GL_STATIC_DRAW );
   glEnableVertexAttribArray( 0 );
   glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( float ) * 3, ( void* ) 0 );

   float quadVertices[ ] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
                             // positions   // texCoords
      -1.0f,  1.0f,  0.0f, 1.0f,
      -1.0f, -1.0f,  0.0f, 0.0f,
      1.0f, -1.0f,  1.0f, 0.0f,

      -1.0f,  1.0f,  0.0f, 1.0f,
      1.0f, -1.0f,  1.0f, 0.0f,
      1.0f,  1.0f,  1.0f, 1.0f
   };
   unsigned int quadVAO;
   unsigned int quadVBO;
   glGenVertexArrays( 1, &quadVAO );
   glGenBuffers( 1, &quadVBO );
   glBindVertexArray( quadVAO );
   glBindBuffer( GL_ARRAY_BUFFER, quadVBO );
   glBufferData( GL_ARRAY_BUFFER, sizeof( quadVertices ), quadVertices, GL_STATIC_DRAW );
   glEnableVertexAttribArray( 0 );
   glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 4, ( void* ) 0 );
   glEnableVertexAttribArray( 1 );
   glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 4, ( void* ) ( sizeof( float ) * 2 ) );

   Shader lightShader( "../Resources/Shaders/SimpleLightVS.glsl", "../Resources/Shaders/SimpleLightPS.glsl" );
   Shader depthVisualizeShader( "../Resources/Shaders/SimpleLightVS.glsl", "../Resources/Shaders/DepthVisualizePS.glsl" );
   Shader outlineShader( "../Resources/Shaders/OutlineVS.glsl", "../Resources/Shaders/OutlinePS.glsl" );
   Shader lightSourceVisualizeShader( "../Resources/Shaders/SimpleLightVS.glsl", "../Resources/Shaders/SimpleLampPS.glsl" );
   Shader transparencyShader( "../Resources/Shaders/SimpleLightVS.glsl", "../Resources/Shaders/TransparencyPS.glsl" );
   Shader screenShader( "../Resources/Shaders/FrameBufferScreenVS.glsl", "../Resources/Shaders/FrameBufferScreenPS.glsl" );
   Shader skyboxShader( "../Resources/Shaders/SkyboxVS.glsl", "../Resources/Shaders/SkyboxPS.glsl" );
   Model nanosuit( "../Resources/Models/Nanosuit/nanosuit.obj" );
   Model sponza( "../Resources/Models/Sponza/sponza.obj" );
   Model dragon( "../Resources/Models/Dragon/dragon.obj" );
   Model bunny( "../Resources/Models/Bunny/bunny.obj" );
   Model sphere( "../Resources/Models/Sphere/Sphere.obj" );
   Mesh grassMesh = Mesh::CreateQuad( );
   auto grassTexture = LoadTexture( "../Resources/Textures/grass.png" );

   /*std::vector<std::string> skyboxTextures =
   {
      "../Resources/Textures/mp_blizzard/blizzard_rt.tga",
      "../Resources/Textures/mp_blizzard/blizzard_lf.tga",
      "../Resources/Textures/mp_blizzard/blizzard_up.tga",
      "../Resources/Textures/mp_blizzard/blizzard_dn.tga",
      "../Resources/Textures/mp_blizzard/blizzard_ft.tga",
      "../Resources/Textures/mp_blizzard/blizzard_bk.tga"
   };*/
   std::vector<std::string> skyboxTextures =
   {
         "../Resources/Textures/ocean/right.jpg",
         "../Resources/Textures/ocean/left.jpg",
         "../Resources/Textures/ocean/top.jpg",
         "../Resources/Textures/ocean/bottom.jpg",
         "../Resources/Textures/ocean/front.jpg",
         "../Resources/Textures/ocean/back.jpg"
   };
   auto skybox = LoadCubeMap( skyboxTextures );

   Shader* usingShader = &lightShader;
   glm::vec3 pointLightPositions[ ] = {
      glm::vec3( 5.0f,  9.5f,  0.0f ),
      glm::vec3( 5.0f, 1.75f, 0.0f ),
      glm::vec3( -15.0f,  1.75f, 0.0f ),
      glm::vec3( -15.0f,  9.5f, 0.0f )
   };

   glm::vec3 vegetationPos[ ] = {
      glm::vec3( 3.0f, -0.02f, 0.0f ),
      glm::vec3( 2.0f, -0.02f, -1.0f ),
      glm::vec3( 3.2f, -0.02f, 1.0f ),
      glm::vec3( 5.0f, -0.02f, 0.3f ),
   };

   //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

   while ( !glfwWindowShouldClose( window ) )
   {
      float currentFrame = glfwGetTime( );
      deltaTime = currentFrame - lastTime;
      lastTime = currentFrame;

      processInput( window );

      glEnable( GL_DEPTH_TEST );
      glBindFramebuffer( GL_FRAMEBUFFER, frameBuffer );
      glClearColor( 0.05f, 0.05f, 0.05f, 1.0f );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

      glm::mat4 proj = glm::perspective( glm::radians( camera.Zoom ), ( float ) SCREEN_WIDTH / ( float ) SCREEN_HEIGHT, 
                                         0.1f, 100.0f );
      glm::mat4 view = camera.GetViewMatrix( );

      transparencyShader.Use( );
      transparencyShader.SetMat4f( "projection", proj );
      transparencyShader.SetMat4f( "view", view );
      for ( int count = 0; count < 4; ++count )
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

      //sponza.Draw( *usingShader );

      usingShader->SetVec3f( "material.baseColor", glm::vec3( 1.0f ) );
      glm::mat4 dragonWorldMat = glm::translate( glm::mat4( ), glm::vec3( 0.0f, 0.35f, 0.0f ) );
      dragonWorldMat = glm::scale( dragonWorldMat, glm::vec3( 0.2f ) );
      usingShader->SetMat4f( "model", dragonWorldMat );
      dragon.Draw( *usingShader );

      glm::mat4 bunnyWorldMat = glm::translate( glm::mat4( ), glm::vec3( 0.0f, 0.35f, 1.8f ) );
      bunnyWorldMat = glm::scale( bunnyWorldMat, glm::vec3( 0.8f ) );
      usingShader->SetMat4f( "model", bunnyWorldMat );
      bunny.Draw( *usingShader );

      glm::mat4 nanosuitWorldMat = glm::translate( glm::mat4( ), glm::vec3( 2.5f, 0.0f, 0.0f ) );
      nanosuitWorldMat = glm::scale( nanosuitWorldMat, glm::vec3( 0.1f ) );
      usingShader->SetMat4f( "model", nanosuitWorldMat );
      usingShader->SetVec3f( "material.baseColor", glm::vec3( 0.1f ) );
      nanosuit.Draw( *usingShader );

      glDepthFunc( GL_LEQUAL );
      skyboxShader.Use( );
      skyboxShader.SetMat4f( "projection", proj );
      skyboxShader.SetMat4f( "view", glm::mat4( glm::mat3( view ) ) );
      glBindVertexArray( skyboxVAO );
      glBindTexture( GL_TEXTURE_CUBE_MAP, skybox );
      glDrawArrays( GL_TRIANGLES, 0, 36 );
      glDepthFunc( GL_LESS );

      glBindFramebuffer( GL_FRAMEBUFFER, 0 );
      glDisable( GL_DEPTH_TEST );
      glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
      glClear( GL_COLOR_BUFFER_BIT );
      
      screenShader.Use( );
      glBindVertexArray( quadVAO );
      glBindTexture( GL_TEXTURE_2D, textureColorBuffer );
      glDrawArrays( GL_TRIANGLES, 0, 6 );

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