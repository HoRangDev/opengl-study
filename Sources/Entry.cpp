#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

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

   float vertices[ ] = {
      0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
      0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
      -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
      -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f
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
   glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( GL_FLOAT ) * 6, ( void* ) 0 );
   glEnableVertexAttribArray( 0 );

   glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( GL_FLOAT ) * 6, ( void* ) ( 3 * sizeof( float ) ) );
   glEnableVertexAttribArray( 1 );

   unsigned int EBO{ 0 };
   glGenBuffers( 1, &EBO );
   glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, EBO );
   glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indices ), indices, GL_STATIC_DRAW );

   std::ifstream inputStream{ "../Resources/Shaders/BasicVS.glsl" };
   std::stringstream ss{ };
   ss << inputStream.rdbuf( );
   std::string shaderSrc{ };
   shaderSrc = ss.str( );
   auto vertexShaderSrc = shaderSrc.c_str( );

   unsigned int vertexShader{ 0 };
   vertexShader = glCreateShader( GL_VERTEX_SHADER );
   glShaderSource( vertexShader, 1, &vertexShaderSrc, nullptr );
   glCompileShader( vertexShader );
   inputStream.clear( );
   inputStream.close( );

   int success{ 0 };
   char infoLog[ 512 ];
   glGetShaderiv( vertexShader, GL_COMPILE_STATUS, &success );
   if ( !success )
   {
      glGetShaderInfoLog( vertexShader, 512, nullptr, infoLog );
      std::cout << "Error::Shader::Vertex:Compilation\n " << infoLog << std::endl;
   }

   inputStream.open( "../Resources/Shaders/BasicPS.glsl" );
   ss.str( "" );
   ss.clear( );
   ss << inputStream.rdbuf( );
   shaderSrc = ss.str( );

   auto fragmentShaderSrc = shaderSrc.c_str( );
   unsigned int fragmentShader{ 0 };
   fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
   glShaderSource( fragmentShader, 1, &fragmentShaderSrc, nullptr );
   glCompileShader( fragmentShader );
   inputStream.close( );

   unsigned int shaderProgram{ 0 };
   shaderProgram = glCreateProgram( );
   glAttachShader( shaderProgram, vertexShader );
   glAttachShader( shaderProgram, fragmentShader );
   glLinkProgram( shaderProgram );

   glDeleteShader( vertexShader );
   glDeleteShader( fragmentShader );

   while ( !glfwWindowShouldClose( window ) )
   {
      ProcessInput( window );

      glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
      glClear( GL_COLOR_BUFFER_BIT );

      glUseProgram( shaderProgram );

      float timeValue = glfwGetTime( );
      float greenValue = ( sin( timeValue ) / 2.0f ) + 0.5f;
      int vertexColorLocation = glGetUniformLocation( shaderProgram, "ourColor" );
      //glUniform4f( vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f );

      glBindVertexArray( VAO );
      glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, EBO );
      glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );

      glfwSwapBuffers( window );
      glfwPollEvents( );
   }

   glfwTerminate( );
   return 0;
}