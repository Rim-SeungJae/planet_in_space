#pragma once
#ifndef __CIRCLE_H__
#define __CIRCLE_H__

struct circle_t
{
	vec3	center=vec3(0);		// 2D position for translation
	float	radius=1.0f;		// radius
	float	theta=0.0f;			// rotation angle for z axis
	float	phi = 0.0f;			// rotation angle for y axis
	vec4	color;				// RGBA color in [0,1]
	mat4	model_matrix;		// modeling transformation

	// public functions
	void	update( float t );
	void	drag(float dx, float dy);
};

inline std::vector<circle_t> create_circles()
{
	std::vector<circle_t> circles;
	circle_t c;
	
	c = {vec3(0),1.0f,0.0f,0.0f,vec4(1.0f,0.5f,0.5f,1.0f)};
	circles.emplace_back(c);

	return circles;
}

inline void circle_t::update( float dt)
{
	theta += dt;
	float c_theta = cos(theta), s_theta = sin(theta);
	float c_phi = cos(phi), s_phi = sin(phi);

	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix =
	{
		radius, 0, 0, 0,
		0, radius, 0, 0,
		0, 0, radius, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix =
	{
		c_theta,-s_theta, 0, 0,
		s_theta, c_theta, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix2 =
	{
		c_phi, 0, s_phi, 0,
		0, 1, 0, 0,
		-s_phi, 0, c_phi, 0,
		0, 0, 0, 1
	};

	mat4 translate_matrix =
	{
		1, 0, 0, center.x,
		0, 1, 0, center.y,
		0, 0, 1, center.z,
		0, 0, 0, 1
	};
	
	model_matrix = translate_matrix*rotation_matrix*rotation_matrix2*scale_matrix;
}

inline void circle_t::drag(float dx, float dy)
{
	theta += dx*0.01f;
	phi += dy*0.01f;
	float c_theta = cos(theta), s_theta = sin(theta);
	float c_phi = cos(phi), s_phi = sin(phi);

	mat4 rotation_matrix =
	{
		c_theta,-s_theta, 0, 0,
		s_theta, c_theta, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix2 =
	{
		c_phi, 0, s_phi, 0,
		0, 1, 0, 0,
		-s_phi, 0, c_phi, 0,
		0, 0, 0, 1
	};

	model_matrix = rotation_matrix * rotation_matrix2;
}

#endif
