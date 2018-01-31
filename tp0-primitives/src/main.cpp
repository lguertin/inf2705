// Prénoms, noms et matricule des membres de l'équipe:
// - Prénom1 NOM1 (matricule1)
// - Prénom2 NOM2 (matricule2)
#warning "Écrire les prénoms, noms et matricule des membres de l'équipe dans le fichier et commenter cette ligne"

#include <iostream>
#include "inf2705-matrice.h"
#include "inf2705-nuanceur.h"
#include "inf2705-fenetre.h"

// variables pour l'utilisation des nuanceurs
GLuint progBase;  // le programme de nuanceurs de base
GLint locVertex = -1;
GLint locColor = -1;
GLint locmatrModel = -1;
GLint locmatrVisu = -1;
GLint locmatrProj = -1;

// matrices de du pipeline graphique
MatricePipeline matrProj, matrVisu, matrModel;

bool afficheAxes = false;      // indique si on affiche les axes
GLenum modePolygone = GL_FILL; // comment afficher les polygones

#define SHOW_W 0

#if SHOW_W
// pour un W
float p1[3] = { -4.0,  2.0,  0.0 };
float p2[3] = { -3.0, -3.0,  0.0 };
float p3[3] = { -1.0, -3.0,  0.0 };
float p4[3] = {  0.0,  0.0,  0.0 };
float p5[3] = {  1.0, -3.0,  0.0 };
float p6[3] = {  3.0, -3.0,  0.0 };
float p7[3] = {  4.0,  2.0,  0.0 };
#else
// pour une flèche (Voir apprentissage supplémentaire)
float p1[3] = { -3.0,  1.0,  0.0 };
float p2[3] = { -3.0, -1.0,  0.0 };
float p3[3] = {  0.0, -1.0,  0.0 };
float p4[3] = { -0.5, -2.5,  0.0 };
float p5[3] = {  3.0,  0.0,  0.0 };
float p6[3] = { -0.5,  2.5,  0.0 };
float p7[3] = {  0.0,  1.0,  0.0 };
#endif

void chargerNuanceurs()
{
   // charger le nuanceur de base
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
      if ( ( locVertex = glGetAttribLocation( progBase, "Vertex" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Vertex" << std::endl;
      if ( ( locColor = glGetAttribLocation( progBase, "Color" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Color" << std::endl;
      if ( ( locmatrModel = glGetUniformLocation( progBase, "matrModel" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrModel" << std::endl;
      if ( ( locmatrVisu = glGetUniformLocation( progBase, "matrVisu" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrVisu" << std::endl;
      if ( ( locmatrProj = glGetUniformLocation( progBase, "matrProj" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrProj" << std::endl;
   }
}

void initialiser()
{
   // donner la couleur de fond
   glClearColor( 0.0, 0.0, 0.0, 1.0 );

   // activer le mélange de couleur pour bien voir les possibles plis à l'affichage
   glEnable( GL_BLEND );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

   // charger les nuanceurs
   chargerNuanceurs();

   glPointSize( 3.0 );
}

void conclure()
{
}

void FenetreTP::afficherScene()
{
#if 0
   // définir le pipeline graphique
   // VERSION OpenGL 2.1
   glMatrixMode( GL_PROJECTION );
   glLoadIdentity();
   glOrtho( -12, 12, -8, 8, -10, 10 ); // la taille de la fenêtre

   glMatrixMode( GL_MODELVIEW );
   glLoadIdentity();

#else
   // définir le pipeline graphique
   // VERSION OpenGL 4.x
   glUseProgram( progBase );

   // définir le pipeline graphique
   // la fenêtre varie en X de -12 à 12 et en Y de -8 à 8
   matrProj.Ortho( -12, 12, -8, 8, -10, 10 );
   glUniformMatrix4fv( locmatrProj, 1, GL_FALSE, matrProj );

   matrVisu.LoadIdentity();
   glUniformMatrix4fv( locmatrVisu, 1, GL_FALSE, matrVisu );

   matrModel.LoadIdentity();
   glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );

   // Attention: Quelques #define ici afin de simuler l'utilisation de OpenGL 2.x
#define glTranslatef( X, Y, Z )                                         \
   {                                                                    \
      matrModel.Translate( (X), (Y), (Z) );                             \
      glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );       \
   }

#endif

   // effacer l'écran et le tampon de profondeur
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   // afficher les axes
   if ( afficheAxes ) FenetreTP::afficherAxes();

   // mode d'affichage courant
   glPolygonMode( GL_FRONT_AND_BACK, modePolygone );

   // exemple: utiliser du rouge opaque
   glColor4f( 1, 0, 0, 1 );
   // mieux encore: utiliser du rouge, mais avec un alpha de 0.8 plutôt que 1.0 pour bien voir les possibles plis à l'affichage
   glColor4f( 1, 0, 0, 0.8 );

   // afficher les lignes blanches séparant l'écran en 6 parties
   // la fenêtre varie en X de -12 à 12 et en Y de -8 à 8
   glColor4f( 1, 1, 1, 1 );
     
   glBegin( GL_LINES);
   {
      glVertex3f(-12.0, 0.0, 0.0);
      glVertex3f(12.0, 0.0,0.0);
   }
   glEnd();
   
   glBegin( GL_LINES);
   {
      glVertex3f(-4.0, 8.0, 0.0);
      glVertex3f(-4.0, -8.0,0.0);
   }
   glEnd();
   
   glBegin( GL_LINES);
   {
      glVertex3f(4.0, 8.0, 0.0);
      glVertex3f(4.0, -8.0, 0.0);
   }
   glEnd();

   #if SHOW_W
	// Pour les W
   // tracer la première forme  ...
   glTranslatef( -8.0, 4.0, -1.0 );
   glColor3f( 1.0, 0.0, 0.0 );
   glBegin( GL_QUADS );
   {
      glVertex3fv(p1);
      glVertex3fv(p2);
      glVertex3fv(p3);
      glVertex3fv(p4);
   }
   glEnd();
   
   glBegin( GL_QUADS );
   {
      glVertex3fv(p1);
      glVertex3fv(p7);
      glVertex3fv(p4);
      glVertex3fv(p4);
   }
   glEnd();
   
   glBegin( GL_QUADS );
   {
      glVertex3fv(p4);
      glVertex3fv(p5);
      glVertex3fv(p6);
      glVertex3fv(p7);
   }
   glEnd();

   // tracer la seconde forme ...
   glTranslatef( 8.0, 0.0, 0.0 );
   glColor3f( 0.0, 1.0, 0.0 );
   glBegin( GL_TRIANGLES );
   {
      glVertex3fv(p1);
      glVertex3fv(p2);
      glVertex3fv(p4);
   }
   glEnd();
   
   glBegin( GL_TRIANGLES );
   {
      glVertex3fv(p2);
      glVertex3fv(p3);
      glVertex3fv(p4);
   }
   glEnd();
   
   glBegin( GL_TRIANGLES );
   {
      glVertex3fv(p1);
      glVertex3fv(p4);
      glVertex3fv(p7);
   }
   glEnd();
   
   glBegin( GL_TRIANGLES );
   {
      glVertex3fv(p4);
      glVertex3fv(p5);
      glVertex3fv(p6);
   }
   glEnd();
   
   glBegin( GL_TRIANGLES );
   {
      glVertex3fv(p4);
      glVertex3fv(p6);
      glVertex3fv(p7);
   }
   glEnd();
   
   // tracer la troisieme forme ...
   glTranslatef( 8.0, 0.0, 0.0 );
   glColor3f( 0.0, 0.0, 1.0 );
   glBegin( GL_POLYGON );
   {
      glVertex3fv(p1);
      glVertex3fv(p2);
      glVertex3fv(p3);
      glVertex3fv(p4);
   }
   glEnd();
   
   glBegin( GL_POLYGON );
   {
      glVertex3fv(p1);
      glVertex3fv(p4);
      glVertex3fv(p7);
   }
   glEnd();
   
   glBegin( GL_POLYGON );
   {
      glVertex3fv(p4);
      glVertex3fv(p5);
      glVertex3fv(p6);
      glVertex3fv(p7);
   }
   glEnd();
   
   // tracer la sixieme forme ...
   glTranslatef( 0.0, -8.0, 0.0 );
   glColor3f( 1.0, 1.0,0.0);
   glBegin( GL_TRIANGLE_FAN );
   {
      glVertex3fv(p4);
      glVertex3fv(p3);
      glVertex3fv(p2);
      glVertex3fv(p1);
      glVertex3fv(p7);
      glVertex3fv(p6);
      glVertex3fv(p5);
   }
   glEnd();
   
   // tracer la cinquieme forme ...
   glTranslatef( -8.0, 0.0, 0.0 );
   glColor3f( 1.0, 0.0,1.0);
   glBegin( GL_TRIANGLE_STRIP );
   {
      glVertex3fv(p2);
      glVertex3fv(p3);
      glVertex3fv(p1);
      glVertex3fv(p4);
      glVertex3fv(p7);
      glVertex3fv(p5);
      glVertex3fv(p6);
   }
   glEnd();
   
   // tracer la quatrieme forme ...
   glTranslatef( -8.0, 0.0, 0.0 );
   glColor3f( 0.0, 1.0, 1.0 );
   glBegin( GL_QUAD_STRIP );
   {
	  glVertex3fv(p2);
      glVertex3fv(p3);
      glVertex3fv(p1);
      glVertex3fv(p4);
      glVertex3fv(p4);
      glVertex3fv(p7);
      glVertex3fv(p5);
      glVertex3fv(p6);
   }
   glEnd();

   #else
   // Pour les fleches
   
   // tracer la première forme  ...
   glTranslatef( -8.0, 4.0, -1.0 );
   glColor3f( 1.0, 0.0, 0.0 );
   glBegin( GL_QUADS );
   {
      glVertex3fv(p1);
      glVertex3fv(p2);
      glVertex3fv(p3);
      glVertex3fv(p7);
   }
   glEnd();
   
   glBegin( GL_QUADS );
   {
      glVertex3fv(p7);
      glVertex3fv(p6);
      glVertex3fv(p5);
      glVertex3fv(p3);
   }
   glEnd();
   
   glBegin( GL_QUADS );
   {
      glVertex3fv(p3);
      glVertex3fv(p3);
      glVertex3fv(p4);
      glVertex3fv(p5);
   }
   glEnd();

   // tracer la seconde forme ...
   glTranslatef( 8.0, 0.0, 0.0 );
   glColor3f( 0.0, 1.0, 0.0 );
   glBegin( GL_TRIANGLES );
   {
      glVertex3fv(p1);
      glVertex3fv(p2);
      glVertex3fv(p3);
   }
   glEnd();
   
   glBegin( GL_TRIANGLES );
   {
      glVertex3fv(p1);
      glVertex3fv(p3);
      glVertex3fv(p7);
   }
   glEnd();
   
   glBegin( GL_TRIANGLES );
   {
      glVertex3fv(p3);
      glVertex3fv(p4);
      glVertex3fv(p5);
   }
   glEnd();
   
   glBegin( GL_TRIANGLES );
   {
      glVertex3fv(p3);
      glVertex3fv(p5);
      glVertex3fv(p7);
   }
   glEnd();
   
   glBegin( GL_TRIANGLES );
   {
      glVertex3fv(p5);
      glVertex3fv(p6);
      glVertex3fv(p7);
   }
   glEnd();
   
   // tracer la troisieme forme ...
   glTranslatef( 8.0, 0.0, 0.0 );
   glColor3f( 0.0, 0.0, 1.0 );
   glBegin( GL_POLYGON );
   {
      glVertex3fv(p1);
      glVertex3fv(p2);
      glVertex3fv(p3);
      glVertex3fv(p5);
      glVertex3fv(p7);
   }
   glEnd();
   
   glBegin( GL_POLYGON );
   {
      glVertex3fv(p5);
      glVertex3fv(p4);
      glVertex3fv(p3);
   }
   glEnd();
   
   glBegin( GL_POLYGON );
   {
      glVertex3fv(p5);
      glVertex3fv(p6);
      glVertex3fv(p7);
   }
   glEnd();
   
   // tracer la sixieme forme ...
   glTranslatef( 0.0, -8.0, 0.0 );
   glColor3f( 1.0, 1.0,0.0);
   glBegin( GL_TRIANGLE_FAN );
   {
      glVertex3fv(p5);
      glVertex3fv(p4);
      glVertex3fv(p3);
      glVertex3fv(p2);
      glVertex3fv(p1);
      glVertex3fv(p7);
      glVertex3fv(p6);
   }
   glEnd();
   
   // tracer la cinquieme forme ...
   glTranslatef( -8.0, 0.0, 0.0 );
   glColor3f( 1.0, 0.0,1.0);
   glBegin( GL_TRIANGLE_STRIP );
   {
      glVertex3fv(p6);
      glVertex3fv(p7);
      glVertex3fv(p5);
      glVertex3fv(p3);
      glVertex3fv(p4);
   }
   glEnd();
   
   glBegin( GL_TRIANGLE_STRIP );
   {
      glVertex3fv(p2);
      glVertex3fv(p3);
      glVertex3fv(p1);
      glVertex3fv(p7);
   }
   glEnd();
   
   // tracer la quatrieme forme ...
   glTranslatef( -8.0, 0.0, 0.0 );
   glColor3f( 0.0, 1.0, 1.0 );
   glBegin( GL_QUAD_STRIP );
   {
	  glVertex3fv(p2);
      glVertex3fv(p1);
      glVertex3fv(p3);
      glVertex3fv(p7);
      glVertex3fv(p5);
      glVertex3fv(p7);
   }
   glEnd();
   
   glBegin( GL_QUAD_STRIP );
   {
	  glVertex3fv(p6);
      glVertex3fv(p5);
      glVertex3fv(p7);
      glVertex3fv(p5);
   }
   glEnd();

   glBegin( GL_QUAD_STRIP );
   {
	  glVertex3fv(p4);
      glVertex3fv(p5);
      glVertex3fv(p3);
      glVertex3fv(p5);
   }
   glEnd();
   
   #endif
}

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
      afficheAxes = !afficheAxes;
      std::cout << "// Affichage des axes ? " << ( afficheAxes ? "OUI" : "NON" ) << std::endl;
      break;

   case TP_v: // Recharger les fichiers des nuanceurs et recréer le programme
      chargerNuanceurs();
      std::cout << "// Recharger nuanceurs" << std::endl;
      break;

   case TP_g: // Permuter l'affichage en fil de fer ou plein
      modePolygone = ( modePolygone == GL_FILL ) ? GL_LINE : GL_FILL;
      break;

   default:
      std::cout << " touche inconnue : " << (char) touche << std::endl;
      imprimerTouches();
      break;
   }
}

void FenetreTP::sourisClic( int button, int state, int x, int y )
{
}

void FenetreTP::sourisWheel( int x, int y )
{
}

void FenetreTP::sourisMouvement( int x, int y )
{
}

int main( int argc, char *argv[] )
{
   // créer une fenêtre
   FenetreTP fenetre( "INF2705 TP" );

   // allouer des ressources et définir le contexte OpenGL
   initialiser();

   bool boucler = true;
   while ( boucler )
   {
      // affichage
      fenetre.afficherScene();
      fenetre.swap();

      // récupérer les événements et appeler la fonction de rappel
      boucler = fenetre.gererEvenement();
   }

   // détruire les ressources OpenGL allouées
   conclure();

   return 0;
}
