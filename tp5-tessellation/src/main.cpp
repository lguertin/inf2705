// Prénoms, noms et matricule des membres de l'équipe:
// - Léandre Guertin
// - Issifath Sanni 1771817
//#warning "Écrire les prénoms, noms et matricule des membres de l'équipe ci-dessus et commenter cette ligne"

#include <iostream>
#include "inf2705-matrice.h"
#include "inf2705-nuanceur.h"
#include "inf2705-fenetre.h"
#include "inf2705-texture.h"
#include "inf2705-forme.h"

// variables pour l'utilisation des nuanceurs
GLuint prog;      // votre programme de nuanceurs
GLint locVertex = -1;
GLint locmatrModel = -1;
GLint locmatrVisu = -1;
GLint locmatrProj = -1;
GLint locmatrNormale = -1;
GLint loctextureDepl = -1;
GLint loctextureCoul = -1;
GLint locfacteurDeform = -1;
GLint locbDim = -1;
GLint locTessLevelInner = -1;
GLint locTessLevelOuter = -1;
GLuint indLightSource = -1;
GLuint indFrontMaterial = -1;
GLuint indLightModel = -1;
GLuint progBase;  // le programme de nuanceurs de base
GLint locVertexBase = -1;
GLint locColorBase = -1;
GLint locmatrModelBase = -1;
GLint locmatrVisuBase = -1;
GLint locmatrProjBase = -1;

GLuint vao[2];
GLuint vbo[3];
GLuint ubo[3];

// matrices du pipeline graphique
MatricePipeline matrProj, matrVisu, matrModel;

// les formes
FormeCube *cubeFil = NULL;
FormeSphere *sphereLumi = NULL;

//
// variables d'état
//
struct Etat
{
public:
   bool enPerspective;     // indique si on est en mode Perspective (true) ou Ortho (false)
   bool enmouvement;       // le modèle est en mouvement/rotation automatique ou non
   bool afficheAxes;       // indique si on affiche les axes
   GLenum modePolygone;    // comment afficher les polygones
   GLfloat TessLevelInner; // niveau de tessellation
   GLfloat TessLevelOuter; // niveau de tessellation
   int curLumi;            // la source lumineuse courante (celle qui peut être déplacée)
   bool positionnelle;     // les sources de lumière sont de type positionnelle?
   bool afficheNormales;   // indique si on utilise les normales comme couleurs (utile pour le débogage)
   glm::vec4 bDim;         // les dimensions de la boite en x,y,z
   int indiceFonction;     // indice de la fonction à afficher
   int indiceTexture;      // indice de la texture à utiliser pour le déplacement
   int indiceDiffuse;      // indice de la texture à utiliser pour la couleur
   float facteurDeform;    // facteur de déformation de la surface
   float temps;            // le temps courant (en secondes)
   const float dt;         // intervalle entre chaque affichage (en secondes)
} etat = { false, false, true, GL_FILL, 40, 40, 0, true, false, glm::vec4( 2.0, 2.0, 2.0, 1.0 ), 1, 0, 0, 1.0, 0.0, 1.0/60.0 };

GLuint textures[9];              // les textures chargées

//
// définition des lumières
//
glm::vec4 posLumiInit[2] = { glm::vec4( -1.0, -2.0, 1.6, 1.0 ), glm::vec4(  1.0, -2.0, 1.6, 1.0 ) };
struct LightSourceParameters
{
   glm::vec4 ambient;
   glm::vec4 diffuse;
   glm::vec4 specular;
   glm::vec4 position[2];
} LightSource = { glm::vec4( 0.2, 0.2, 0.2, 1.0 ),
                  glm::vec4( 0.45, 0.45, 0.45, 1.0 ),
                  glm::vec4( 0.6, 0.6, 0.6, 1.0 ),
                  { posLumiInit[0], posLumiInit[1] } };

//
// définition du matériau
//
struct MaterialParameters
{
   glm::vec4 emission;
   glm::vec4 ambient;
   glm::vec4 diffuse;
   glm::vec4 specular;
   float shininess;
} FrontMaterial = { glm::vec4( 0.0, 0.0, 0.0, 1.0 ),
                    glm::vec4( 0.2, 0.2, 0.2, 1.0 ),
                    glm::vec4( 1.0, 1.0, 1.0, 1.0 ),
                    glm::vec4( 1.0, 1.0, 1.0, 1.0 ),
                    500.0 };

struct LightModelParameters
{
   glm::vec4 ambient; // couleur ambiante
   int localViewer;   // doit-on prendre en compte la position de l'observateur? (local ou à l'infini)
   int twoSide;       // éclairage sur les deux côtés ou un seul?
} LightModel = { glm::vec4(0,0,0,1), true, false };

struct VueStereo
{
   int affichageStereo; // type d'affichage: mono, stéréo anaglyphe, stéréo double
   GLdouble dip;       // la distance interpupillaire
   GLdouble factzoom;  // le facteur de zoom
   GLdouble zavant;    // la position du plan avant du volume de visualisation
   GLdouble zarriere;  // la position du plan arrière du volume de visualisation
   GLdouble zecran;    // la position du plan de l'écran: les objets affichés en avant de ce plan «sortiront» de l'écran
   GLint modele;       // le modèle à afficher
};
struct VueStereo vue = { 0, 0.80, 1.0, 4.0, 20.0, 10.0, 1 };

//
// variables pour définir le point de vue
//
class Camera
{
public:
   void definir()
   {
      matrVisu.LookAt( dist*cos(glm::radians(theta))*sin(glm::radians(phi)),
                       dist*sin(glm::radians(theta))*sin(glm::radians(phi)),
                       dist*cos(glm::radians(phi)),
                       0.0, 0.0, 0.0,
                       0.0, 0.0, 1.0 );
   }
   void verifierAngles() // vérifier que les angles ne débordent pas les valeurs permises
   {
      if ( theta > 360.0 ) theta -= 360.0; else if ( theta < 0.0 ) theta += 360.0;
      const GLdouble MINPHI = 0.01, MAXPHI = 180.0 - 0.01;
      phi = glm::clamp( phi, MINPHI, MAXPHI );
   }
   double theta;         // angle de rotation de la caméra (coord. sphériques)
   double phi;           // angle de rotation de la caméra (coord. sphériques)
   double dist;          // distance (coord. sphériques)
} camera = { -90.0, 80.0, 10.0 };

void calculerPhysique( )
{
   if ( etat.enmouvement )
   {
      // avancer le temps
      etat.temps += etat.dt;

#if 1
      // faire varier la fonction ou la texture utilisée
      if ( fmod( etat.temps+etat.dt, 10 ) <= etat.dt )
      {
         void chargerNuanceurs();
         if ( etat.indiceFonction ) { if ( ++etat.indiceFonction > 6 ) etat.indiceFonction = 1; chargerNuanceurs(); }
         if ( etat.indiceTexture ) { if ( ++etat.indiceTexture > 9 ) etat.indiceTexture = 1; etat.indiceDiffuse = etat.indiceTexture; chargerNuanceurs(); }

      }
#endif

#if 1
      // faire varier la déformation
      static int sensZ = +1;
      etat.facteurDeform += 0.01 * sensZ;
      if ( etat.facteurDeform < 0.1 ) sensZ = +1.0;
      else if ( etat.facteurDeform > 1.0 ) sensZ = -1.0;
#endif

#if 0
      // faire varier le point de vue
      camera.theta += 0.2;
      // static int sensPhi = 1;
      // camera.phi += 0.7 * sensPhi;
      // const GLdouble MPHI = 70.0;
      // if ( camera.phi < MPHI || camera.phi > 180.0-MPHI ) sensPhi = -sensPhi;
      camera.phi = 75.0 + 20.0 * sin(etat.temps); // variation lisse
      camera.verifierAngles();
#endif

#if 1
      // faire varier la taille de la boite
      static int sensX = 1;
      etat.bDim.x += 0.005 * sensX;
      if ( etat.bDim.x < 1.2 ) sensX = +1;
      else if ( etat.bDim.x > 4.0 ) sensX = -1;

      static int sensY = 1;
      etat.bDim.y += 0.001 * sensY;
      if ( etat.bDim.y < 1.7 ) sensY = +1;
      else if ( etat.bDim.y > 2.3 ) sensY = -1;
#endif

#if 0
      static int sensrotZ = +1;
      deltaCam += 0.1 * sensrotZ;
      if ( deltaCam < -10.0 ) sensrotZ = +1.0;
      else if ( deltaCam > +10.0 ) sensrotZ = -1.0;
#endif
   }
}

void charger1Texture( std::string fichier, GLuint &texture )
{
   unsigned char *pixels;
   GLsizei largeur, hauteur;
   if ( ( pixels = ChargerImage( fichier, largeur, hauteur ) ) != NULL )
   {
      glGenTextures( 1, &texture );
      glBindTexture( GL_TEXTURE_2D, texture );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, largeur, hauteur, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glGenerateMipmap( GL_TEXTURE_2D );
      glBindTexture( GL_TEXTURE_2D, 0 );
      delete[] pixels;
   }
}
void chargerTextures()
{
   charger1Texture( "textures/texture1.bmp", textures[0] );
   charger1Texture( "textures/texture2.bmp", textures[1] );
   charger1Texture( "textures/texture3.bmp", textures[2] );
   charger1Texture( "textures/texture4.bmp", textures[3] );
   charger1Texture( "textures/texture5.bmp", textures[4] );
   charger1Texture( "textures/texture6.bmp", textures[5] );
   charger1Texture( "textures/texture7.bmp", textures[6] );
   charger1Texture( "textures/texture8.bmp", textures[7] );
   charger1Texture( "textures/texture9.bmp", textures[8] );
}

void chargerNuanceurs()
{
   {
      // créer le programme
      progBase = glCreateProgram();

      // attacher le nuanceur de sommets
      {
         GLuint nuanceurObj = glCreateShader( GL_VERTEX_SHADER );
         glShaderSource( nuanceurObj, 1, &ProgNuanceur::chainesSommetsMinimal, NULL );
         glCompileShader( nuanceurObj );
         glAttachShader( progBase, nuanceurObj );
         ProgNuanceur::afficherLogCompile( nuanceurObj );
      }
      // attacher le nuanceur de fragments
      {
         GLuint nuanceurObj = glCreateShader( GL_FRAGMENT_SHADER );
         glShaderSource( nuanceurObj, 1, &ProgNuanceur::chainesFragmentsMinimal, NULL );
         glCompileShader( nuanceurObj );
         glAttachShader( progBase, nuanceurObj );
         ProgNuanceur::afficherLogCompile( nuanceurObj );
      }

      // faire l'édition des liens du programme
      glLinkProgram( progBase );
      ProgNuanceur::afficherLogLink( progBase );

      // demander la "Location" des variables
      if ( ( locVertexBase = glGetAttribLocation( progBase, "Vertex" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Vertex" << std::endl;
      if ( ( locColorBase = glGetAttribLocation( progBase, "Color" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Color" << std::endl;
      if ( ( locmatrModelBase = glGetUniformLocation( progBase, "matrModel" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrModel" << std::endl;
      if ( ( locmatrVisuBase = glGetUniformLocation( progBase, "matrVisu" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrVisu" << std::endl;
      if ( ( locmatrProjBase = glGetUniformLocation( progBase, "matrProj" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrProj" << std::endl;
   }

   {
      // charger le nuanceur de ce TP
      std::ostringstream preambule; // ce préambule sera ajouté avant le contenu du fichier du nuanceur
      preambule << "#version 410" << std::endl
                << "#define INDICEFONCTION " << etat.indiceFonction << std::endl
                << "#define INDICETEXTURE " << etat.indiceTexture << std::endl
                << "#define INDICEDIFFUSE " << etat.indiceDiffuse << std::endl
                << "#define AFFICHENORMALES " << etat.afficheNormales << std::endl;
      std::string preambulestr = preambule.str();
      const char *preambulechar = preambulestr.c_str();

      // créer le programme
      prog = glCreateProgram();

      // attacher le nuanceur de sommets
      const GLchar *chainesSommets[2] = { preambulechar, ProgNuanceur::lireNuanceur( "nuanceurSommets.glsl" ) };
      if ( chainesSommets[1] != NULL )
      {
         GLuint nuanceurObj = glCreateShader( GL_VERTEX_SHADER );
         glShaderSource( nuanceurObj, 2, chainesSommets, NULL );
         glCompileShader( nuanceurObj );
         glAttachShader( prog, nuanceurObj );
         ProgNuanceur::afficherLogCompile( nuanceurObj );
         delete [] chainesSommets[1];
      }
//#if 0
      // partie 1: À ACTIVER (enlever le #if 0 et le #endif)
      // attacher le nuanceur de controle de la tessellation
      const GLchar *chainesTessCtrl[2] = { preambulechar, ProgNuanceur::lireNuanceur( "nuanceurTessCtrl.glsl" ) };
      if ( chainesTessCtrl[1] != NULL )
      {
         GLuint nuanceurObj = glCreateShader( GL_TESS_CONTROL_SHADER );
         glShaderSource( nuanceurObj, 2, chainesTessCtrl, NULL );
         glCompileShader( nuanceurObj );
         glAttachShader( prog, nuanceurObj );
         ProgNuanceur::afficherLogCompile( nuanceurObj );
         delete [] chainesTessCtrl[1];
      }
      // attacher le nuanceur d'évaluation de la tessellation
      const GLchar *chainesTessEval[2] = { preambulechar, ProgNuanceur::lireNuanceur( "nuanceurTessEval.glsl" ) };
      if ( chainesTessEval[1] != NULL )
      {
         GLuint nuanceurObj = glCreateShader( GL_TESS_EVALUATION_SHADER );
         glShaderSource( nuanceurObj, 2, chainesTessEval, NULL );
         glCompileShader( nuanceurObj );
         glAttachShader( prog, nuanceurObj );
         ProgNuanceur::afficherLogCompile( nuanceurObj );
         delete [] chainesTessEval[1];
      }
//#endif
      // attacher le nuanceur de géometrie
      const GLchar *chainesGeometrie[2] = { preambulechar, ProgNuanceur::lireNuanceur( "nuanceurGeometrie.glsl" ) };
      if ( chainesGeometrie[1] != NULL )
      {
         GLuint nuanceurObj = glCreateShader( GL_GEOMETRY_SHADER );
         glShaderSource( nuanceurObj, 2, chainesGeometrie, NULL );
         glCompileShader( nuanceurObj );
         glAttachShader( prog, nuanceurObj );
         ProgNuanceur::afficherLogCompile( nuanceurObj );
         delete [] chainesGeometrie[1];
      }
      // attacher le nuanceur de fragments
      const GLchar *chainesFragments[2] = { preambulechar, ProgNuanceur::lireNuanceur( "nuanceurFragments.glsl" ) };
      if ( chainesFragments[1] != NULL )
      {
         GLuint nuanceurObj = glCreateShader( GL_FRAGMENT_SHADER );
         glShaderSource( nuanceurObj, 2, chainesFragments, NULL );
         glCompileShader( nuanceurObj );
         glAttachShader( prog, nuanceurObj );
         ProgNuanceur::afficherLogCompile( nuanceurObj );
         delete [] chainesFragments[1];
      }
      // faire l'édition des liens du programme
      glLinkProgram( prog );
      ProgNuanceur::afficherLogLink( prog );

      // demander la "Location" des variables
      if ( ( locVertex = glGetAttribLocation( prog, "Vertex" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Vertex" << std::endl;
      if ( ( locmatrModel = glGetUniformLocation( prog, "matrModel" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrModel" << std::endl;
      if ( ( locmatrVisu = glGetUniformLocation( prog, "matrVisu" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrVisu" << std::endl;
      if ( ( locmatrProj = glGetUniformLocation( prog, "matrProj" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrProj" << std::endl;
      if ( ( locmatrNormale = glGetUniformLocation( prog, "matrNormale" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrNormale" << std::endl;
      // if ( ( loctextureDepl = glGetUniformLocation( prog, "textureDepl" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de textureDepl" << std::endl;
      // if ( ( loctextureCoul = glGetUniformLocation( prog, "textureCoul" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de textureCoul" << std::endl;
      loctextureDepl = glGetUniformLocation( prog, "textureDepl" );
      loctextureCoul = glGetUniformLocation( prog, "textureCoul" );
      if ( ( locfacteurDeform = glGetUniformLocation( prog, "facteurDeform" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de facteurDeform" << std::endl;
      if ( ( locbDim = glGetUniformLocation( prog, "bDim" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de bDim" << std::endl;
      if ( ( locTessLevelInner = glGetUniformLocation( prog, "TessLevelInner" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de TessLevelInner" << std::endl;
      if ( ( locTessLevelOuter = glGetUniformLocation( prog, "TessLevelOuter" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de TessLevelOuter" << std::endl;
      if ( ( indLightSource = glGetUniformBlockIndex( prog, "LightSourceParameters" ) ) == GL_INVALID_INDEX ) std::cerr << "!!! pas trouvé l'\"index\" de LightSource" << std::endl;
      if ( ( indFrontMaterial = glGetUniformBlockIndex( prog, "MaterialParameters" ) ) == GL_INVALID_INDEX ) std::cerr << "!!! pas trouvé l'\"index\" de FrontMaterial" << std::endl;
      if ( ( indLightModel = glGetUniformBlockIndex( prog, "LightModelParameters" ) ) == GL_INVALID_INDEX ) std::cerr << "!!! pas trouvé l'\"index\" de LightModel" << std::endl;

      // charger les ubo
      {
         glBindBuffer( GL_UNIFORM_BUFFER, ubo[0] );
         glBufferData( GL_UNIFORM_BUFFER, sizeof(LightSource), &LightSource, GL_DYNAMIC_COPY );
         glBindBuffer( GL_UNIFORM_BUFFER, 0 );
         const GLuint bindingIndex = 0;
         glBindBufferBase( GL_UNIFORM_BUFFER, bindingIndex, ubo[0] );
         glUniformBlockBinding( prog, indLightSource, bindingIndex );
      }
      {
         glBindBuffer( GL_UNIFORM_BUFFER, ubo[1] );
         glBufferData( GL_UNIFORM_BUFFER, sizeof(FrontMaterial), &FrontMaterial, GL_DYNAMIC_COPY );
         glBindBuffer( GL_UNIFORM_BUFFER, 0 );
         const GLuint bindingIndex = 3;
         glBindBufferBase( GL_UNIFORM_BUFFER, bindingIndex, ubo[1] );
         glUniformBlockBinding( prog, indFrontMaterial, bindingIndex );
      }
      {
         glBindBuffer( GL_UNIFORM_BUFFER, ubo[2] );
         glBufferData( GL_UNIFORM_BUFFER, sizeof(LightModel), &LightModel, GL_DYNAMIC_COPY );
         glBindBuffer( GL_UNIFORM_BUFFER, 0 );
         const GLuint bindingIndex = 4;
         glBindBufferBase( GL_UNIFORM_BUFFER, bindingIndex, ubo[2] );
         glUniformBlockBinding( prog, indLightModel, bindingIndex );
      }
   }
}

void FenetreTP::initialiser()
{
   // donner la couleur de fond
   glClearColor( 0.2, 0.2, 0.2, 1.0 );

   // activer les etats openGL
   glEnable( GL_DEPTH_TEST );

   // allouer les UBO pour les variables uniformes
   glGenBuffers( 3, ubo );

   // charger les nuanceurs
   chargerNuanceurs();

   // charger les textures
   chargerTextures();

   // créer le VAO pour conserver les informations
   glGenVertexArrays( 2, vao ); // les VAOs: un pour la surface, l'autre pour les décorations
   glGenBuffers( 3, vbo ); // les VBO pour différents tableaux

   // la surface
   glBindVertexArray( vao[0] );
   {
      // les coordonnées
      const GLfloat sommets[] = { 0.0, 1.0,
                                  1.0, 1.0,
                                  1.0, 0.0,
                                  0.0, 0.0 };
      const GLuint connec[] = { 0, 1, 2, 2, 3, 0 };
      // créer le VBO pour les sommets
      glBindBuffer( GL_ARRAY_BUFFER, vbo[0] );
      glBufferData( GL_ARRAY_BUFFER, sizeof(sommets), sommets, GL_STATIC_DRAW );
      // faire le lien avec l'attribut du nuanceur de sommets
      glVertexAttribPointer( locVertex, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );
      glEnableVertexAttribArray(locVertex);
      // charger le VBO pour la connectivité
      glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbo[1] );
      glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(connec), connec, GL_STATIC_DRAW );
   }

   // les décorations
   glBindVertexArray( vao[1] );
   {
      // créer le VBO pour les sommets
      glBindBuffer( GL_ARRAY_BUFFER, vbo[2] );

      GLfloat coords[] = { 1., 0., 0., 0., 0., 0. };
      glBufferData( GL_ARRAY_BUFFER, sizeof(coords), coords, GL_STATIC_DRAW );

      // faire le lien avec l'attribut du nuanceur de sommets
      glVertexAttribPointer( locVertexBase, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
      glEnableVertexAttribArray(locVertexBase);
   }

   glBindVertexArray( 0 );

   // créer quelques autres formes
   glUseProgram( progBase );
   cubeFil = new FormeCube( 2.0, false );
   sphereLumi = new FormeSphere( 0.1, 10, 10 );
}

void FenetreTP::conclure()
{
   glDeleteBuffers( 3, vbo );
   glDeleteVertexArrays( 2, vao );
   glDeleteBuffers( 3, ubo );
   delete cubeFil;
   delete sphereLumi;
}

void definirProjection( int OeilMult, int w, int h ) // 0: mono, -1: oeil gauche, +1: oeil droit
{
   // partie 2: utiliser plutôt Frustum() pour le stéréo
   matrProj.Perspective( 35.0, (GLdouble) w / (GLdouble) h, vue.zavant, vue.zarriere );
}

void afficherDecoration()
{
   // remettre le programme de base pour le reste des décorations
   glUseProgram( progBase );
   glUniformMatrix4fv( locmatrProjBase, 1, GL_FALSE, matrProj ); // donner la projection courante

   // dessiner le cube englobant
   glVertexAttrib3f( locColorBase, 1.0, 1.0, 1.0 ); // blanc
   matrModel.PushMatrix(); {
      matrModel.Scale( etat.bDim.x, etat.bDim.y, etat.bDim.z );
      glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
      cubeFil->afficher();
   } matrModel.PopMatrix();
   glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );

   for ( int i = 0 ; i < 2 ; ++i )
   {
      // dessiner une ligne vers la source lumineuse
      glVertexAttrib3f( locColorBase, 1.0, 1.0, 0.5 ); // jaune
      if ( !etat.positionnelle )
      {
         matrModel.PushMatrix(); {
            glm::mat4 a;
            a[0] = glm::vec4( 2*LightSource.position[i].x,  2*LightSource.position[i].y,  2*LightSource.position[i].z, 0.0 );
            a[1] = glm::vec4( 2*LightSource.position[i].x, -2*LightSource.position[i].y,  2*LightSource.position[i].z, 0.0 );
            a[2] = glm::vec4( 2*LightSource.position[i].x,  2*LightSource.position[i].y, -2*LightSource.position[i].z, 0.0 );
            matrModel.setMatr( matrModel.getMatr() * a );
            glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
            glBindVertexArray( vao[1] );
            glDrawArrays( GL_LINES, 0, 2 );
            glBindVertexArray( 0 );
         } matrModel.PopMatrix();
         glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
      }

      // dessiner une sphère à la position de la lumière
      matrModel.PushMatrix(); {
         matrModel.Translate( LightSource.position[i].x, LightSource.position[i].y, LightSource.position[i].z );
         glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
         sphereLumi->afficher();
      } matrModel.PopMatrix();
      glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
   }
}

void afficherModele()
{
   // afficher d'abord les décorations (en utilisant progBase)
   afficherDecoration();

   // afficher le modèle (en utilisant prog)
   glUseProgram( prog );

   // s'il y a lieu, assigner les textures aux unités de texture
   glActiveTexture( GL_TEXTURE0 );
   glBindTexture( GL_TEXTURE_2D, etat.indiceTexture ? textures[etat.indiceTexture-1] : 0 );
   glActiveTexture( GL_TEXTURE1 );
   glBindTexture( GL_TEXTURE_2D, etat.indiceDiffuse ? textures[etat.indiceDiffuse-1] : 0 );

   // partie 1: activer les deux glClipPane limitant le z vers le haut et vers le bas
   glEnable( GL_CLIP_PLANE0 );
   glEnable( GL_CLIP_PLANE1 );

   // afficher la surface (plane)
   glBindVertexArray( vao[0] );
   glPatchParameteri( GL_PATCH_VERTICES, 4 );

   // À MODIFIER (utiliser des GL_PATCHES)
   // glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
   glDrawArrays( GL_PATCHES, 0, 4 ); // UTILISER des GL_PATCHES plutôt que des GL_TRIANGLES

   glBindVertexArray( 0 );

   glDisable( GL_CLIP_PLANE0 );
   glDisable( GL_CLIP_PLANE1 );
}

void FenetreTP::afficherScene()
{
   // effacer l'écran et le tampon de profondeur
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   glUseProgram( progBase );

   // définir le pipeline graphique
   definirProjection( 0, largeur_, hauteur_ );
   glUniformMatrix4fv( locmatrProjBase, 1, GL_FALSE, matrProj );

   camera.definir();
   glUniformMatrix4fv( locmatrVisuBase, 1, GL_FALSE, matrVisu );

   matrModel.LoadIdentity();
   glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
   glUniformMatrix3fv( locmatrNormale, 1, GL_TRUE, glm::value_ptr( glm::inverse( glm::mat3( matrVisu.getMatr() * matrModel.getMatr() ) ) ) );

   // afficher les axes
   if ( etat.afficheAxes ) FenetreTP::afficherAxes( );

   glUseProgram( prog );
   glUniform1i( loctextureDepl, 0 );    // '0' => utilisation de GL_TEXTURE0
   glUniform1i( loctextureCoul, 1 );    // '1' => utilisation de GL_TEXTURE1
   glUniform1f( locfacteurDeform, etat.facteurDeform );
   glUniform4fv( locbDim, 1, glm::value_ptr(etat.bDim) );
   glUniform1f( locTessLevelInner, etat.TessLevelInner );
   glUniform1f( locTessLevelOuter, etat.TessLevelOuter );

   // mettre à jour les blocs de variables uniformes
   {
      glBindBuffer( GL_UNIFORM_BUFFER, ubo[0] );
      GLvoid *p = glMapBuffer( GL_UNIFORM_BUFFER, GL_WRITE_ONLY );
      memcpy( p, &LightSource, sizeof(LightSource) );
      glUnmapBuffer( GL_UNIFORM_BUFFER );
   }
   {
      glBindBuffer( GL_UNIFORM_BUFFER, ubo[1] );
      GLvoid *p = glMapBuffer( GL_UNIFORM_BUFFER, GL_WRITE_ONLY );
      memcpy( p, &FrontMaterial, sizeof(FrontMaterial) );
      glUnmapBuffer( GL_UNIFORM_BUFFER );
   }
   {
      glBindBuffer( GL_UNIFORM_BUFFER, ubo[2] );
      GLvoid *p = glMapBuffer( GL_UNIFORM_BUFFER, GL_WRITE_ONLY );
      memcpy( p, &LightModel, sizeof(LightModel) );
      glUnmapBuffer( GL_UNIFORM_BUFFER );
   }

   //glUniformMatrix4fv( locmatrProj, 1, GL_FALSE, matrProj ); // inutile car on modifie ensuite la projection
   glUniformMatrix4fv( locmatrVisu, 1, GL_FALSE, matrVisu );
   glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
   // (partie 1: ne pas oublier de calculer et donner une matrice pour les transformations des normales)
   glUniformMatrix3fv( locmatrNormale, 1, GL_TRUE, glm::value_ptr( glm::inverse( glm::mat3( matrVisu.getMatr() * matrModel.getMatr() ) ) ) );

   // partie 2: afficher la surface en mono ou en stéréo

   switch ( vue.affichageStereo )
   {
   case 0: // mono
      definirProjection( 0, largeur_, hauteur_ );
      glUniformMatrix4fv( locmatrProj, 1, GL_FALSE, matrProj );
      afficherModele();
      break;

   case 1: // stéréo anaglyphe
      // partie 2: à modifier pour afficher en anaglyphe
      definirProjection( 0, largeur_, hauteur_ );
      glUniformMatrix4fv( locmatrProj, 1, GL_FALSE, matrProj );
      afficherModele();
      break;

   case 2: // stéréo double
      // partie 2: à modifier pour afficher en stéréo double
      definirProjection( 0, largeur_, hauteur_ );
      glUniformMatrix4fv( locmatrProj, 1, GL_FALSE, matrProj );
      afficherModele();
      break;
   }
}

// fonction de redimensionnement de la fenêtre graphique
void FenetreTP::redimensionner( GLsizei w, GLsizei h )
{
   glViewport( 0, 0, w, h );
}

void FenetreTP::clavier( TP_touche touche )
{
   switch ( touche )
   {
   case TP_ECHAP:
   case TP_q: // Quitter l'application
      quit();
      break;

   case TP_x: // Activer/désactiver l'affichage des axes
      etat.afficheAxes = !etat.afficheAxes;
      std::cout << "// Affichage des axes ? " << ( etat.afficheAxes ? "OUI" : "NON" ) << std::endl;
      break;

   case TP_v: // Recharger les fichiers des nuanceurs et recréer le programme
      chargerNuanceurs();
      std::cout << "// Recharger nuanceurs" << std::endl;
      break;

   case TP_i: // Augmenter le niveau de tessellation interne
      ++etat.TessLevelInner;
      std::cout << " TessLevelInner=" << etat.TessLevelInner << " TessLevelOuter=" << etat.TessLevelOuter << std::endl;
      glPatchParameteri( GL_PATCH_DEFAULT_INNER_LEVEL, etat.TessLevelInner );
      break;
   case TP_k: // Diminuer le niveau de tessellation interne
      if ( --etat.TessLevelInner < 1 ) etat.TessLevelInner = 1;
      std::cout << " TessLevelInner=" << etat.TessLevelInner << " TessLevelOuter=" << etat.TessLevelOuter << std::endl;
      glPatchParameteri( GL_PATCH_DEFAULT_INNER_LEVEL, etat.TessLevelInner );
      break;

   case TP_o: // Augmenter le niveau de tessellation externe
      ++etat.TessLevelOuter;
      std::cout << " TessLevelInner=" << etat.TessLevelInner << " TessLevelOuter=" << etat.TessLevelOuter << std::endl;
      glPatchParameteri( GL_PATCH_DEFAULT_OUTER_LEVEL, etat.TessLevelOuter );
      break;
   case TP_l: // Diminuer le niveau de tessellation externe
      if ( --etat.TessLevelOuter < 1 ) etat.TessLevelOuter = 1;
      std::cout << " TessLevelInner=" << etat.TessLevelInner << " TessLevelOuter=" << etat.TessLevelOuter << std::endl;
      glPatchParameteri( GL_PATCH_DEFAULT_OUTER_LEVEL, etat.TessLevelOuter );
      break;

   case TP_u: // Augmenter les deux niveaux de tessellation
      ++etat.TessLevelOuter;
      etat.TessLevelInner = etat.TessLevelOuter;
      std::cout << " TessLevelInner=" << etat.TessLevelInner << " TessLevelOuter=" << etat.TessLevelOuter << std::endl;
      glPatchParameteri( GL_PATCH_DEFAULT_OUTER_LEVEL, etat.TessLevelOuter );
      glPatchParameteri( GL_PATCH_DEFAULT_INNER_LEVEL, etat.TessLevelInner );
      break;
   case TP_j: // Diminuer les deux niveaux de tessellation
      if ( --etat.TessLevelOuter < 1 ) etat.TessLevelOuter = 1;
      etat.TessLevelInner = etat.TessLevelOuter;
      std::cout << " TessLevelInner=" << etat.TessLevelInner << " TessLevelOuter=" << etat.TessLevelOuter << std::endl;
      glPatchParameteri( GL_PATCH_DEFAULT_OUTER_LEVEL, etat.TessLevelOuter );
      glPatchParameteri( GL_PATCH_DEFAULT_INNER_LEVEL, etat.TessLevelInner );
      break;

   case TP_PLUS: // Augmenter l'effet du déplacement
   case TP_EGAL:
      etat.facteurDeform += 0.01;
      std::cout << " facteurDeform=" << etat.facteurDeform << std::endl;
      break;
   case TP_MOINS: // Diminuer l'effet du déplacement
      etat.facteurDeform -= 0.01;
      std::cout << " facteurDeform=" << etat.facteurDeform << std::endl;
      break;

   case TP_DROITE: // Augmenter la dimension de la boîte en X
      etat.bDim.x += 0.1;
      std::cout << " bDim= " << etat.bDim.x << " x " << etat.bDim.y << " x " << etat.bDim.z << std::endl;
      break;
   case TP_GAUCHE: // Diminuer la dimension de la boîte en X
      if ( etat.bDim.x > 0.25 ) etat.bDim.x -= 0.1;
      std::cout << " bDim= " << etat.bDim.x << " x " << etat.bDim.y << " x " << etat.bDim.z << std::endl;
      break;
   case TP_BAS: // Augmenter la dimension de la boîte en Y
      etat.bDim.y += 0.1;
      std::cout << " bDim= " << etat.bDim.x << " x " << etat.bDim.y << " x " << etat.bDim.z << std::endl;
      break;
   case TP_HAUT: // Diminuer la dimension de la boîte en Y
      if ( etat.bDim.y > 0.25 ) etat.bDim.y -= 0.1;
      std::cout << " bDim= " << etat.bDim.x << " x " << etat.bDim.y << " x " << etat.bDim.z << std::endl;
      break;
   case TP_PAGEPREC: // Augmenter la dimension de la boîte en Z
      etat.bDim.z += 0.1;
      std::cout << " bDim= " << etat.bDim.x << " x " << etat.bDim.y << " x " << etat.bDim.z << std::endl;
      break;
   case TP_PAGESUIV: // Diminuer la dimension de la boîte en Z
      if ( etat.bDim.z > 0.25 ) etat.bDim.z -= 0.1;
      std::cout << " bDim= " << etat.bDim.x << " x " << etat.bDim.y << " x " << etat.bDim.z << std::endl;
      break;

   case TP_0: // Revenir à la surface de base (ne pas utiliser de textures)
      LightSource.position[0] = posLumiInit[0];
      LightSource.position[1] = posLumiInit[1];
      etat.indiceFonction = etat.indiceTexture = etat.indiceDiffuse = 0;
      std::cout << "indiceFonction=" << etat.indiceFonction << " indiceTexture=" << etat.indiceTexture << " indiceDiffuse=" << etat.indiceDiffuse << std::endl;
      chargerNuanceurs(); // pour les #define
      break;

   case TP_f: // Varier la fonction mathématique utilisée pour le déplacement
      if ( ++etat.indiceFonction > 6 ) etat.indiceFonction = 0;
      etat.indiceTexture = 0;
      std::cout << "indiceFonction=" << etat.indiceFonction << " indiceTexture=" << etat.indiceTexture << " indiceDiffuse=" << etat.indiceDiffuse << std::endl;
      chargerNuanceurs(); // pour les #define
      break;

   case TP_t: // Varier l'indice de la texture utilisée pour le déplacement
      if ( ++etat.indiceTexture > 9 ) etat.indiceTexture = 0;
      etat.indiceFonction = 0;
      std::cout << "indiceFonction=" << etat.indiceFonction << " indiceTexture=" << etat.indiceTexture << " indiceDiffuse=" << etat.indiceDiffuse << std::endl;
      chargerNuanceurs(); // pour les #define
      break;

   case TP_c: // Varier l'indice de la texture utilisée pour la composante diffuse de la couleur
      if ( ++etat.indiceDiffuse > 9 ) etat.indiceDiffuse = 0;
      std::cout << "indiceFonction=" << etat.indiceFonction << " indiceTexture=" << etat.indiceTexture << " indiceDiffuse=" << etat.indiceDiffuse << std::endl;
      chargerNuanceurs(); // pour les #define
      break;

   case TP_e: // Varier l'indice de la texture utilisée pour la couleur ET le déplacement
      if ( ++etat.indiceTexture > 9 ) etat.indiceTexture = 0;
      etat.indiceDiffuse = etat.indiceTexture;
      etat.indiceFonction = 0;
      std::cout << "indiceFonction=" << etat.indiceFonction << " indiceTexture=" << etat.indiceTexture << " indiceDiffuse=" << etat.indiceDiffuse << std::endl;
      chargerNuanceurs(); // pour les #define
      break;

   case TP_p: // Permuter lumière positionnelle ou directionnelle
      etat.positionnelle = !etat.positionnelle;
      LightSource.position[0].w = LightSource.position[1].w = etat.positionnelle ? 1.0 : 0.0;
      std::cout << " positionnelle=" << etat.positionnelle << std::endl;
      break;

   case TP_s: // Varier le type d'affichage stéréo: mono, stéréo anaglyphe, stéréo double
      if ( ++vue.affichageStereo > 2 ) vue.affichageStereo = 0;
      std::cout << " affichageStereo=" << vue.affichageStereo << std::endl;
      break;

   case TP_g: // Permuter l'affichage en fil de fer ou plein
      etat.modePolygone = ( etat.modePolygone == GL_FILL ) ? GL_LINE : GL_FILL;
      glPolygonMode( GL_FRONT_AND_BACK, etat.modePolygone );
      break;

   case TP_n: // Utiliser ou non les normales calculées comme couleur (pour le débogage)
      etat.afficheNormales = !etat.afficheNormales;
      chargerNuanceurs(); // pour les #define
      break;

   case TP_ESPACE: // Mettre en pause ou reprendre l'animation
      etat.enmouvement = !etat.enmouvement;
      break;

   default:
      std::cout << " touche inconnue : " << (char) touche << std::endl;
      imprimerTouches();
      break;
   }
}

// fonction callback pour un clic de souris
// la dernière position de la souris
glm::ivec2 sourisPosPrec(0,0);
static enum { deplaceCam, deplaceLumProfondeur, deplaceLum } deplace = deplaceCam;
static bool pressed = false;
void FenetreTP::sourisClic( int button, int state, int x, int y )
{
   pressed = ( state == TP_PRESSE );
   if ( pressed )
   {
      // on vient de presser la souris
      sourisPosPrec.x = x;
      sourisPosPrec.y = y;
      switch ( button )
      {
      case TP_BOUTON_GAUCHE: // Déplacer la caméra
         deplace = deplaceCam;
         break;
      case TP_BOUTON_MILIEU: // Déplacer la lumière en profondeur uniquement
         deplace = deplaceLumProfondeur;
         break;
      case TP_BOUTON_DROIT: // Déplacer la lumière à la position de la souris (sans changer la profondeur)
         deplace = deplaceLum;
         break;
      }
      if ( deplace != deplaceCam )
      {
         glm::mat4 VM = matrVisu.getMatr()*matrModel.getMatr();
         glm::mat4 P = matrProj.getMatr();
         glm::vec4 cloture( 0, 0, largeur_, hauteur_ );
         glm::vec2 ecranLumi0 = glm::vec2( glm::project( glm::vec3(LightSource.position[0]), VM, P, cloture ) );
         glm::vec2 ecranLumi1 = glm::vec2( glm::project( glm::vec3(LightSource.position[1]), VM, P, cloture ) );
         glm::vec2 ecranXY( x, hauteur_-y );
         etat.curLumi = ( glm::distance( ecranLumi0, ecranXY ) <
                          glm::distance( ecranLumi1, ecranXY ) ) ? 0 : 1;
      }
   }
   else
   {
      // on vient de relacher la souris
   }
}

void FenetreTP::sourisWheel( int x, int y ) // Changer le facteurDeform
{
   const int sens = +1;
   etat.facteurDeform += 0.01 * sens * y;
   std::cout << " facteurDeform=" << etat.facteurDeform << std::endl;
}

// fonction de mouvement de la souris
void FenetreTP::sourisMouvement( int x, int y )
{
   if ( pressed )
   {
      int dx = x - sourisPosPrec.x;
      int dy = y - sourisPosPrec.y;
      glm::mat4 VM = matrVisu.getMatr()*matrModel.getMatr();
      glm::mat4 P = matrProj.getMatr();
      glm::vec4 cloture( 0, 0, largeur_, hauteur_ );
      // obtenir les coordonnées d'écran correspondant à la position de la lumière
      glm::vec3 ecranLumi = glm::project( glm::vec3(LightSource.position[etat.curLumi]), VM, P, cloture );
      switch ( deplace )
      {
      case deplaceCam: // déplacer la caméra par incrément
         camera.theta -= dx / 3.0;
         camera.phi   -= dy / 3.0;
         break;
      case deplaceLumProfondeur:
         // modifier seulement la profondeur de la lumière
         ecranLumi[2] -= dy * 0.001;
         LightSource.position[etat.curLumi] = glm::vec4( glm::unProject( ecranLumi, VM, P, cloture ), etat.positionnelle ? 1.0 : 0.0 );
         break;
      case deplaceLum:
         // placer la lumière à la nouvelle position (en utilisant la profondeur actuelle)
         glm::vec3 ecranPos( x, hauteur_-y, ecranLumi.z );
         LightSource.position[etat.curLumi] = glm::vec4( glm::unProject( ecranPos, VM, P, cloture ), etat.positionnelle ? 1.0 : 0.0 );
         break;
      }

      sourisPosPrec.x = x;
      sourisPosPrec.y = y;

      camera.verifierAngles();
   }
}

int main( int argc, char *argv[] )
{
   // créer une fenêtre
   FenetreTP fenetre( "INF2705 TP" );

   // allouer des ressources et définir le contexte OpenGL
   fenetre.initialiser();

   bool boucler = true;
   while ( boucler )
   {
      // mettre à jour la physique
      calculerPhysique( );

      // affichage
      fenetre.afficherScene();
      fenetre.swap();

      // récupérer les événements et appeler la fonction de rappel
      boucler = fenetre.gererEvenement();
   }

   // détruire les ressources OpenGL allouées
   fenetre.conclure();

   return 0;
}
