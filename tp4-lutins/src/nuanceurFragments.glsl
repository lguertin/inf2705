#version 410

uniform sampler2D laTexture;
uniform int texnumero;

in Attribs {
   vec4 couleur;
   vec2 texCoord;
} AttribsIn;

out vec4 FragColor;

void main( void )
{
   FragColor = AttribsIn.couleur;

   if (texnumero != 0) {
      vec4 texel = texture( laTexture, AttribsIn.texCoord );
      if (texel.a < 0.1) discard;
      FragColor.rgb = mix( FragColor.rgb, texel.rgb, 0.7 );
   }
}
