#version 410

layout(points) in;
layout(points, max_vertices = 1) out;

in Attribs {
   vec4 couleur;
   float tempsRestant;
   //float sens; // du vol
} AttribsIn[];

out Attribs {
   vec4 couleur;
} AttribsOut;

void main()
{
   gl_PointSize = 5.0; // en pixels
   gl_Position = gl_in[0].gl_Position;
   AttribsOut.couleur = AttribsIn[0].couleur;
   EmitVertex();
}
