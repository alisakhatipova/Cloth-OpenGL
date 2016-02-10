#version 330

out vec4 fragColor;

void main(void)
{	
  fragColor = vec4(1,0,0,1);
}



	fragColor = (min(max(dot(g_sunDir, normalize(fragmentNormal)), 0.0) + 0.25, 1))* diffcolor/(1 + 0.25);