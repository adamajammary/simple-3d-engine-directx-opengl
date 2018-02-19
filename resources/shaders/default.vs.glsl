attribute vec3 VertexNormal;
attribute vec3 VertexPosition;
attribute vec2 VertexTextureCoords;

varying vec3 FragmentNormal;
varying vec4 FragmentPosition;
varying vec2 FragmentTextureCoords;

uniform mat4 MatrixModel;
//uniform mat4 MatrixView;
//uniform mat4 MatrixProjection;
uniform mat4 MatrixMVP;

void main()
{
    //FragmentNormal        = ((mat3)MatrixModel * VertexNormal);
    //FragmentNormal        = vec3(MatrixModel * vec4(VertexNormal, 0.0));

    FragmentNormal        = (MatrixModel * vec4(VertexNormal, 0.0)).xyz;
    FragmentTextureCoords = VertexTextureCoords;
    FragmentPosition      = (MatrixModel * vec4(VertexPosition, 1.0));
    gl_Position           = (MatrixMVP   * vec4(VertexPosition, 1.0));
}
