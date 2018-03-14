#version 410

uniform sampler2D laTexture;

in Attribs {
   vec4 couleur;
} AttribsIn;

out vec4 FragColor;

void main( void )
{
   // Mettre un test bidon afin que l'optimisation du compilateur n'élimine l'attribut "couleur".
   // Vous MODIFIEREZ ce test inutile!
   if ( AttribsIn.couleur.r < 0.0 ) discard;

   //FragColor = texture( laTexture, gl_PointCoord );
   FragColor = AttribsIn.couleur;
}
