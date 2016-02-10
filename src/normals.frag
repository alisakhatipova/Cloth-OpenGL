#version 330

//uniform sampler2D LandTexture;

in vec3 initnormal;
//in vec2 fragmentTexCoord;
out vec4 fragColor;
void main(void)
{	
  //fragColor = texture(LandTexture, fragmentTexCoord);
   fragColor = vec4(abs(initnormal.x),abs(initnormal.y), abs(initnormal.z), 1);
}





