#version 410
uniform mat4 matrVisu;
// Définition des paramètres des sources de lumière
layout (std140) uniform LightSourceParameters
{
   vec4 ambient;
   vec4 diffuse;
   vec4 specular;
   vec4 position;      // dans le repère du monde
   vec3 spotDirection; // dans le repère du monde
   float spotExponent;
   float spotAngleOuverture; // ([0.0,90.0] ou 180.0)
   float constantAttenuation;
   float linearAttenuation;
   float quadraticAttenuation;
} LightSource[1];

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

layout (std140) uniform varsUnif
{
   // partie 1: illumination
   int typeIllumination;     // 0:Lambert, 1:Gouraud, 2:Phong
   bool utiliseBlinn;        // indique si on veut utiliser modèle spéculaire de Blinn ou Phong
   bool utiliseDirect;       // indique si on utilise un spot style Direct3D ou OpenGL
   bool afficheNormales;     // indique si on utilise les normales comme couleurs (utile pour le débogage)
   // partie 3: texture
   int texnumero;            // numéro de la texture appliquée
   bool utiliseCouleur;      // doit-on utiliser la couleur de base de l'objet en plus de celle de la texture?
   int afficheTexelFonce;    // un texel noir doit-il être affiché 0:noir, 1:mi-coloré, 2:transparent?
};

uniform sampler2D laTexture;

/////////////////////////////////////////////////////////////////

in Attribs {
   vec4 couleur;
   vec3 lightDir;
   vec3 normal;
   vec3 obsVec;
   vec2 texCoord;
} AttribsIn;

out vec4 FragColor;

float calculerSpot( in vec3 spotDir, in vec3 L )
{
   float cosGamma = dot(-L, spotDir);
   float cosDelta = cos(radians(LightSource[0].spotAngleOuverture));
   float c = LightSource[0].spotExponent;
   float fact;
   if(utiliseDirect)
		 fact = smoothstep(pow(cosDelta,1.01+c/2),cosDelta, cosGamma);
   else {
      fact = (cosGamma > cosDelta) ? 
         pow(cosGamma, c):
         0.0;
   }
   return fact;
}

vec4 calculerReflexion( in vec3 L, in vec3 N, in vec3 O )
{
   vec4 ambient = FrontMaterial.emission + FrontMaterial.ambient * LightModel.ambient;
   ambient += FrontMaterial.ambient * LightSource[0].ambient;

   vec4 diffuse = FrontMaterial.diffuse *
		LightSource[0].diffuse *
		max(dot(L, N), 0.0);

	float reflectionFactor;
	if(utiliseBlinn)
		reflectionFactor = max(0.0, dot(normalize(L + O), N));
	else
		reflectionFactor = max(0.0, dot(reflect(-L, N), O));

	vec4 specular = FrontMaterial.specular * LightSource[0].specular * pow(reflectionFactor, FrontMaterial.shininess);
   return clamp(ambient + diffuse + specular, 0.0, 1.0);
}

void main( void )
{
   // ...

   // assigner la couleur finale
   FragColor = ( typeIllumination == 1) ? 
		AttribsIn.couleur :		
		calculerReflexion( normalize(AttribsIn.lightDir), normalize(AttribsIn.normal), normalize(AttribsIn.obsVec) );
   // ...
   vec3 spotDir = transpose(inverse(mat3(matrVisu))) * (LightSource[0].spotDirection);
   FragColor *= calculerSpot(normalize(spotDir), normalize(AttribsIn.lightDir));
   if ( afficheNormales ) FragColor = vec4(normalize(AttribsIn.normal),1.0);
   FragColor = texture(laTexture, AttribsIn.texCoord);
}
