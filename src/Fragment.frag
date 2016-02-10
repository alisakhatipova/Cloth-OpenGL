#version 330

uniform sampler2D LandTexture;
in vec2 fragmentTexCoord;
in vec3 fragmentSunView;
in vec3 fragmentNormal;
out vec4 fragColor;

void main(void)
{	
	float lamb  = max(dot(fragmentNormal, normalize(fragmentSunView)), 0.0);
	float carry = clamp(lamb + 0.5, 0.0, 1.0);
	fragColor = texture(LandTexture, fragmentTexCoord) * carry;
}





