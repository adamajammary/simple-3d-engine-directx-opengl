attribute vec3 VertexNormal;
attribute vec3 VertexPosition;
attribute vec2 VertexTextureCoords;

//uniform mat4 MatrixModel;
//uniform mat4 MatrixView;
//uniform mat4 MatrixProjection;
uniform mat4 MatrixMVP;

void main()
{
    //vec4 worldPosition = (MatrixModel * vec4(VertexPosition, 1.0));
    //gl_Position = (MatrixProjection * MatrixView * worldPosition);

	gl_Position = (MatrixMVP * vec4(VertexPosition, 1.0));
}
