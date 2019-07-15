#version 450
#extension GL_ARB_separate_shader_objects : enable

const int MAX_TEXTURES      = 6;
const int VERTICES_PER_FACE = 3;

layout(triangles)                       in;
layout(triangle_strip, max_vertices=18) out;

layout(binding = 0) uniform MatrixBuffer {
	mat4 Normal;
	mat4 Model;
	mat4 VP[MAX_TEXTURES];
	mat4 MVP;
} mb;

layout(binding = 1) uniform DepthBuffer {
	vec4 LightPosition;
} db;

layout(location = 0) out vec4 FragmentPosition;

void main()
{
	for (int face = 0; face < MAX_TEXTURES; face++)
	{
		int layer = int(db.LightPosition.w);
		gl_Layer  = (layer >= 0 ? (layer * MAX_TEXTURES + face) : face);

		for (int vertex = 0; vertex < VERTICES_PER_FACE; vertex++)
		{
			FragmentPosition = gl_in[vertex].gl_Position;
			gl_Position      = (mb.VP[face] * FragmentPosition);

			EmitVertex();
		}    

		EndPrimitive();
	}
}
