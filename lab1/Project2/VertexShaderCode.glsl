#version 430

in layout(location=0) vec3 position;
in layout(location=1) vec3 vertexColor;

uniform mat4 modelTransformMatrix;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 incolor;

out vec3 theColor;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
    theColor = vertexColor * incolor; // 结合顶点颜色和 uniform 颜色
}