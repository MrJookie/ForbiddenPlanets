#version 130

in vec2 texCoord;

uniform isampler2D tex1;
uniform sampler2D tex2;

out vec4 color;

void main() {
	ivec2 indxs = ivec2(texCoord) >> 5;
	ivec2 diff = (ivec2(texCoord) - (indxs << 5));
	
	int r = texelFetch( tex1, indxs, 0 ).r;
	int y = (r >> 6);
	int x = r - (y << 6);
	color = texelFetch( tex2, ivec2( (x << 5) + diff[0], (y << 5) + diff[1] ), 0 );
	
	//color = vec4(1,0,0,1);
	//~ //color = texture(tex2, texCoord);
	//~ color = texture(tex2, texCoord.x + texCoord.y * 2000);
	//~ //color = vec4(0, texCoord, 1);
}
