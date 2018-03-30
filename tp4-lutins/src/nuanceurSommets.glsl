#version 410

uniform mat4 matrModel;
uniform mat4 matrVisu;

layout(location=0) in vec4 Vertex;
layout(location=3) in vec4 Color;
in float tempsRestant;
in vec3 vitesse;

out Attribs {
   vec4 couleur;
   float tempsRestant;
   float sensVol;
} AttribsOut;

void main( void )
{
   // transformation standard du sommet
   gl_Position = matrVisu * matrModel * Vertex;

   AttribsOut.tempsRestant = tempsRestant;

   AttribsOut.sensVol = sign(vitesse.x);
   AttribsOut.couleur = Color;
}
