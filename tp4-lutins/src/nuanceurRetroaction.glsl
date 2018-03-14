#version 410

uniform vec3 bDim, positionPuits;
uniform float temps, dt, tempsMax, gravite;

in vec3 position;
in vec3 vitesse;
in vec4 couleur;
in float tempsRestant;

out vec3 positionMod;
out vec3 vitesseMod;
out vec4 couleurMod;
out float tempsRestantMod;

uint randhash( uint seed ) // entre  0 et UINT_MAX
{
   uint i=(seed^12345391u)*2654435769u;
   i ^= (i<<6u)^(i>>26u);
   i *= 2654435769u;
   i += (i<<5u)^(i>>12u);
   return i;
}
float myrandom( uint seed ) // entre  0 et 1
{
   const float UINT_MAX = 4294967295.0;
   return float(randhash(seed)) / UINT_MAX;
}

void main( void )
{
   // Mettre un test bidon afin que l'optimisation du compilateur n'élimine pas les attributs dt, gravite, tempsMax positionPuits et bDim.
   // Vous ENLEVEREZ cet énoncé inutile!
   if ( dt+gravite+tempsMax+positionPuits.x < -100000 ) tempsRestantMod += .000001;

   if ( tempsRestant <= 0.0 )
   {
      // se préparer à produire une valeur un peu aléatoire
      uint seed = uint(temps * 1000.0) + uint(gl_VertexID);
      // faire renaitre la particule au puits
      //positionMod = ...

      // assigner un vitesse
      vitesseMod = vec3( mix( -0.5, 0.5, myrandom(seed++) ),   // entre -0.5 et 0.5
                         mix( -0.5, 0.5, myrandom(seed++) ),   // entre -0.5 et 0.5
                         mix(  0.5, 1.0, myrandom(seed++) ) ); // entre  0.5 et 1
      //vitesseMod = vec3( -0.8, 0., 0.6 );

      // nouveau temps de vie
      //tempsRestantMod = ...; // entre 0 et tempsMax secondes

      // interpolation linéaire entre COULMIN et COULMAX
      const float COULMIN = 0.2; // valeur minimale d'une composante de couleur lorsque la particule (re)naît
      const float COULMAX = 0.9; // valeur maximale d'une composante de couleur lorsque la particule (re)naît
      //couleurMod = ...
   }
   else
   {
      // avancer la particule
      positionMod = position; // ...
      vitesseMod = vitesse;

      // diminuer son temps de vie
      tempsRestantMod = tempsRestant;

      // garder la couleur courante
      couleurMod = couleur;

      // collision avec la demi-sphère ?
      // ...

      // collision avec le sol ?
      // ...

      // appliquer la gravité
      // ...
   }
}
