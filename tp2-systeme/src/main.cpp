// Prénoms, noms et matricule des membres de l'équipe:
// - Léandre Guertin (1841782)
// - Issifath Sanni (1771817)
// #warning "Écrire les prénoms, noms et matricule des membres de l'équipe dans le fichier et commenter cette ligne"

#include <stdlib.h>
#include <iostream>
#include "inf2705-matrice.h"
#include "inf2705-nuanceur.h"
#include "inf2705-fenetre.h"
#include "inf2705-forme.h"
#include <glm/gtx/io.hpp>
#include <vector>

// variables pour l'utilisation des nuanceurs
GLuint prog; // votre programme de nuanceurs
GLint locVertex = -1;
GLint locColor = -1;
GLint locmatrModel = -1;
GLint locmatrVisu = -1;
GLint locmatrProj = -1;
GLint locplanCoupe = -1;
GLint loccoulProfondeur = -1;
GLuint progBase; // le programme de nuanceurs de base
GLint locVertexBase = -1;
GLint locColorBase = -1;
GLint locmatrModelBase = -1;
GLint locmatrVisuBase = -1;
GLint locmatrProjBase = -1;

// matrices du pipeline graphique
MatricePipeline matrProj, matrVisu, matrModel;

// les formes
FormeCube *cube = NULL;
FormeSphere *sphere = NULL;
FormeTheiere *theiere = NULL;
FormeTore *toreTerre = NULL;
FormeTore *toreMars = NULL;
FormeTore *toreJupiter = NULL;
GLuint vao = 0;
GLuint vbo[2] = {0, 0};

//
// variables d'état
//
struct Etat
{
    int modele;          // le modèle à afficher comme CorpsCeleste (1-sphère, 2-cube, 3-théière).
    bool modeSelection;  // on est en mode sélection?
    bool enmouvement;    // le modèle est en mouvement/rotation automatique ou non
    bool afficheAxes;    // indique si on affiche les axes
    bool coulProfondeur; // indique si on veut colorer selon la profondeur
    GLenum modePolygone; // comment afficher les polygones
    glm::ivec2 sourisPosPrec;
    // partie 1: utiliser un plan de coupe
    glm::vec4 planCoupe; // équation du plan de coupe (partie 1)
    GLfloat angleCoupe;  // angle (degrés) autour de x (partie 1)
    // apprentissage supplémentaire: facteur de réchauffement
    float facteurRechauffement; // un facteur qui sert à calculer la couleur des pôles (0.0=froid, 1.0=chaud)
} etat = {1, false, true, true, false, GL_FILL, glm::ivec2(0), glm::vec4(0, 0, 1, 0), 0.0, 0.2};

//
// variables pour définir le point de vue
//
class Camera
{
  public:
    void definir()
    {
        matrVisu.LookAt(dist * cos(glm::radians(theta)) * sin(glm::radians(phi)),
                        dist * sin(glm::radians(theta)) * sin(glm::radians(phi)),
                        dist * cos(glm::radians(phi)),
                        0, 0, 0,
                        0, 0, 1);

        // (pour apprentissage supplémentaire): La caméra est sur la Terre et voir passer les autres objets célestes en utilisant l'inverse de la matrice mm
    }
    void verifierAngles() // vérifier que les angles ne débordent pas les valeurs permises
    {
        const GLdouble MINPHI = 0.01, MAXPHI = 180.0 - 0.01;
        phi = glm::clamp(phi, MINPHI, MAXPHI);
    }
    double theta;    // angle de rotation de la caméra (coord. sphériques)
    double phi;      // angle de rotation de la caméra (coord. sphériques)
    double dist;     // distance (coord. sphériques)
    bool modeLookAt; // on utilise LookAt (au lieu de Rotate et Translate)
} camera = {90.0, 75.0, 35.0, true};

//
// les corps célestes
//
class CorpsCeleste
{
  public:
    CorpsCeleste(float r, float dist, float rot, float rev, float vitRot, float vitRev,
                 glm::vec4 coul = glm::vec4(1., 1., 1., 1.)) : rayon(r), distance(dist),
                                                               rotation(rot), revolution(rev),
                                                               vitRotation(vitRot), vitRevolution(vitRev),
                                                               couleur(coul)
    {
    }

    void ajouteEnfant(CorpsCeleste &bebe)
    {
        enfants.push_back(&bebe);
    }

    void afficher()
    {
        matrModel.PushMatrix();
        {
            matrModel.Rotate(revolution, 0, 0, 1); // révolution du corps autour de son parent
            matrModel.Translate(distance, 0, 0);   // position par rapport à son parent

            // afficher d'abord les enfants
            std::vector<CorpsCeleste *>::iterator it;
            for (it = enfants.begin(); it != enfants.end(); it++)
            {
                (*it)->afficher();
            }

            // afficher le parent
            matrModel.PushMatrix();
            {
                matrModel.Rotate(rotation, 0, 0, 1);  // rotation sur lui-même
                matrModel.Scale(rayon, rayon, rayon); // la taille du corps
                glUniformMatrix4fv(locmatrModel, 1, GL_FALSE, matrModel);

                // la couleur du corps
                glVertexAttrib4fv(locColor, glm::value_ptr(couleur));

                if (couleur.w < 1.00)
                {
                    glEnable(GL_BLEND);
                    glDepthMask(GL_FALSE);
                }

                switch (etat.modele)
                {
                default:
                case 1:
                    sphere->afficher();
                    break;
                case 2:
                    cube->afficher();
                    break;
                case 3:
                    matrModel.Scale(0.5, 0.5, 0.5);
                    matrModel.Translate(0.0, 0.0, -1.0);
                    glUniformMatrix4fv(locmatrModel, 1, GL_FALSE, matrModel);
                    theiere->afficher();
                    break;
                }

                if (couleur.w < 1.0)
                {
                    glDepthMask(GL_TRUE);
                    glDisable(GL_BLEND);
                }
            }
            matrModel.PopMatrix();
            glUniformMatrix4fv(locmatrModel, 1, GL_FALSE, matrModel);
        }
        matrModel.PopMatrix();
        glUniformMatrix4fv(locmatrModel, 1, GL_FALSE, matrModel);
    }

    void avancerPhysique()
    {
        if (!estSelectionne)
        {
            const float dt = 0.5; // intervalle entre chaque affichage (en secondes)
            rotation += dt * vitRotation;
            revolution += dt * vitRevolution;
        }
    }

    std::vector<CorpsCeleste *> enfants; // la liste des enfants
    float rayon;                         // le rayon du corps
    float distance;                      // la distance au parent
    float rotation;                      // l'angle actuel de rotation en degrés
    float revolution;                    // l'angle actuel de révolution en degrés
    float vitRotation;                   // la vitesse de rotation
    float vitRevolution;                 // la vitesse de révolution
    glm::vec4 couleur;                   // la couleur du corps
    bool estSelectionne;                 // le corps est sélectionné ?
    //glm::vec3 couleurSel;                // la couleur en mode sélection
};

//                     rayon  dist  rota revol vrota  vrevol
CorpsCeleste Soleil(4.00, 0.0, 0.0, 0.0, 0.05, 0.0, glm::vec4(1.0, 1.0, 0.0, 0.5));

CorpsCeleste Terre(0.70, 7.0, 30.0, 30.0, 2.5, 0.10, glm::vec4(0.5, 0.5, 1.0, 1.0));
CorpsCeleste Lune(0.20, 1.5, 20.0, 30.0, 2.5, -0.35, glm::vec4(0.6, 0.6, 0.6, 1.0));

CorpsCeleste Mars(0.50, 11.0, 20.0, 140.0, 2.5, 0.13, glm::vec4(0.6, 1.0, 0.5, 1.0));
CorpsCeleste Phobos(0.20, 1.0, 5.0, 15.0, 3.5, 1.7, glm::vec4(0.4, 0.4, 0.8, 1.0));
CorpsCeleste Deimos(0.25, 1.7, 10.0, 2.0, 4.0, 0.5, glm::vec4(0.5, 0.5, 0.1, 1.0));

CorpsCeleste Jupiter(1.20, 16.0, 10.0, 40.0, 0.2, 0.02, glm::vec4(1.0, 0.5, 0.5, 1.0));
CorpsCeleste Io(0.20, 1.7, 5.0, 1.5, 2.5, 4.3, glm::vec4(0.7, 0.4, 0.5, 1.0));
CorpsCeleste Europa(0.25, 2.5, 87.0, 11.9, 3.5, 3.4, glm::vec4(0.8, 0.4, 0.8, 1.0));
CorpsCeleste Ganymede(0.30, 3.1, 10.0, 42.4, 4.0, 1.45, glm::vec4(0.3, 0.6, 0.1, 1.0));
CorpsCeleste Callisto(0.35, 4.0, 51.0, 93.1, 1.0, 0.45, glm::vec4(0.7, 0.5, 0.1, 1.0));

void calculerPhysique()
{
    if (etat.enmouvement)
    {
        // incrémenter rotation[] et revolution[] pour faire tourner les planètes
        Soleil.avancerPhysique();
        Terre.avancerPhysique();
        Lune.avancerPhysique();
        Mars.avancerPhysique();
        Phobos.avancerPhysique();
        Deimos.avancerPhysique();
        Jupiter.avancerPhysique();
        Io.avancerPhysique();
        Europa.avancerPhysique();
        Ganymede.avancerPhysique();
        Callisto.avancerPhysique();
    }
}

void chargerNuanceurs()
{
    // charger le nuanceur de base
    {
        // créer le programme
        progBase = glCreateProgram();

        // attacher le nuanceur de sommets
        {
            GLuint nuanceurObj = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(nuanceurObj, 1, &ProgNuanceur::chainesSommetsMinimal, NULL);
            glCompileShader(nuanceurObj);
            glAttachShader(progBase, nuanceurObj);
            ProgNuanceur::afficherLogCompile(nuanceurObj);
        }
        // attacher le nuanceur de fragments
        {
            GLuint nuanceurObj = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(nuanceurObj, 1, &ProgNuanceur::chainesFragmentsMinimal, NULL);
            glCompileShader(nuanceurObj);
            glAttachShader(progBase, nuanceurObj);
            ProgNuanceur::afficherLogCompile(nuanceurObj);
        }

        // faire l'édition des liens du programme
        glLinkProgram(progBase);
        ProgNuanceur::afficherLogLink(progBase);

        // demander la "Location" des variables
        if ((locVertexBase = glGetAttribLocation(progBase, "Vertex")) == -1)
            std::cerr << "!!! pas trouvé la \"Location\" de Vertex" << std::endl;
        if ((locColorBase = glGetAttribLocation(progBase, "Color")) == -1)
            std::cerr << "!!! pas trouvé la \"Location\" de Color" << std::endl;
        if ((locmatrModelBase = glGetUniformLocation(progBase, "matrModel")) == -1)
            std::cerr << "!!! pas trouvé la \"Location\" de matrModel" << std::endl;
        if ((locmatrVisuBase = glGetUniformLocation(progBase, "matrVisu")) == -1)
            std::cerr << "!!! pas trouvé la \"Location\" de matrVisu" << std::endl;
        if ((locmatrProjBase = glGetUniformLocation(progBase, "matrProj")) == -1)
            std::cerr << "!!! pas trouvé la \"Location\" de matrProj" << std::endl;
    }

    {
        // charger le nuanceur de ce TP

        // créer le programme
        prog = glCreateProgram();

        // attacher le nuanceur de sommets
        const GLchar *chainesSommets = ProgNuanceur::lireNuanceur("nuanceurSommets.glsl");
        if (chainesSommets != NULL)
        {
            GLuint nuanceurObj = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(nuanceurObj, 1, &chainesSommets, NULL);
            glCompileShader(nuanceurObj);
            glAttachShader(prog, nuanceurObj);
            ProgNuanceur::afficherLogCompile(nuanceurObj);
            delete[] chainesSommets;
        }
#if 1
      // partie 2:
      const GLchar *chainesGeometrie = ProgNuanceur::lireNuanceur( "nuanceurGeometrie.glsl" );
      if ( chainesGeometrie != NULL )
      {
         GLuint nuanceurObj = glCreateShader( GL_GEOMETRY_SHADER );
         glShaderSource( nuanceurObj, 1, &chainesGeometrie, NULL );
         glCompileShader( nuanceurObj );
         glAttachShader( prog, nuanceurObj );
         ProgNuanceur::afficherLogCompile( nuanceurObj );
         delete [] chainesGeometrie;
      }
#endif
        // attacher le nuanceur de fragments
        const GLchar *chainesFragments = ProgNuanceur::lireNuanceur("nuanceurFragments.glsl");
        if (chainesFragments != NULL)
        {
            GLuint nuanceurObj = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(nuanceurObj, 1, &chainesFragments, NULL);
            glCompileShader(nuanceurObj);
            glAttachShader(prog, nuanceurObj);
            ProgNuanceur::afficherLogCompile(nuanceurObj);
            delete[] chainesFragments;
        }

        // faire l'édition des liens du programme
        glLinkProgram(prog);
        ProgNuanceur::afficherLogLink(prog);

        // demander la "Location" des variables
        if ((locVertex = glGetAttribLocation(prog, "Vertex")) == -1)
            std::cerr << "!!! pas trouvé la \"Location\" de Vertex" << std::endl;
        if ((locColor = glGetAttribLocation(prog, "Color")) == -1)
            std::cerr << "!!! pas trouvé la \"Location\" de Color" << std::endl;
        if ((locmatrModel = glGetUniformLocation(prog, "matrModel")) == -1)
            std::cerr << "!!! pas trouvé la \"Location\" de matrModel" << std::endl;
        if ((locmatrVisu = glGetUniformLocation(prog, "matrVisu")) == -1)
            std::cerr << "!!! pas trouvé la \"Location\" de matrVisu" << std::endl;
        if ((locmatrProj = glGetUniformLocation(prog, "matrProj")) == -1)
            std::cerr << "!!! pas trouvé la \"Location\" de matrProj" << std::endl;
        if ((locplanCoupe = glGetUniformLocation(prog, "planCoupe")) == -1)
            std::cerr << "!!! pas trouvé la \"Location\" de planCoupe" << std::endl;
        if ((loccoulProfondeur = glGetUniformLocation(prog, "coulProfondeur")) == -1)
            std::cerr << "!!! pas trouvé la \"Location\" de coulProfondeur" << std::endl;
    }
}

void FenetreTP::initialiser()
{
    // donner la couleur de fond
    glClearColor(0.1, 0.1, 0.1, 1.0);

    // activer les etats openGL
    glEnable(GL_DEPTH_TEST);

    // activer le mélange de couleur pour la transparence
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // charger les nuanceurs
    chargerNuanceurs();
    glUseProgram(prog);

    // les valeurs à utiliser pour tracer le quad
    const GLfloat taille = Jupiter.distance + Callisto.distance + Callisto.rayon;
    GLfloat coo[] = {-taille, taille, 0,
                     taille, taille, 0,
                     taille, -taille, 0,
                     -taille, -taille, 0};
    const GLuint connec[] = {0, 1, 2, 2, 3, 0};

    // partie 1: initialiser le VAO (quad)
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    // partie 1: créer les deux VBO pour les sommets et la connectivité
    // ...
    glGenBuffers(2, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(coo), coo, GL_STATIC_DRAW);
    glVertexAttribPointer(locVertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(locVertex);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(connec), connec, GL_STATIC_DRAW);
    glBindVertexArray(0);

    // ...

    // construire le graphe de scène
    Soleil.ajouteEnfant(Terre);
    Terre.ajouteEnfant(Lune);

    Soleil.ajouteEnfant(Mars);
    Mars.ajouteEnfant(Phobos);
    Mars.ajouteEnfant(Deimos);

    Soleil.ajouteEnfant(Jupiter);
    Jupiter.ajouteEnfant(Io);
    Jupiter.ajouteEnfant(Europa);
    Jupiter.ajouteEnfant(Ganymede);
    Jupiter.ajouteEnfant(Callisto);

    // créer quelques autres formes
    cube = new FormeCube(1.5);
    sphere = new FormeSphere(1.0, 16, 16);
    theiere = new FormeTheiere();
    toreTerre = new FormeTore(0.08, Terre.distance, 8, 200);
    toreMars = new FormeTore(0.08, Mars.distance, 8, 200);
    toreJupiter = new FormeTore(0.08, Jupiter.distance, 8, 200);
}

void FenetreTP::conclure()
{
    delete cube;
    delete sphere;
    delete theiere;
    delete toreTerre;
    delete toreMars;
    delete toreJupiter;
    glDeleteBuffers(2, vbo);
    glDeleteVertexArrays(1, &vao);
}

void afficherQuad(GLfloat alpha) // le plan qui ferme les solides
{
    glVertexAttrib4f(locColor, 1.0, 1.0, 1.0, alpha);
    // afficher le plan tourné selon l'angle courant et à la position courante
    // partie 1: modifs ici ...
    // ...
    matrModel.PushMatrix();
    {
        matrModel.Translate(0.0, 0.0, -etat.planCoupe.w);
        matrModel.Rotate(etat.angleCoupe, 0.0, 1.0, 0.0);
        glUniformMatrix4fv(locmatrModel, 1, GL_FALSE, matrModel);
        glEnable(GL_BLEND);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glDisable(GL_BLEND);
    }
    matrModel.PopMatrix();
}

void afficherModele()
{
    glVertexAttrib4f(locColor, 1.0, 1.0, 1.0, 1.0);

#if 1
    // afficher les deux tores pour identifier les orbites des planetes
    glVertexAttrib3f(locColor, 0.0, 0.0, 1.0);
    toreTerre->afficher();
    glVertexAttrib3f(locColor, 0.0, 1.0, 0.0);
    toreMars->afficher();
    glVertexAttrib3f(locColor, 1.0, 0.0, 0.0);
    toreJupiter->afficher();
#endif

    // afficher le système solaire en commençant à la racine
    Soleil.afficher();
}

void FenetreTP::afficherScene()
{
    // effacer l'ecran et le tampon de profondeur et le stencil
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glUseProgram(progBase);

    // définir le pipeline graphique
    matrProj.Perspective(50.0, (GLdouble)largeur_ / (GLdouble)hauteur_, 0.1, 100.0);
    glUniformMatrix4fv(locmatrProjBase, 1, GL_FALSE, matrProj);

    camera.definir();
    glUniformMatrix4fv(locmatrVisuBase, 1, GL_FALSE, matrVisu);

    matrModel.LoadIdentity();
    glUniformMatrix4fv(locmatrModelBase, 1, GL_FALSE, matrModel);

    // afficher les axes
    if (etat.afficheAxes)
        FenetreTP::afficherAxes();

    // dessiner la scène
    glUseProgram(prog);
    glUniformMatrix4fv(locmatrProj, 1, GL_FALSE, matrProj);
    glUniformMatrix4fv(locmatrVisu, 1, GL_FALSE, matrVisu);
    glUniformMatrix4fv(locmatrModel, 1, GL_FALSE, matrModel);
    glUniform4fv(locplanCoupe, 1, glm::value_ptr(etat.planCoupe));
    glUniform1i(loccoulProfondeur, etat.coulProfondeur);

    if (etat.modeSelection)
    {
        glFinish();

        GLint cloture[4];
        glGetIntegerv(GL_VIEWPORT, cloture);
        GLint posX = etat.sourisPosPrec.x, posY = cloture[3] - etat.sourisPosPrec.y;

        glReadBuffer(GL_BACK);

        GLubyte couleur[3];
        glReadPixels(posX, posY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, couleur);

        verifierSelectionCorpsCelestres(couleur, Soleil.enfants);
        /*if (couleur[0] == 1.0 && couleur[1] == 1.0 && couleur[2] == 0.0)
            Soleil.estSelectionne = true;
        else if (couleur[0] == 0.5 && couleur[1] == 0.5 && couleur[2] == 1.0)
            Terre.estSelectionne = true;
        else if (couleur[0] == 0.6 && couleur[1] == 0.6 && couleur[2] == 0.6)
            Lune.estSelectionne = true;
        else if (couleur[0] == 0.6 && couleur[1] == 1.0 && couleur[2] == 0.5)
            Mars.estSelectionne = true;
        else if (couleur[0] == 0.4 && couleur[1] == 0.4 && couleur[2] == 0.8)
            Phobos.estSelectionne = true;
        else if (couleur[0] == 0.5 && couleur[1] == 0.5 && couleur[2] == 0.1)
            Deimos.estSelectionne = true;
        else if (couleur[0] == 1.0 && couleur[1] == 0.5 && couleur[2] == 0.5)
            Jupiter.estSelectionne = true;
        else if (couleur[0] == 0.7 && couleur[1] == 0.4 && couleur[2] == 0.5)
            Io.estSelectionne = true;
        else if (couleur[0] == 0.8 && couleur[1] == 0.4 && couleur[2] == 0.8)
            Europa.estSelectionne = true;
        else if (couleur[0] == 0.3 && couleur[1] == 0.6 && couleur[2] == 0.1)
            Ganymede.estSelectionne = true;
        else if (couleur[0] == 0.7 && couleur[1] == 0.5 && couleur[2] == 0.1)
            Callisto.estSelectionne = true;*/
    }
    else
    {
        // afficher le modèle et tenir compte du stencil et du plan de coupe
        // partie 1: modifs ici ...
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 1, 1);
        glStencilOp(GL_INCR, GL_INCR, GL_INCR);
        glEnable(GL_CLIP_PLANE0);
        // TODO
        afficherModele();
        glDisable(GL_CLIP_PLANE0);

        glStencilFunc(GL_EQUAL, 1, 1);
        afficherQuad(1.0);
        // en plus, dessiner le plan en transparence pour bien voir son étendue
        afficherQuad(0.25);
        glDisable(GL_STENCIL_TEST);
    }
}

void verifierSelectionCorpsCelestres(GLubyte couleurSel[3], std::vector<CorpsCeleste *> enfants) {
    std::vector<CorpsCeleste *>::iterator it;
    for (it = enfants.begin(); it != enfants.end(); it++)
    {
        if (couleurSel[0] == (*it)->couleur.r && couleurSel[1] == (*it)->couleur.g && couleurSel[2] == (it*)->couleur.b) {
            (*it)->estSelectionne = true;
            break;
        } else {
            verifierSelectionCorpsCelestres(couleurSel, (it*)->enfants);
        }
    }
}

void FenetreTP::redimensionner(GLsizei w, GLsizei h)
{
    glViewport(0, 0, w, h);
}

void FenetreTP::clavier(TP_touche touche)
{
    switch (touche)
    {
    case TP_ECHAP:
    case TP_q: // Quitter l'application
        quit();
        break;

    case TP_x: // Activer/désactiver l'affichage des axes
        etat.afficheAxes = !etat.afficheAxes;
        std::cout << "// Affichage des axes ? " << (etat.afficheAxes ? "OUI" : "NON") << std::endl;
        break;

    case TP_v: // Recharger les fichiers des nuanceurs et recréer le programme
        chargerNuanceurs();
        std::cout << "// Recharger nuanceurs" << std::endl;
        break;

    case TP_ESPACE: // Mettre en pause ou reprendre l'animation
        etat.enmouvement = !etat.enmouvement;
        break;

    case TP_g: // Permuter l'affichage en fil de fer ou plein
        etat.modePolygone = (etat.modePolygone == GL_FILL) ? GL_LINE : GL_FILL;
        glPolygonMode(GL_FRONT_AND_BACK, etat.modePolygone);
        break;

    case TP_m: // Choisir le modèle: 1-sphère, 2-cube, 3-théière (déjà implanté)
        if (++etat.modele > 3)
            etat.modele = 1;
        std::cout << " etat.modele=" << etat.modele << std::endl;
        break;

    case TP_p: // Atténuer ou non la couleur selon la profondeur
        etat.coulProfondeur = !etat.coulProfondeur;
        std::cout << " etat.coulProfondeur=" << etat.coulProfondeur << std::endl;
        break;

    case TP_HAUT: // Déplacer le plan de coupe vers le haut
        etat.planCoupe.w += 0.1;
        std::cout << " etat.planCoupe.w=" << etat.planCoupe.w << std::endl;
        break;

    case TP_BAS: // Déplacer le plan de coupe vers le bas
        etat.planCoupe.w -= 0.1;
        std::cout << " etat.planCoupe.w=" << etat.planCoupe.w << std::endl;
        break;

    case TP_CROCHETDROIT:
    case TP_DROITE: // Augmenter l'angle du plan de coupe
        etat.angleCoupe += 0.5;
        etat.planCoupe.x = sin(glm::radians(etat.angleCoupe));
        etat.planCoupe.z = cos(glm::radians(etat.angleCoupe));
        std::cout << " etat.angleCoupe=" << etat.angleCoupe << std::endl;
        break;
    case TP_CROCHETGAUCHE:
    case TP_GAUCHE: // Diminuer l'angle du plan de coupe
        etat.angleCoupe -= 0.5;
        etat.planCoupe.x = sin(glm::radians(etat.angleCoupe));
        etat.planCoupe.z = cos(glm::radians(etat.angleCoupe));
        std::cout << " etat.angleCoupe=" << etat.angleCoupe << std::endl;
        break;

        // case TP_c: // Augmenter le facteur de réchauffement
        //    etat.facteurRechauffement += 0.05; if ( etat.facteurRechauffement > 1.0 ) etat.facteurRechauffement = 1.0;
        //    std::cout << " etat.facteurRechauffement=" << etat.facteurRechauffement << " " << std::endl;
        //    break;
        // case TP_f: // Diminuer le facteur de réchauffement
        //    etat.facteurRechauffement -= 0.05; if ( etat.facteurRechauffement < 0.0 ) etat.facteurRechauffement = 0.0;
        //    std::cout << " etat.facteurRechauffement=" << etat.facteurRechauffement << " " << std::endl;
        //    break;

    case TP_PLUS: // Incrémenter la distance de la caméra
    case TP_EGAL:
        camera.dist--;
        std::cout << " camera.dist=" << camera.dist << std::endl;
        break;

    case TP_SOULIGNE:
    case TP_MOINS: // Décrémenter la distance de la caméra
        camera.dist++;
        std::cout << " camera.dist=" << camera.dist << std::endl;
        break;

    default:
        std::cout << " touche inconnue : " << (char)touche << std::endl;
        imprimerTouches();
        break;
    }
}

static bool pressed = false;
void FenetreTP::sourisClic(int button, int state, int x, int y)
{
    pressed = (state == TP_PRESSE);
    if (pressed)
    {
        switch (button)
        {
        default:
        case TP_BOUTON_GAUCHE: // Modifier le point de vue
            etat.modeSelection = false;
            break;
        case TP_BOUTON_DROIT: // Sélectionner des objets
            etat.modeSelection = true;
            break;
        }
        etat.sourisPosPrec.x = x;
        etat.sourisPosPrec.y = y;
    }
    else
    {
        etat.modeSelection = false;
        Soleil.estSelectionne = false;
        Terre.estSelectionne = false;
        Lune.estSelectionne = false;
        Mars.estSelectionne = false;
        Phobos.estSelectionne = false;
        Deimos.estSelectionne = false;
        Jupiter.estSelectionne = false;
        Io.estSelectionne = false;
        Europa.estSelectionne = false;
        Ganymede.estSelectionne = false;
        Callisto.estSelectionne = false;
    }
}

void FenetreTP::sourisWheel(int x, int y) // Déplacer le plan de coupe
{
    const int sens = +1;
    etat.planCoupe.w += 0.02 * sens * y;
    std::cout << " etat.planCoupe.w=" << etat.planCoupe.w << std::endl;
}

void FenetreTP::sourisMouvement(int x, int y)
{
    if (pressed)
    {
        if (!etat.modeSelection)
        {
            int dx = x - etat.sourisPosPrec.x;
            int dy = y - etat.sourisPosPrec.y;
            camera.theta -= dx / 3.0;
            camera.phi -= dy / 3.0;
        }

        etat.sourisPosPrec.x = x;
        etat.sourisPosPrec.y = y;

        camera.verifierAngles();
    }
}

int main(int argc, char *argv[])
{
    // créer une fenêtre
    FenetreTP fenetre("INF2705 TP");

    // allouer des ressources et définir le contexte OpenGL
    fenetre.initialiser();

    bool boucler = true;
    while (boucler)
    {
        // mettre à jour la physique
        calculerPhysique();

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
