#version 410

const float M_PI = 3.14159265358979323846;	// pi
const float M_PI_2 = 1.57079632679489661923;	// pi/2

uniform mat4 matrModel;
uniform mat4 matrVisu;
uniform mat4 matrProj;

uniform vec4 planCoupe; // équation du plan de coupe

layout(location=0) in vec4 Vertex;
layout(location=3) in vec4 Color;

out Attribs {
   vec4 couleur;
   // float clipDistance;
} AttribsOut;

void main( void )
{
   // transformation standard du sommet
   gl_Position = matrProj * matrVisu * matrModel * Vertex;

   // couleur du sommet
   AttribsOut.couleur = Color;
}
