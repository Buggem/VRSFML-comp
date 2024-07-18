#version 150

uniform mat4 sf_u_projectionMatrix;
uniform mat4 sf_u_textureMatrix;
uniform mat4 sf_u_modelViewMatrix;

attribute vec4 sf_a_color;
attribute vec2 sf_a_position;
attribute vec2 sf_a_texCoord;

varying vec4 sf_v_color;
varying vec2 sf_v_texCoord;

// TODO: check local branch on PC, I think this worked

void main()
{
    gl_Position   = sf_u_projectionMatrix * sf_u_modelViewMatrix * vec4(sf_a_position, 0.0, 1.0);
    sf_v_texCoord = (sf_u_textureMatrix * vec4(sf_a_texCoord, 0.0, 1.0)).xy;
    sf_v_color    = sf_a_color;
}
