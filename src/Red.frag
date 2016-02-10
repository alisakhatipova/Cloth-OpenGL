#version 330

uniform int g_clothShaderId;
uniform sampler2D diffuseTexture;
uniform vec3 g_sunPos;
uniform vec3 g_sunDir;

in vec3 fragmentNormal;
in vec3 initnormal;
in vec2 fragmentTexCoord;
//in vec3 l;
in vec3 fragmentSunView;
out vec4 fragColor;

void main(void)
{	
   if (g_clothShaderId == 1)
	fragColor = vec4(1,0,0,1);
   else if (g_clothShaderId == 3) {
	vec4 diffcolor = texture(diffuseTexture, fragmentTexCoord);
	//fragColor = max(dot(g_sunDir, normalize(fragmentNormal)) + 2.5, 0.0)* diffcolor/3.5;
	//fragColor = max(dot(l, fragmentNormal), 0.0)* diffcolor;
	float lamb  = max(dot(fragmentNormal, normalize(fragmentSunView)), 0.0);
	float carry = clamp(lamb + 0.5, 0.0, 1.0);
	fragColor = texture(diffuseTexture, fragmentTexCoord) * carry;
	//fragColor = texture(diffuseTexture, fragmentTexCoord);
   }
   else 	
   	fragColor = vec4(abs(initnormal.x),abs(initnormal.y), abs(initnormal.z), 1);
}



