#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D sprite;
uniform vec3 spriteColor;
uniform vec2 player_pos;
in vec4 frag_position;
uniform int light;


void main()
{
    float dist = length(frag_position-vec4(player_pos,0.0f,1.0f));
    if(dist>100.0f && light==1)
    {
        color = vec4(0.0f);
    }else{
    color =vec4(spriteColor, 1.0) * texture(sprite, TexCoords);
    }
}