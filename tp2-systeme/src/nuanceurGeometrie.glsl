#version 410

layout(triangles) in;
layout(triangle_strip, max_vertices = 6) out;

in Attribs {
   vec4 couleur;
   float clipDistance;
} AttribsIn[];

out Attribs {
   vec4 couleur;
} AttribsOut;

void main()
{
   for ( int i = 0 ; i < gl_in.length() ; ++i )
   {
      gl_ViewportIndex = 0;
      gl_ClipDistance[0] = -AttribsIn[i].clipDistance;
      gl_Position = gl_in[i].gl_Position;
      AttribsOut.couleur = AttribsIn[i].couleur;
      EmitVertex();
   }
   EndPrimitive();

   for ( int i = 0 ; i < gl_in.length() ; ++i )
   {
      gl_ViewportIndex = 1;
      gl_ClipDistance[0] = AttribsIn[i].clipDistance;
      gl_Position = gl_in[i].gl_Position;
      AttribsOut.couleur = AttribsIn[i].couleur;
      EmitVertex();
   }
   EndPrimitive();
}

