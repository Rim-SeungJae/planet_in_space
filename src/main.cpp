#include "cgmath.h"		// slee's simple math library
#include "cgut.h"		// slee's OpenGL utility
#include "circle.h"		// circle class definition

//*************************************
// global constants
static const char*	window_name = "cgbase - A2";
static const char*	vert_shader_path = "../bin/shaders/circ.vert";
static const char*	frag_shader_path = "../bin/shaders/circ.frag";
static const uint	MIN_TESS = 3;		// minimum tessellation factor (down to a triangle)
static const uint	MAX_TESS = 256;		// maximum tessellation factor (up to 256 triangles)
uint				NUM_TESS = 7;		// initial tessellation factor of the circle as a polygon

//*************************************
// window objects
GLFWwindow*	window = nullptr;
ivec2		window_size = ivec2(1280, 720); // initial window size

//*************************************
// OpenGL objects
GLuint	program = 0;		// ID holder for GPU program
GLuint	vertex_array = 0;	// ID holder for vertex array object

//*************************************
// global variables
int		frame = 0;						// index of rendering frames
float	t = 0.0f;						// current simulation parameter
float	dt = 0.0f;
float	dx = 0.0f;
float	dy = 0.0f;
dvec2	pos;
bool	b_time = false;
bool	b_solid_color = false;			// use circle's color?
bool	b_index_buffer = true;	// use index buffering?
int		color_type = 0;
bool	b_rotate = true;
bool	b_lmouse = false;
bool	b_drag = false;
#ifndef GL_ES_VERSION_2_0
bool	b_wireframe = false;
#endif
auto	circles = std::move(create_circles());
struct { bool add=false, sub=false; operator bool() const { return add||sub; } } b; // flags of keys for smooth changes

//*************************************
// holder of vertices and indices of a unit circle
std::vector<vertex>	unit_circle_vertices;	// host-side vertices

//*************************************
void update()
{
	dt = float(glfwGetTime()) * 0.4f - t;
	if (!b_time||!b_rotate)
	{
		dt = 0;
		b_time = true;
	}
	// update global simulation parameter
	t = float(glfwGetTime())*0.4f;

	// tricky aspect correction matrix for non-square window
	float aspect = window_size.x/float(window_size.y);
	mat4 aspect_matrix = 
	{
		1, 0, 0, 0,
		0, std::min(1 / aspect,1.0f), 0, 0,
		0, 0, std::min(aspect,1.0f), 0,
		0, 0, 0, 1
	};

	mat4 view_projection_matrix = { 0,1,0,0,0,0,1,0,-1,0,0,1,0,0,0,1 };

	// update common uniform variables in vertex/fragment shaders
	GLint uloc;
	uloc = glGetUniformLocation( program, "b_solid_color" );	if(uloc>-1) glUniform1i( uloc, b_solid_color );
	uloc = glGetUniformLocation(program, "color_type");	if (uloc > -1) glUniform1i(uloc, color_type);
	uloc = glGetUniformLocation( program, "aspect_matrix" );	if(uloc>-1) glUniformMatrix4fv( uloc, 1, GL_TRUE, aspect_matrix );
	uloc = glGetUniformLocation(program, "view_projection_matrix");
	glUniformMatrix4fv(uloc, 1, GL_TRUE, view_projection_matrix);
}

void render()
{
	// clear screen (with background color) and clear depth buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// notify GL that we use our own program
	glUseProgram( program );

	// bind vertex array object
	glBindVertexArray( vertex_array );

	// render two circles: trigger shader program to process vertex data
	for( auto& c : circles )
	{
		if (b_lmouse && !b_rotate) {
			dvec2 cur_pos;
			glfwGetCursorPos(window, &cur_pos.x, &cur_pos.y);
			dx = float(cur_pos.x) - float(pos.x);
			dy = float(cur_pos.y) - float(pos.y);
			if (!b_drag)
			{
				dx = 0;
				dy = 0;
				b_drag = true;
			}
			pos.x = cur_pos.x;
			pos.y = cur_pos.y;
		}
		// per-circle update
		if (b_rotate||!b_lmouse)c.update(dt);
		else c.drag(dx, dy);

		// update per-circle uniforms
		GLint uloc;
		uloc = glGetUniformLocation( program, "solid_color" );		if(uloc>-1) glUniform4fv( uloc, 1, c.color );	// pointer version
		uloc = glGetUniformLocation( program, "model_matrix" );		if(uloc>-1) glUniformMatrix4fv( uloc, 1, GL_TRUE, c.model_matrix );

		// per-circle draw calls
		if(b_index_buffer)	glDrawElements( GL_TRIANGLES, 35*72*6, GL_UNSIGNED_INT, nullptr );
		else				glDrawArrays( GL_TRIANGLES, 0, NUM_TESS * 3 ); // NUM_TESS = N
	}

	// swap front and back buffers, and display to screen
	glfwSwapBuffers( window );
}

void reshape( GLFWwindow* window, int width, int height )
{
	// set current viewport in pixels (win_x, win_y, win_width, win_height)
	// viewport: the window area that are affected by rendering 
	window_size = ivec2(width,height);
	glViewport( 0, 0, width, height );
}

void print_help()
{
	printf( "[help]\n" );
	printf( "- press ESC or 'q' to terminate the program\n" );
	printf( "- press F1 or 'h' to see help\n" );
	printf( "- press 'd' to change color type\n" );
	//printf( "- press '+/-' to increase/decrease tessellation factor (min=%d, max=%d)\n", MIN_TESS, MAX_TESS );
	//printf( "- press 'i' to toggle between index buffering and simple vertex buffering\n" );
#ifndef GL_ES_VERSION_2_0
	printf( "- press 'w' to toggle wireframe\n" );
#endif
	printf( "\n" );
}

std::vector<vertex> create_circle_vertices( uint N )
{
	std::vector<vertex> v = {};
	for( uint i=0; i <= 36; i++ )
	{
		float theta = PI * i / 36.0f, c_theta = cos(theta), s_theta = sin(theta);
		for (uint j = 0;j <= 72;j++)
		{
			float phi = PI * 2.0f * j / 72.0f, c_phi = cos(phi), s_phi = sin(phi);
			v.push_back({ vec3(s_theta * c_phi,s_theta * s_phi,c_theta), vec3(s_theta * c_phi,s_theta * s_phi,c_theta), vec2(phi/2.0f/PI,1.0f-theta/PI) });
		}
		//float t=PI*2.0f*k/float(N), c=cos(t), s=sin(t);
	}
	return v;
}

void update_vertex_buffer( const std::vector<vertex>& vertices, uint N )
{
	static GLuint vertex_buffer = 0;	// ID holder for vertex buffer
	static GLuint index_buffer = 0;		// ID holder for index buffer

	// clear and create new buffers
	if(vertex_buffer)	glDeleteBuffers( 1, &vertex_buffer );	vertex_buffer = 0;
	if(index_buffer)	glDeleteBuffers( 1, &index_buffer );	index_buffer = 0;

	// check exceptions
	if(vertices.empty()){ printf("[error] vertices is empty.\n"); return; }

	// create buffers
	if(b_index_buffer)
	{
		std::vector<uint> indices;
		/*
		for( uint k=0; k < N; k++ )
		{
			indices.push_back(0);	// the origin
			indices.push_back(k+1);
			indices.push_back(k+2);
		}
		*/
		for (uint i = 0;i < 36;i++)
		{
			for (uint j = 0;j < 72;j++)
			{
				if (i != 0)
				{
					indices.push_back(i * (72 + 1) + j);
					indices.push_back(i * (72 + 1) + (72 + 1) + j);
					indices.push_back(i * (72 + 1) + 1 + j);
				}
				if (i != 35)
				{
					indices.push_back(i * (72 + 1) + 1 + j);
					indices.push_back(i * (72 + 1) + (72 + 1) + j);
					indices.push_back(i * (72 + 1) + (72 + 1) + 1 + j);
				}
			}
		}

		// generation of vertex buffer: use vertices as it is
		glGenBuffers( 1, &vertex_buffer );
		glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer );
		glBufferData( GL_ARRAY_BUFFER, sizeof(vertex)*vertices.size(), &vertices[0], GL_STATIC_DRAW);

		// geneation of index buffer
		glGenBuffers( 1, &index_buffer );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, index_buffer );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(uint)*indices.size(), &indices[0], GL_STATIC_DRAW );
	}
	else
	{
		std::vector<vertex> v; // triangle vertices
		for( uint k=0; k < N; k++ )
		{
			v.push_back(vertices.front());	// the origin
			v.push_back(vertices[k+1]);
			v.push_back(vertices[k+2]);
		}

		// generation of vertex buffer: use triangle_vertices instead of vertices
		glGenBuffers( 1, &vertex_buffer );
		glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer );
		glBufferData( GL_ARRAY_BUFFER, sizeof(vertex)*v.size(), &v[0], GL_STATIC_DRAW );
	}

	// generate vertex array object, which is mandatory for OpenGL 3.3 and higher
	if(vertex_array) glDeleteVertexArrays(1,&vertex_array);
	vertex_array = cg_create_vertex_array( vertex_buffer, index_buffer );
	if(!vertex_array){ printf("%s(): failed to create vertex aray\n",__func__); return; }
}

void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	if(action==GLFW_PRESS)
	{
		if(key==GLFW_KEY_ESCAPE||key==GLFW_KEY_Q)	glfwSetWindowShouldClose( window, GL_TRUE );
		else if(key==GLFW_KEY_H||key==GLFW_KEY_F1)	print_help();
		/*
		else if(key==GLFW_KEY_KP_ADD||(key==GLFW_KEY_EQUAL&&(mods&GLFW_MOD_SHIFT)))	b.add = true;
		else if(key==GLFW_KEY_KP_SUBTRACT||key==GLFW_KEY_MINUS) b.sub = true;
		else if(key==GLFW_KEY_I)
		{
			b_index_buffer = !b_index_buffer;
			update_vertex_buffer( unit_circle_vertices,NUM_TESS );
			printf( "> using %s buffering\n", b_index_buffer?"index":"vertex" );
		}
		*/
		else if(key==GLFW_KEY_D)
		{
			color_type = (color_type + 1) % 3;
			printf( "> using color type: %d \n", color_type );
		}
#ifndef GL_ES_VERSION_2_0
		else if(key==GLFW_KEY_W)
		{
			b_wireframe = !b_wireframe;
			glPolygonMode( GL_FRONT_AND_BACK, b_wireframe ? GL_LINE:GL_FILL );
			printf( "> using %s mode\n", b_wireframe ? "wireframe" : "solid" );
		}
		else if (key == GLFW_KEY_R)
		{
			b_rotate = !b_rotate;
			printf("> rotate %s\n", b_rotate ? "start" : "stop");
		}
#endif
	}
	else if(action==GLFW_RELEASE)
	{
		if(key==GLFW_KEY_KP_ADD||(key==GLFW_KEY_EQUAL&&(mods&GLFW_MOD_SHIFT)))	b.add = false;
		else if(key==GLFW_KEY_KP_SUBTRACT||key==GLFW_KEY_MINUS) b.sub = false;
	}
}

void mouse( GLFWwindow* window, int button, int action, int mods )
{
	if(button==GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
		{
			b_lmouse = true;
		}
		else if (action == GLFW_RELEASE)
		{
			b_lmouse = false;
			b_drag = false;
		}
	}
}

void motion( GLFWwindow* window, double x, double y )
{
}

bool user_init()
{
	// log hotkeys
	print_help();

	// init GL states
	glLineWidth( 1.0f );
	glClearColor( 39/255.0f, 40/255.0f, 34/255.0f, 1.0f );	// set clear color
	glEnable( GL_CULL_FACE );								// turn on backface culling
	glEnable( GL_DEPTH_TEST );								// turn on depth tests
	
	// define the position of four corner vertices
	unit_circle_vertices = std::move(create_circle_vertices( NUM_TESS ));

	// create vertex buffer; called again when index buffering mode is toggled
	update_vertex_buffer( unit_circle_vertices, NUM_TESS );

	return true;
}

void user_finalize()
{
}

int main( int argc, char* argv[] )
{
	// create window and initialize OpenGL extensions
	if(!(window = cg_create_window( window_name, window_size.x, window_size.y ))){ glfwTerminate(); return 1; }
	if(!cg_init_extensions( window )){ glfwTerminate(); return 1; }	// init OpenGL extensions

	// initializations and validations of GLSL program
	if(!(program=cg_create_program_from_string( "layout(location = 0) in vec3 position;\
	layout(location = 1) in vec3 normal;\
	layout(location = 2) in vec2 texcoord;\
\
	out vec3 norm;\
	out vec2 tc;\
\
	uniform mat4	model_matrix;\
	uniform mat4	aspect_matrix;\
	uniform mat4	view_projection_matrix;\
\
	void main()\
	{\
		gl_Position = view_projection_matrix * aspect_matrix * model_matrix * vec4(position, 1);\
\
		norm = normal;\
		tc = texcoord;\
	}","#ifdef GL_ES\
\n	#ifndef GL_FRAGMENT_PRECISION_HIGH\
\n		#define highp mediump\
\n	#endif\
\n	precision highp float;\
\n#endif\
\
\n	in vec2 tc;\
\
	out vec4 fragColor;\
\
	uniform bool b_solid_color;\
	uniform vec4 solid_color;\
	uniform int	color_type;\
\
	void main()\
	{\
		switch (color_type)\
		{\
		case 0:\
			fragColor = vec4(tc.xy, 0, 1);\
			break;\
		case 1:\
			fragColor = vec4(tc.xxx, 1);\
			break;\
		case 2:\
			fragColor = vec4(tc.yyy, 1);\
			break;\
		}\
	}" ))){ glfwTerminate(); return 1; }	// create and compile shaders/program
	if(!user_init()){ printf( "Failed to user_init()\n" ); glfwTerminate(); return 1; }					// user initialization

	// register event callbacks
	glfwSetWindowSizeCallback( window, reshape );	// callback for window resizing events
    glfwSetKeyCallback( window, keyboard );			// callback for keyboard events
	glfwSetMouseButtonCallback( window, mouse );	// callback for mouse click inputs
	glfwSetCursorPosCallback( window, motion );		// callback for mouse movements

	// enters rendering/event loop
	for( frame=0; !glfwWindowShouldClose(window); frame++ )
	{
		glfwPollEvents();	// polling and processing of events
		update();			// per-frame update
		render();			// per-frame render
	}
	
	// normal termination
	user_finalize();
	cg_destroy_window(window);

	return 0;
}
