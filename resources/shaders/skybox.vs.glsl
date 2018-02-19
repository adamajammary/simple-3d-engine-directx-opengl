attribute vec3 VertexNormal;
attribute vec3 VertexPosition;
attribute vec2 VertexTextureCoords;

//varying vec3 FragmentNormal;
//varying vec4 FragmentPosition;
varying vec3 FragmentTextureCoords;

uniform mat4 MatrixModel;
uniform mat4 MatrixView;
uniform mat4 MatrixProjection;
uniform mat4 MatrixMVP;

void main()
{
    //vec4 worldPosition    = (MatrixModel * vec4(VertexPosition, 1.0));
    //gl_Position           = (MatrixProjection * mat4(mat3(MatrixView)) * vec4(VertexPosition, 1.0));
	//gl_Position           = vec4(MatrixProjection * mat4(mat3(MatrixView)) * vec4(VertexPosition, 1.0)).xyww;

	FragmentTextureCoords = VertexPosition;
	gl_Position           = vec4(MatrixMVP * vec4(VertexPosition, 1.0)).xyww;
}
