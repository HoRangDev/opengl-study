#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <fstream>
#include <sstream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
//#include "stb_image.h"

#include "Shader.h"
#include "Model.h"

class App
{
public:
   App( float width, float height ) :
      m_window( nullptr ), m_lightingShader( nullptr ), m_lampShader( nullptr ),
      m_width( width ), m_height( height ),
      m_deltaTime( 0.0f ), m_lastFrame( 0.0f ),
      m_radLight( 1.2f ), m_nanosuit( nullptr )
   {
      lastX = m_width * 0.5f;
      lastY = m_height * 0.5f;
      yaw = -90.0f;
      pitch = 0.0f;
      isFirstMouse = true;
   }

   ~App( )
   {
      delete m_lightingShader;
      delete m_lampShader;
      delete m_nanosuit;
   }

   static void FrameBufferSizeCallBcak( GLFWwindow* window, int width, int height )
   {
      glViewport( 0, 0, width, height );
   }

   void ProcessInput( GLFWwindow* window )
   {
      constexpr float camSpeed = 2.0f;
      if ( glfwGetKey( window, GLFW_KEY_ESCAPE ) )
      {
         glfwSetWindowShouldClose( window, true );
      }

      if ( glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS )
      {
         m_camPos += ( m_deltaTime * camSpeed * m_camFront );
      }
      if ( glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS )
      {
         m_camPos -= glm::normalize( ( glm::cross( m_camFront, m_camUP ) ) ) * camSpeed * m_deltaTime;
      }
      if ( glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS )
      {
         m_camPos -= ( m_deltaTime * camSpeed * m_camFront );
      }
      if ( glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS )
      {
         m_camPos += glm::normalize( ( glm::cross( m_camFront, m_camUP ) ) ) * camSpeed * m_deltaTime;
      }
   }

   static void MouseCallback( GLFWwindow* window, double xPos, double yPos )
   {
      if ( !isFirstMouse )
      {
         float xOffset = xPos - lastX;
         float yOffset = lastY - yPos; // reversed y-coord (screen space)

         float sensitivity = 0.05f;
         xOffset *= sensitivity;
         yOffset *= sensitivity;

         yaw += xOffset;
         pitch += yOffset;

         if ( pitch > 89.0f )
         {
            pitch = 89.0f;
         }
         else if ( pitch < -89.0f )
         {
            pitch = -89.0f;
         }
      }
      else
      {
         isFirstMouse = false;
      }

      lastX = xPos;
      lastY = yPos;
   }

   static void ScrollCallback( GLFWwindow* window, double xOffset, double yOffset )
   {
      if ( fov >= 1.0f && fov <= 45.0f )
      {
         fov -= yOffset;
      }
      if ( fov <= 1.0f )
      {
         fov = 1.0f;
      }
      if ( fov >= 45.0f )
      {
         fov = 45.0f;
      }
   }

   int Run( )
   {
      if ( !Init( ) )
      {
         return -1;
      }

      glm::vec3 pointLightPositions[ ] = {
         glm::vec3( 0.7f,  0.2f,  2.0f ),
         glm::vec3( 1.3f, -2.3f, -2.0f ),
         glm::vec3( -2.0f,  1.0f, -4.0f ),
         glm::vec3( 0.0f,  0.0f, -3.0f )
      };

      float pointLightIntensity = 1.1f;
      float spotLightIntensity = 1.4f;
      while ( !glfwWindowShouldClose( m_window ) )
      {
         ProcessInput( m_window );

         glClearColor( 0.2f, 0.2f, 0.2f, 1.0f );
         glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

         glm::vec3 camForward;
         camForward.x = cos( glm::radians( pitch ) ) * cos( glm::radians( yaw ) );
         camForward.y = sin( glm::radians( pitch ) );
         camForward.z = cos( glm::radians( pitch ) ) * sin( glm::radians( yaw ) );
         m_camFront = glm::normalize( camForward );

         glm::vec3 lightColor;
         lightColor.x = sin( glfwGetTime( ) * 2.0f );
         lightColor.y = sin( glfwGetTime( ) * 0.7f );
         lightColor.z = sin( glfwGetTime( ) * 1.3f );

         glm::vec3 diffuseColor( 1.0f );
         glm::vec3 ambientColor( 0.05f, 0.05f, 0.05f);

         m_view = glm::lookAt( m_camPos,
                               m_camPos + m_camFront,
                               m_camUP );

         glm::mat4 model;
         m_lightingShader->Use( );
         m_lightingShader->SetMat4f( "view", m_view );
         m_lightingShader->SetMat4f( "projection", m_proj );
         m_lightingShader->SetVec3f( "viewPos", m_camPos );
         m_lightingShader->SetVec3f( "material.ambient", 1.0f, 0.5f, 0.31f );
         m_lightingShader->SetFloat( "material.shininess", 32.0f );

         for ( int count = 0; count < 4; ++count )
         {
            auto countStr = std::to_string( count );
            auto prefix = "pointLights[" + std::to_string( count ) + "]";
            m_lightingShader->SetVec3f( prefix + ".position", pointLightPositions[ count ] );
            m_lightingShader->SetFloat( prefix + ".constant", 1.0f );
            m_lightingShader->SetFloat( prefix + ".linear", 0.09f );
            m_lightingShader->SetFloat( prefix + ".quadratic", 0.032f );
            m_lightingShader->SetVec3f( prefix + ".ambient", ambientColor * pointLightIntensity );
            m_lightingShader->SetVec3f( prefix + ".diffuse", diffuseColor * pointLightIntensity );
            m_lightingShader->SetVec3f( prefix + ".specular", 1.f, 1.f, 1.f );
         }

         m_lightingShader->SetVec3f( "spotLight.position", m_camPos );
         m_lightingShader->SetVec3f( "spotLight.direction", m_camFront );
         m_lightingShader->SetFloat( "spotLight.constant", 1.0f );
         m_lightingShader->SetFloat( "spotLight.linear", 0.09f );
         m_lightingShader->SetFloat( "spotLight.quadratic", 0.032f );
         m_lightingShader->SetVec3f( "spotLight.ambient", ambientColor * spotLightIntensity );
         m_lightingShader->SetVec3f( "spotLight.diffuse", diffuseColor * spotLightIntensity );
         m_lightingShader->SetVec3f( "spotLight.specular", 1.f, 1.f, 1.f );
         m_lightingShader->SetFloat( "spotLight.cutOff", glm::cos( glm::radians( 12.5f ) ) );
         m_lightingShader->SetFloat( "spotLight.outerCutOff", glm::cos( glm::radians( 17.5f ) ) );

         model = glm::mat4( );
         model = glm::scale( model, glm::vec3( 0.1f ) );
         m_lightingShader->SetMat4f( "model", model );
         m_nanosuit->Draw( *m_lightingShader );

         for ( int idx = 0; idx < 4; ++idx )
         {
            model = glm::mat4( );
            model = glm::translate( model, pointLightPositions[ idx ] );
            model = glm::scale( model, glm::vec3( 0.2f ) );
            glBindVertexArray( m_lampVAO );
            m_lampShader->Use( );
            m_lampShader->SetMat4f( "model", model );
            m_lampShader->SetMat4f( "view", m_view );
            m_lampShader->SetMat4f( "projection", m_proj );
            m_lampShader->SetVec3f( "lightColor", diffuseColor * pointLightIntensity );
            glDrawArrays( GL_TRIANGLES, 0, 36 );
         }

         glfwSwapBuffers( m_window );
         glfwPollEvents( );

         float currentTime = glfwGetTime( );
         m_deltaTime = currentTime - m_lastFrame;
         m_lastFrame = currentTime;
      }

      glfwTerminate( );
      return 0;
   }

   bool Init( )
   {
      glfwInit( );
      glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
      glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
      glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

      m_window = glfwCreateWindow( ( int ) m_width, ( int ) m_height, "TEST", nullptr, nullptr );
      if ( m_window == nullptr )
      {
         std::cout << "Failed to init GLFW" << std::endl;
         glfwTerminate( );
         return false;
      }

      glfwMakeContextCurrent( m_window );
      glfwSetInputMode( m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
      glfwSetCursorPosCallback( m_window, App::MouseCallback );
      glfwSetScrollCallback( m_window, App::ScrollCallback );

      if ( !gladLoadGLLoader( ( GLADloadproc ) glfwGetProcAddress ) )
      {
         std::cout << "Failed to init GLAD" << std::endl;
         return false;
      }

      glViewport( 0, 0, m_width, m_height );
      glfwSetFramebufferSizeCallback( m_window, App::FrameBufferSizeCallBcak );

      float vertices[ ] = {
         // positions          // normals           // texture coords
         -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
         -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

         -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
         -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
         -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

         -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
         -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

         -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
         -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
      };

      glGenVertexArrays( 1, &m_lampVAO );

      glGenBuffers( 1, &m_vbo );
      glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
      glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), &vertices, GL_STATIC_DRAW );

      // It is a little bit inefficient because we dont need normal at Lamp
      glBindVertexArray( m_lampVAO );
      glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
      glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( GL_FLOAT ) * 8, ( void* ) 0 );
      glEnableVertexAttribArray( 0 );

      // Load Shader program
      m_lightingShader = new Shader( "../Resources/Shaders/SimpleLightVS.glsl", "../Resources/Shaders/SimpleLightPS.glsl" );
      m_lampShader = new Shader( "../Resources/Shaders/SimpleLightVS.glsl", "../Resources/Shaders/SimpleLampPS.glsl" );

      glEnable( GL_DEPTH_TEST );

      m_proj = glm::perspective( glm::radians( fov ), m_width / m_height, 0.1f, 1000.0f );
      m_camPos = glm::vec3( 0.0f, 0.0f, 3.0f );
      m_camFront = glm::vec3( 0.0f, 0.0f, -1.0f );
      m_camUP = glm::vec3( 0.0f, 1.0f, 0.0f );

      m_lightDir = glm::vec3( -0.2f, -1.0f, -0.3f );

      m_nanosuit = new Model( "../Resources/Models/Nanosuit/nanosuit.obj" );

      return true;
   }

private:
   float m_width;
   float m_height;

   float m_deltaTime;
   float m_lastFrame;

   GLFWwindow* m_window;
   Shader*     m_lightingShader;
   Shader*     m_lampShader;

   unsigned int m_lampVAO;
   unsigned int m_vbo;

   glm::mat4   m_view;
   glm::mat4   m_proj;

   glm::vec3   m_camPos;
   glm::vec3   m_camFront;
   glm::vec3   m_camUP;

   float       m_radLight;
   glm::vec3   m_lightDir;

   static float   lastX;
   static float   lastY;
   static float   yaw;
   static float   pitch;
   static float   fov;
   static bool    isFirstMouse;

   Model* m_nanosuit;

};

float App::lastX = 0.0f;
float App::lastY = 0.0f;
float App::yaw = 0.0f;
float App::pitch = 0.0f;
float App::fov = 45.0f;
bool App::isFirstMouse = true;

int main( )
{
   App app{ 800, 600 };
   return app.Run( );
}