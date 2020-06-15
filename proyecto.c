/*##########################################################################################
############################################################################################
###                                                                                      ###
###    Codigo extraído originalmente de www.librorealidadaumentada.com                   ###
###                                                                                      ###
###    Modificado por LYDIA PRADO IBAÑEZ para la entrega practica de la asignatura       ###
###    RVRA del Master en Ingeneria Informatica de la Escuela Superior de Informatica    ###
###    de Ciudad Real (UCLM)                                                             ###
###                                                                                      ###
############################################################################################
##########################################################################################*/

#include <math.h>
#include <GL/glut.h>
#include <AR/gsub.h>
#include <AR/video.h>
#include <AR/param.h>
#include <AR/ar.h>

// ==== Definicion de constantes y variables globales ===============
int patt_id;             // Identificador unico de la marca
double patt_trans[3][4]; // Matriz de transformacion de la marca
int flag = 0;            //Cambia entre el cubo y el toro
double distancia;        //Distancia entre las marcas
float tamF = 80.0;       //Variable que guarda el tamaño final de los objetos

// ==== Definicion de estructuras ===================================
struct TObject
{
  int id;                  // Identificador del patron
  int visible;             // Es visible el objeto?
  double width;            // Ancho del patron
  double center[2];        // Centro del patron
  double patt_trans[3][4]; // Matriz asociada al patron
  void (*drawme)(void);    // Puntero a funcion drawme
};

struct TObject *objects = NULL;
int nobjects = 0;

void print_error(char *error)
{
  printf("%s\n", error);
  exit(0);
}

// ==== addObject (Anade objeto a la lista de objetos) ==============
void addObject(char *p, double w, double c[2], void (*drawme)(void))
{
  int pattid;

  if ((pattid = arLoadPatt(p)) < 0)
    print_error("Error en carga de patron\n");

  nobjects++;
  objects = (struct TObject *)
      realloc(objects, sizeof(struct TObject) * nobjects);

  objects[nobjects - 1].id = pattid;
  objects[nobjects - 1].width = w;
  objects[nobjects - 1].center[0] = c[0];
  objects[nobjects - 1].center[1] = c[1];
  objects[nobjects - 1].drawme = drawme;
}
// ======== cleanup =================================================
static void cleanup(void)
{
  arVideoCapStop(); // Libera recursos al salir ...
  arVideoClose();
  argCleanup();
}

// ======== draw ====================================================
static void draw(void)
{
  double gl_para[16];                                     // Esta matriz 4x4 es la usada por OpenGL
  GLfloat color[] = {1.0, 1.0, 0.0, 1.0};                 // Color inicial de las figuras
  GLfloat light_position[] = {100.0, -200.0, 200.0, 0.0}; // Iluminacion de las figuras
  double m[3][4], m2[3][4];
  int i, wire = 0; // wire indica si la figura se tiene que dibujar solida o con cables
  float angle = 0.0, module = 0.0;
  double v[3];
  float tam = 0.0; //Tamaño local al método que tiene el objeto

  argDrawMode3D();              // Cambiamos el contexto a 3D
  argDraw3dCamera(0, 0);        // Y la vista de la camara a 3D
  glClear(GL_DEPTH_BUFFER_BIT); // Limpiamos buffer de profundidad
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  if ((objects[0].visible))
  {
    argConvGlpara(objects[0].patt_trans, gl_para);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(gl_para); // Cargamos su matriz de transf.

    if (objects[1].visible == 1)
    {
      // Calculamos el angulo de rotacion de la segunda marca
      v[0] = objects[1].patt_trans[0][0];
      v[1] = objects[1].patt_trans[1][0];
      v[2] = objects[1].patt_trans[2][0];

      module = sqrt(pow(v[0], 2) + pow(v[1], 2) + pow(v[2], 2));
      v[0] = v[0] / module;
      v[1] = v[1] / module;
      v[2] = v[2] / module;
      angle = acos(v[0]) * 57.2958; // Se obtiene un ángulo de entre 0º y 180º

      //Define qué figura se muestra en función del ángulo de la marca pequeña
      if (angle >= 0 && angle < 90) //Se pinta un cono
        flag = 1;
      if (angle >= 90 && angle < 180) //Se pinta un toro
        flag = 0;

      arUtilMatInv(objects[0].patt_trans, m);
      arUtilMatMul(m, objects[1].patt_trans, m2);
      //Se calcula la distancia que existe entre las dos marcas
      distancia = sqrt(pow(m2[0][3], 2) + pow(m2[1][3], 2) + pow(m2[2][3], 2));
      tam = (320 - distancia) / 160.0;
      // Para evitar que al alejar demasiado las marcas, la distancia de un número negativo
      // y las figuras se muestren dadas la vuelta
      if (tam < 0) // El tamaño de las figuras nunca va a ser inferior a 0
        tamF = 0;
      else // Se escala la figura un 150% del tamaño obtenido con la fórmula de la distancia
        tamF = tam * 150;
      printf("Distancia entre las marcas = %G\n", tamF);
    }
    else if (objects[1].visible == 0)
    {
      // Cambia el color de las figuras a violeta si la marca pequeña tiene oclusión
      color[0] = 1.0;
      color[1] = 0.0;
      color[2] = 1.0;
      color[3] = 0.0;
      wire = 1;
    }

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    // Si la marca pequeña tiene una rotación de entre 0º y 90º, se dibuja un cono
    if (flag == 1)
    {
      glMaterialfv(GL_FRONT, GL_AMBIENT, color);
      if (wire == 1) // Si detecta oclusión, el cono se dibuja con cables
        glutWireCone(tamF, tamF * 2, 100, 100);
      else // Si no decta oclusión, el cono se dibuja sólido
        glutSolidCone(tamF, tamF * 2, 100, 100);
      // Si la marca pequeña tiene una rotación de entre 90º y 180ª, se dibuja un toro
    }
    else
    {
      glMaterialfv(GL_FRONT, GL_AMBIENT, color);
      if (wire == 1) // Si detecta oclusión, el toro se dibuja con cables
        glutWireTorus(tamF / 3, tamF / 1.5, 100, 100);
      else // Si no decta oclusión, el toro se dibuja sólido
        glutSolidTorus(tamF / 3, tamF / 1.5, 100, 100);
    }
  }
  glDisable(GL_DEPTH_TEST);
}

// ======== init ====================================================
static void init(void)
{
  ARParam wparam, cparam;   // Parametros intrinsecos de la camara
  int xsize, ysize;         // Tamano del video de camara (pixels)
  double c[2] = {0.0, 0.0}; // Centro de patron (por defecto)

  // Abrimos dispositivo de video
  if (arVideoOpen("-dev=/dev/video1") < 0)
    exit(0);
  if (arVideoInqSize(&xsize, &ysize) < 0)
    exit(0);

  // Cargamos los parametros intrinsecos de la camara
  if (arParamLoad("data/camera_para.dat", 1, &wparam) < 0)
    print_error("Error en carga de parametros de camara\n");

  arParamChangeSize(&wparam, xsize, ysize, &cparam);
  arInitCparam(&cparam); // Inicializamos la camara con "cparam"

  // Inicializamos la lista de objetos
  addObject("data/simple.patt", 120.0, c, NULL);
  addObject("data/identic.patt", 90.0, c, NULL);

  argInit(&cparam, 1.0, 0, 0, 0, 0); // Abrimos la ventana
}

// ======== mainLoop ================================================
static void mainLoop(void)
{
  ARUint8 *dataPtr;
  ARMarkerInfo *marker_info;
  int marker_num, i, j, k;
  int vis = 0; // Controla el numero de marcas que detecta visibles

  // Capturamos un frame de la camara de video
  if ((dataPtr = (ARUint8 *)arVideoGetImage()) == NULL)
  {
    // Si devuelve NULL es porque no hay un nuevo frame listo
    arUtilSleep(2);
    return; // Dormimos el hilo 2ms y salimos
  }

  argDrawMode2D();
  argDispImage(dataPtr, 0, 0); // Dibujamos lo que ve la camara

  // Detectamos la marca en el frame capturado (return -1 si error)
  if (arDetectMarker(dataPtr, 100, &marker_info, &marker_num) < 0)
  {
    cleanup();
    exit(0); // Si devolvio -1, salimos del programa!
  }

  arVideoCapNext(); // Frame pintado y analizado... A por otro!

  // Vemos donde detecta el patron con mayor fiabilidad
  for (i = 0; i < nobjects; i++)
  {
    for (j = 0, k = -1; j < marker_num; j++)
    {
      if (objects[i].id == marker_info[j].id)
      {
        if (k == -1)
        {
          k = j;
          vis++;
        }
        else if (marker_info[k].cf < marker_info[j].cf)
        {
          k = j;
          vis++;
        }
      }
    }
    // Establece si la marca es o no visible
    if (k != -1)
    {
      arGetTransMat(&marker_info[k], objects[i].center, objects[i].width, objects[i].patt_trans);
      if (vis == 1 && i == 0)
      { // Si solo ha detectado una marca y es la primera iteracion, se hace visible la primera marca
        objects[0].visible = 1;
        objects[1].visible = 0;
      }
      else if (vis == 1 && i == 1)
      { // Si solo ha detectado una marca y es la segunda iteracion, se hace visible la segunda marca
        objects[0].visible = 0;
        objects[1].visible = 1;
      }
      else if (vis == 2)
      { // Si detecta las dos marcas, se hacen visibles las dos marcas
        objects[0].visible = 1;
        objects[1].visible = 1;
      }
    }
    else
    {
      objects[i].visible = 0;
    } // El objeto no es visible
  }

  draw();           // Dibujamos los objetos de la escena
  argSwapBuffers(); // Cambiamos el buffer con lo que tenga dibujado
}

// ======== Main ====================================================
int main(int argc, char **argv)
{
  glutInit(&argc, argv); // Creamos la ventana OpenGL con Glut
  init();                // Llamada a nuestra funcion de inicio

  arVideoCapStart();                 // Creamos un hilo para captura de video
  argMainLoop(NULL, NULL, mainLoop); // Asociamos callbacks...
  return (0);
}
