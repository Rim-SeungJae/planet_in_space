#ifdef GL_ES
	#ifndef GL_FRAGMENT_PRECISION_HIGH	// highp may not be defined
		#define highp mediump
	#endif
	precision highp float; // default precision needs to be defined
#endif

// inputs from vertex shader
in vec2 tc;	// used for texture coordinate visualization

// output of the fragment shader
out vec4 fragColor;

// shader's global variables, called the uniform variables
uniform bool b_solid_color;
uniform vec4 solid_color;
uniform int	color_type;

void main()
{
	//fragColor = b_solid_color ? solid_color : vec4(tc.xy,0,1);
	switch(color_type)
	{
		case 0:
			fragColor=vec4(tc.xy,0,1);
			break;
		case 1:
			fragColor=vec4(tc.xxx,1);
			break;
		case 2:
			fragColor=vec4(tc.yyy,1);
			break;
	}
}