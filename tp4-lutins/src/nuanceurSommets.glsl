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
   //float sens; // du vol
} AttribsOut;

void main( void )
{
   // transformation standard du sommet
   gl_Position = matrVisu * matrModel * Vertex;

   AttribsOut.tempsRestant = tempsRestant;

   // couleur du sommet
   AttribsOut.couleur = Color;

   // À SUPPRIMER: les lignes suivantes servent seulement à forcer le compilateur à conserver cet attribut
   if ( tempsRestant < 0.0 ) AttribsOut.couleur.rgb += 0.00001*vitesse;
   if ( tempsRestant < 0.0 ) AttribsOut.couleur.a += 0.00001;
}
