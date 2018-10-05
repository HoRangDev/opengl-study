#include <iostream>
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

   while ( !glfwWindowShouldClose( window ) )
   {
      ProcessInput( window );

      glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
      glClear( GL_COLOR_BUFFER_BIT );

      glfwSwapBuffers( window );
      glfwPollEvents( );
   }

   glfwTerminate( );
   return 0;
}