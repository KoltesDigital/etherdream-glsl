#version 330

in float index;

layout(location=0)out vec2 position;
layout(location=1)out vec3 color;

void main()
{
	float angle=index*.05;
	float radius=abs(sin(angle*.4))*.5;
	position=vec2(cos(angle*1.1),sin(angle))*radius;
	color=vec3(1);
}
