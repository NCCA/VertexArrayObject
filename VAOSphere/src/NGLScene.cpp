#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/Transformation.h>
#include <ngl/Texture.h>
#include <ngl/NGLInit.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOFactory.h>
#include <ngl/SimpleVAO.h>
//#include  <cstddef>

NGLScene::NGLScene()
{
  setTitle("Qt5 Simple NGL Demo");
}


NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
}


// a simple structure to hold our vertex data
struct vertData
{
  GLfloat x;  // 0
  GLfloat y;  // 1
  GLfloat z;  // 2
  GLfloat nx; // 3
  GLfloat ny; // 4
  GLfloat nz; // 5
  GLfloat u; // 6
  GLfloat v; // 7
};

void NGLScene::buildVAOSphere()
{
	//  Sphere code based on a function Written by Paul Bourke.
	//  http://astronomy.swin.edu.au/~pbourke/opengl/sphere/
	// first we grab an instance of our VOA class as a TRIANGLE_STRIP
  m_vao=ngl::VAOFactory::createVAO(ngl::simpleVAO,GL_TRIANGLE_STRIP);
	// next we bind it so it's active for setting data
  m_vao->bind();
  // the next part of the code calculates the P,N,UV of the sphere for tri_strips

	float theta1 = 0.0;
	float theta2 = 0.0;
	float theta3 = 0.0;


  float radius=1.0;
  float precision=100;
  // a std::vector to store our verts, remember vector packs contiguously so we can use it
  size_t buffSize = (static_cast<size_t>(precision)/2) * ((static_cast<size_t>(precision)+1)*2);

	std::vector <vertData> data(buffSize);
	// calculate how big our buffer is
	// Disallow a negative number for radius.
	if( radius < 0 )
	{
    radius = -radius;
	}
	// Disallow a negative number for _precision.
	if( precision < 4 )
	{
    precision = 4;
	}
	// now fill in a vertData structure and add to the data list for our sphere
	vertData d;
	unsigned int index=0;
  for( size_t i = 0; i < precision/2; ++i )
	{
		theta1 = i * ngl::TWO_PI / precision - ngl::PI2;
		theta2 = (i + 1) * ngl::TWO_PI / precision - ngl::PI2;

    for( size_t j = 0; j <= precision; ++j )
		{
			theta3 = j * ngl::TWO_PI / precision;

			d.nx = cosf(theta2) * cosf(theta3);
			d.ny = sinf(theta2);
			d.nz = cosf(theta2) * sinf(theta3);
			d.x = radius * d.nx;
			d.y = radius * d.ny;
			d.z = radius * d.nz;

      d.u  = (j/precision);
      d.v  = 2*(i+1)/precision;

			data[index++]=d;

			d.nx = cosf(theta1) * cosf(theta3);
			d.ny = sinf(theta1);
			d.nz = cosf(theta1) * sinf(theta3);
			d.x = radius * d.nx;
			d.y = radius * d.ny;
			d.z = radius * d.nz;

      d.u  = (j/precision);
      d.v  = 2*i/precision;
			data[index++]=d;
		} // end inner loop
	}// end outer loop


	// now we have our data add it to the VAO, we need to tell the VAO the following
	// how much (in bytes) data we are copying
	// a pointer to the first element of data (in this case the address of the first element of the
	// std::vector
  m_vao->setData(ngl::AbstractVAO::VertexData(buffSize*sizeof(vertData),data[0].x));
	// in this case we have packed our data in interleaved format as follows
	// u,v,nx,ny,nz,x,y,z
	// If you look at the shader we have the following attributes being used
	// attribute vec3 inVert; attribute 0
  // attribute vec3 inNormal; attribure 1
  // attribute vec2 inUV; attribute 2
  // so we need to set the vertexAttributePointer so the correct size and type as follows
	// vertex is attribute 0 with x,y,z(3) parts of type GL_FLOAT, our complete packed data is
	// sizeof(vertData) and the offset into the data structure for the first x component is 5 (u,v,nx,ny,nz)..x

  m_vao->setVertexAttributePointer(0,3,GL_FLOAT,sizeof(vertData),0);
  m_vao->setVertexAttributePointer(1,3,GL_FLOAT,sizeof(vertData),3);
  m_vao->setVertexAttributePointer(2,2,GL_FLOAT,sizeof(vertData),6);
  // set the size of the buffer / num verts to draw
  m_vao->setNumIndices(buffSize);
	// finally we have finished for now so time to unbind the VAO
  m_vao->unbind();
}



void NGLScene::resizeGL( int _w, int _h )
{
  m_project=ngl::perspective( 45.0f, static_cast<float>( _w ) / _h, 0.05f, 350.0f );
  m_win.width  = static_cast<int>( _w * devicePixelRatio() );
  m_win.height = static_cast<int>( _h * devicePixelRatio() );
}

void NGLScene::initializeGL()
{
  // we need to initialise the NGL lib which will load all of the OpenGL functions, this must
  // be done once we have a valid GL context but before we call any GL commands. If we dont do
  // this everything will crash
  ngl::NGLInit::initialize();
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0,1,3);
  ngl::Vec3 to(0,0,0);
  ngl::Vec3 up(0,1,0);

  m_view=ngl::lookAt(from,to,up);
	// set the shape using FOV 45 Aspect Ratio based on Width and Height
	// The final two are near and far clipping planes of 0.5 and 10
  m_project=ngl::perspective(45,(float)720.0/576.0,0.001,150);

	ngl::ShaderLib::createShaderProgram("TextureShader");

	ngl::ShaderLib::attachShader("SimpleVertex",ngl::ShaderType::VERTEX);
	ngl::ShaderLib::attachShader("SimpleFragment",ngl::ShaderType::FRAGMENT);
	ngl::ShaderLib::loadShaderSource("SimpleVertex","shaders/TextureVertex.glsl");
	ngl::ShaderLib::loadShaderSource("SimpleFragment","shaders/TextureFragment.glsl");

	ngl::ShaderLib::compileShader("SimpleVertex");
	ngl::ShaderLib::compileShader("SimpleFragment");
	ngl::ShaderLib::attachShaderToProgram("TextureShader","SimpleVertex");
	ngl::ShaderLib::attachShaderToProgram("TextureShader","SimpleFragment");


	ngl::ShaderLib::linkProgramObject("TextureShader");
	ngl::ShaderLib::use("TextureShader");
	// build our VertexArrayObject
	buildVAOSphere();
	// load and set a texture
	ngl::Texture t("textures/earth.tiff");
	t.setTextureGL();

}




void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0,0,m_win.width,m_win.height);
  // Rotation based on the mouse position for our global
  // transform
  ngl::Mat4 rotX;
  ngl::Mat4 rotY;
  // create the rotation matrices
  rotX.rotateX(m_win.spinXFace);
  rotY.rotateY(m_win.spinYFace);
  // multiply the rotations
  m_mouseGlobalTX=rotY*rotX;
  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;


	ngl::ShaderLib::use("TextureShader");
	ngl::Mat4 MVP;
  MVP=m_project*m_view*m_mouseGlobalTX;

	ngl::ShaderLib::setUniform("MVP",MVP);

	// now we bind back our vertex array object and draw
  m_vao->bind();
  m_vao->draw();
	// now we are done so unbind
  m_vao->unbind();

}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseMoveEvent( QMouseEvent* _event )
{
  // note the method buttons() is the button state when event was called
  // that is different from button() which is used to check which button was
  // pressed when the mousePress/Release event is generated
  if ( m_win.rotate && _event->buttons() == Qt::LeftButton )
  {
    int diffx = _event->x() - m_win.origX;
    int diffy = _event->y() - m_win.origY;
    m_win.spinXFace += static_cast<int>( 0.5f * diffy );
    m_win.spinYFace += static_cast<int>( 0.5f * diffx );
    m_win.origX = _event->x();
    m_win.origY = _event->y();
    update();
  }
  // right mouse translate code
  else if ( m_win.translate && _event->buttons() == Qt::RightButton )
  {
    int diffX      = static_cast<int>( _event->x() - m_win.origXPos );
    int diffY      = static_cast<int>( _event->y() - m_win.origYPos );
    m_win.origXPos = _event->x();
    m_win.origYPos = _event->y();
    m_modelPos.m_x += INCREMENT * diffX;
    m_modelPos.m_y -= INCREMENT * diffY;
    update();
  }
}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mousePressEvent( QMouseEvent* _event )
{
  // that method is called when the mouse button is pressed in this case we
  // store the value where the maouse was clicked (x,y) and set the Rotate flag to true
  if ( _event->button() == Qt::LeftButton )
  {
    m_win.origX  = _event->x();
    m_win.origY  = _event->y();
    m_win.rotate = true;
  }
  // right mouse translate mode
  else if ( _event->button() == Qt::RightButton )
  {
    m_win.origXPos  = _event->x();
    m_win.origYPos  = _event->y();
    m_win.translate = true;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseReleaseEvent( QMouseEvent* _event )
{
  // that event is called when the mouse button is released
  // we then set Rotate to false
  if ( _event->button() == Qt::LeftButton )
  {
    m_win.rotate = false;
  }
  // right mouse translate mode
  if ( _event->button() == Qt::RightButton )
  {
    m_win.translate = false;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::wheelEvent( QWheelEvent* _event )
{

  // check the diff of the wheel position (0 means no change)
  if ( _event->delta() > 0 )
  {
    m_modelPos.m_z += ZOOM;
  }
  else if ( _event->delta() < 0 )
  {
    m_modelPos.m_z -= ZOOM;
  }
  update();
}

//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;
  // turn on wirframe rendering
  case Qt::Key_W : glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); break;
  // turn off wire frame
  case Qt::Key_S : glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); break;
  // show full screen
  case Qt::Key_F : showFullScreen(); break;
  // show windowed
  case Qt::Key_N : showNormal(); break;
  default : break;
  }
  // finally update the GLWindow and re-draw
  //if (isExposed())
    update();
}
