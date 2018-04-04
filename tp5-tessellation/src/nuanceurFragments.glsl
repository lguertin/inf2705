//#version 410

// Définition des paramètres des sources de lumière
layout (std140) uniform LightSourceParameters
{
   vec4 ambient;
   vec4 diffuse;
   vec4 specular;
   vec4 position[2];
} LightSource;

// Définition des paramètres des matériaux
layout (std140) uniform MaterialParameters
{
   vec4 emission;
   vec4 ambient;
   vec4 diffuse;
   vec4 specular;
   float shininess;
} FrontMaterial;

// Définition des paramètres globaux du modèle de lumière
layout (std140) uniform LightModelParameters
{
   vec4 ambient;       // couleur ambiante
   bool localViewer;   // observateur local ou à l'infini?
   bool twoSide;       // éclairage sur les deux côtés ou un seul?
} LightModel;

uniform sampler2D textureCoul;

// bool gl_FrontFacing  // variable standard d'OpenGL: c'est la face avant qui est actuellement tracée?

in Attribs {
   vec2 texCoord;
   vec3 normale, lumiDir[2], obsVec;
} AttribsIn;

out vec4 FragColor;

bool utiliseBlinn = true;  // on utilise toujours Blinn

void main( void )
{
   vec4 coul = FrontMaterial.emission + FrontMaterial.ambient * LightModel.ambient;

   // vecteur normal
   vec3 N = normalize( gl_FrontFacing ? AttribsIn.normale : -AttribsIn.normale );

   // calculer la contribution de chaque source lumineuse
   for ( int i = 0 ; i < 2 ; ++i )
   {
      // ajouter la contribution de la composante ambiante
      coul += FrontMaterial.ambient * LightSource.ambient;

      // direction de la lumière
      // calcul du vecteur de la surface vers la source lumineuse
      // normaliser le vecteur de la surface vers la source lumineuse
      N = normalize(N);
      vec3 L = normalize(AttribsIn.lumiDir[i]);

      // produit scalaire pour le calcul de la réflexion diffuse
      // normale . direction de la lumière
      float NdotL = dot(N, L);

      // calcul de l'éclairage seulement si le produit scalaire est positif
      if ( NdotL > 0.0 )
      {
         // ajouter la contribution de la composante diffuse
#if ( INDICEDIFFUSE == 0 )
         // la composante diffuse (kd) du matériel est utilisé
         coul += FrontMaterial.diffuse * LightSource.diffuse * NdotL;
#else
         // la composante diffuse (kd) provient de la texture 'textureCoul'
         //coul += ...
#endif

         // ajouter la contribution de la composante spéculaire
         // produit scalaire pour la réflexion spéculaire (selon Blinn)
         //...
         vec3 O = normalize(AttribsIn.obsVec);
         coul += FrontMaterial.specular * LightSource.specular * 
            pow(max(0.0, dot(normalize(L + O), N)), FrontMaterial.shininess);
      }
   }

   // assigner la couleur finale
   FragColor = clamp( coul, 0.0, 1.0 );

#if ( AFFICHENORMALES == 1 )
   // pour le débogage
   FragColor = vec4(N,1.0);
#endif
}
