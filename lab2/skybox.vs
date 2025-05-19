#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
	//������Ϊ��պл���֮�󸲸����еĳ������������塣������Ҫˣ����������Ȼ���������պе���Ȼ�������������ֵ1.0�����ֻҪ�и����������Ȳ��Ծͻ�ʧ�ܣ��������������ǰ����
	//͸�ӳ�����perspective division�����ڶ�����ɫ������֮��ִ�еģ���gl_Position��xyz�������wԪ�ء����Ǵ���Ȳ��Խ̳��˽⵽���������zԪ�ص��ڶ�������ֵ�����������Ϣ�����ǿ��԰����λ�õ�zԪ������Ϊ����wԪ�أ������ͻᵼ��zԪ�ص���1.0�ˣ���Ϊ����͸�ӳ���Ӧ�ú�����zԪ��ת��Ϊw/w = 1.0��
    gl_Position = pos.xyww;
}  