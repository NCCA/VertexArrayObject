#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/Camera.h>
#include <ngl/Transformation.h>
#include <ngl/Material.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOFactory.h>
#include <ngl/SimpleIndexVAO.h>
#include <array>
//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for x/y translation with mouse movement
//----------------------------------------------------------------------------------------------------------------------
constexpr float INCREMENT=0.01f;
//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for the wheel zoom
//----------------------------------------------------------------------------------------------------------------------
constexpr float ZOOM=0.1f;

NGLScene::NGLScene()
{
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  m_rotate=false;
  // mouse rotation values set to 0
  m_spinXFace=0;
  m_spinYFace=0;
  setTitle("Qt5 SimpleInexVAO created from VAOFactory NGL Demo");
}


NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
  m_vao->removeVAO();
}

void NGLScene::resizeGL(QResizeEvent *_event)
{
  m_width=static_cast<int>(_event->size().width()*devicePixelRatio());
  m_height=static_cast<int>(_event->size().height()*devicePixelRatio());
  // now set the camera size values as the screen size has changed
  m_cam.setShape(45.0f,static_cast<float>(width())/height(),0.05f,350.0f);
}

void NGLScene::resizeGL(int _w , int _h)
{
  m_cam.setShape(45.0f,static_cast<float>(_w)/_h,0.05f,350.0f);
  m_width=static_cast<int>(_w*devicePixelRatio());
  m_height=static_cast<int>(_h*devicePixelRatio());
}


void NGLScene::initializeGL()
{
  // we need to initialise the NGL lib which will load all of the OpenGL functions, this must
  // be done once we have a valid GL context but before we call any GL commands. If we dont do
  // this everything will crash
  ngl::NGLInit::instance();

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0,1,2);
  ngl::Vec3 to(0,0,0);
  ngl::Vec3 up(0,1,0);

  m_cam.set(from,to,up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_cam.setShape(45,720.0f/576.0f,0.001f,150.0f);

  // now to load the shader and set the values
  // grab an instance of shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  // load a frag and vert shaders

  // we are creating a shader called Colour
  shader->createShaderProgram("Colour");
  // now we are going to create empty shaders for Frag and Vert
  shader->attachShader("ColourVertex",ngl::ShaderType::VERTEX);
  shader->attachShader("ColourFragment",ngl::ShaderType::FRAGMENT);
  // attach the source
  shader->loadShaderSource("ColourVertex","shaders/ColourVertex.glsl");
  shader->loadShaderSource("ColourFragment","shaders/ColourFragment.glsl");
  // compile the shaders
  shader->compileShader("ColourVertex");
  shader->compileShader("ColourFragment");
  // add them to the program
  shader->attachShaderToProgram("Colour","ColourVertex");
  shader->attachShaderToProgram("Colour","ColourFragment");

  // now we have associated this data we can link the shader
  shader->linkProgramObject("Colour");
  // and make it active ready to load values
  (*shader)["Colour"]->use();

  buildVAO();
  ngl::VAOFactory::listCreators();
  glViewport(0,0,width(),height());
}


void NGLScene::buildVAO()
{
  // data from here http://rbwhitaker.wikidot.com/index-and-vertex-buffers
  std::array<ngl::Vec3,24> vertAndColour=
  {
   {
   ngl::Vec3(-0.26286500f, 0.0000000f, 0.42532500f), ngl::Vec3(1.0f,0.0f,0.0f),
   ngl::Vec3(0.26286500f, 0.0000000f, 0.42532500f), ngl::Vec3(1.0f,0.55f,0.0f),
   ngl::Vec3(-0.26286500f, 0.0000000f, -0.42532500f),  ngl::Vec3(1.0f,0.0f,1.0f),
   ngl::Vec3(0.26286500f, 0.0000000f, -0.42532500f),  ngl::Vec3(0.0f,1.0f,0.0f),
   ngl::Vec3(0.0000000f, 0.42532500f, 0.26286500f),  ngl::Vec3(0.0f,0.0f,1.0f),
   ngl::Vec3(0.0000000f, 0.42532500f, -0.26286500f),  ngl::Vec3(0.29f,0.51f,0.0f),
   ngl::Vec3(0.0000000f, -0.42532500f, 0.26286500f),  ngl::Vec3(0.5f,0.0f,0.5f),
   ngl::Vec3(0.0000000f, -0.42532500f, -0.26286500f),  ngl::Vec3(1.0f,1.0f,1.0f),
   ngl::Vec3(0.42532500f, 0.26286500f, 0.0000000f),  ngl::Vec3(0.0f,1.0f,1.0f),
   ngl::Vec3(-0.42532500f, 0.26286500f, 0.0000000f),  ngl::Vec3(0.0f,0.0f,0.0f),
   ngl::Vec3(0.42532500f, -0.26286500f, 0.0000000f),  ngl::Vec3(0.12f,0.56f,1.0f),
   ngl::Vec3(-0.42532500f, -0.26286500f, 0.0000000f),  ngl::Vec3(0.86f,0.08f,0.24f)
  }
  };

  std::array<GLshort,60> indices=
  {
    {0, 6,1,0,11, 6,1, 4, 0,1, 8, 4,1,10, 8,2, 5, 3,
    2, 9, 5,2,11, 9,3, 7, 2,3,10, 7,4, 8, 5,4, 9, 0,
    5, 8, 3,5, 9, 4,6,10, 1,6,11, 7,7,10, 6,7,11, 2,
    8,10, 3,9,11, 0}
  };
  // create a vao as a series of GL_TRIANGLES
  m_vao.reset(ngl::VAOFactory::createVAO("simpleIndexVAO",GL_TRIANGLES) );
  m_vao->bind();

  // in this case we are going to set our data as the vertices above

  m_vao->setData(ngl::SimpleIndexVAO::VertexData(
                                                  vertAndColour.size()*sizeof(ngl::Vec3),
                                                  vertAndColour[0].m_x,
                                                  sizeof(indices),&indices[0],
                                                  GL_UNSIGNED_SHORT));
  // data is 24 bytes apart ( two Vec3's) first index
  // is 0 second is 3 floats into the data set (i.e. vec3 offset)
  m_vao->setVertexAttributePointer(0,3,GL_FLOAT,24,0);
  m_vao->setVertexAttributePointer(1,3,GL_FLOAT,24,3);
  m_vao->setNumIndices(indices.size());

 // now unbind
  m_vao->unbind();


}



void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0,0,m_width,m_height);
  // Rotation based on the mouse position for our global
  // transform
  ngl::Mat4 rotX;
  ngl::Mat4 rotY;
  // create the rotation matrices
  rotX.rotateX(m_spinXFace);
  rotY.rotateY(m_spinYFace);
  // multiply the rotations
  m_mouseGlobalTX=rotY*rotX;
  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;

  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["Colour"]->use();


  ngl::Mat4 MVP= m_mouseGlobalTX*m_cam.getVPMatrix();
  shader->setUniform("MVP",MVP);

  m_vao->bind();
  m_vao->draw();
  m_vao->unbind();
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseMoveEvent (QMouseEvent * _event)
{
  // note the method buttons() is the button state when event was called
  // this is different from button() which is used to check which button was
  // pressed when the mousePress/Release event is generated
  if(m_rotate && _event->buttons() == Qt::LeftButton)
  {
    int diffx=_event->x()-m_origX;
    int diffy=_event->y()-m_origY;
    m_spinXFace += (float) 0.5f * diffy;
    m_spinYFace += (float) 0.5f * diffx;
    m_origX = _event->x();
    m_origY = _event->y();
    update();

  }
        // right mouse translate code
  else if(m_translate && _event->buttons() == Qt::RightButton)
  {
    int diffX = (int)(_event->x() - m_origXPos);
    int diffY = (int)(_event->y() - m_origYPos);
    m_origXPos=_event->x();
    m_origYPos=_event->y();
    m_modelPos.m_x += INCREMENT * diffX;
    m_modelPos.m_y -= INCREMENT * diffY;
    update();

   }
}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mousePressEvent ( QMouseEvent * _event)
{
  // this method is called when the mouse button is pressed in this case we
  // store the value where the maouse was clicked (x,y) and set the Rotate flag to true
  if(_event->button() == Qt::LeftButton)
  {
    m_origX = _event->x();
    m_origY = _event->y();
    m_rotate =true;
  }
  // right mouse translate mode
  else if(_event->button() == Qt::RightButton)
  {
    m_origXPos = _event->x();
    m_origYPos = _event->y();
    m_translate=true;
  }

}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseReleaseEvent ( QMouseEvent * _event )
{
  // this event is called when the mouse button is released
  // we then set Rotate to false
  if (_event->button() == Qt::LeftButton)
  {
    m_rotate=false;
  }
        // right mouse translate mode
  if (_event->button() == Qt::RightButton)
  {
    m_translate=false;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::wheelEvent(QWheelEvent *_event)
{

	// check the diff of the wheel position (0 means no change)
	if(_event->delta() > 0)
	{
		m_modelPos.m_z+=ZOOM;
	}
	else if(_event->delta() <0 )
	{
		m_modelPos.m_z-=ZOOM;
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
