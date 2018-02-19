attribute vec3 VertexNormal;
attribute vec3 VertexPosition;
attribute vec2 VertexTextureCoords;

varying vec4 ClipSpace;
varying vec4 FragmentPosition;
varying vec2 FragmentTextureCoords;
//varying vec3 FromVertexToCamera;
//varying vec3 FromLightToVertex;

// uniform vec3  CameraPosition;
//uniform float ClipHeight;

uniform mat4 MatrixModel;
//uniform mat4 MatrixView;
//uniform mat4 MatrixProjection;
uniform mat4 MatrixMVP;

void main()
{
    //vec4 worldPosition = (MatrixModel * vec4(VertexPosition, 1.0));
    //FragmentPosition      = worldPosition;
    //fragmentTextureCoords = vec2(vertexPosition.x * 0.5 + 0.5, vertexPosition.y * 0.5 + 0.5);
    //fromVertexToCamera    = (cameraPosition - worldPosition.xyz);
    //ClipSpace             = (matrixProjection * matrixView * worldPosition);

    FragmentTextureCoords = VertexTextureCoords;
    FragmentPosition      = (MatrixModel * vec4(VertexPosition, 1.0));
    ClipSpace             = (MatrixMVP   * vec4(VertexPosition, 1.0));

    gl_Position = ClipSpace;
}
