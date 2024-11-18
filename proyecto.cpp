/*
* 
* 11 - Animación de Partículas
*/

#include <iostream>
#include <stdlib.h>

// GLAD: Multi-Language GL/GLES/EGL/GLX/WGL Loader-Generator
// https://glad.dav1d.de/
#include <glad/glad.h>

// GLFW: https://www.glfw.org/
#include <GLFW/glfw3.h>

// GLM: OpenGL Math library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Model loading classes
#include <shader_m.h>
#include <camera.h>
#include <model.h>
#include <material.h>
#include <light.h>
#include <cubemap.h>
#include <particles.h>

#include <irrKlang.h>
using namespace irrklang;

// Functions
bool Start();
bool Update();
void dibujarSistemaSolar(glm::mat4 projection, glm::mat4 view);

// Definición de callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// Gobals
GLFWwindow* window;

// Tamaño en pixeles de la ventana
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

// Definición de cámara (posición en XYZ)
Camera camera(glm::vec3(0.0f, 8.0f, 10.0f));
Camera camera3rd(glm::vec3(0.0f, 0.0f, 0.0f));

// Definicion de fuentes de Luz
Light light01, light02, light03, light04;

// Controladores para el movimiento del mouse
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Variables de control para movimiebto de planetas
float timeScale;        // Escala de tiempo para acelerar la simulación
float orbitalPeriod;    // Periodo orbital en días
float semiMajorAxis;	// Semieje mayor en AU (Tierra a 1 AU del Sol)
float eccentricity;     // Excentricidad de la órbita de la Tierra
glm::vec3 centerPosition(0.0f, 0.0f, 0.0f); // Posición del Sol en el centro

// Variables para control del paso del tiempo
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float elapsedTime = 0.0f;

// Variables de control para la vista
float     rotateCharacter = 0.0f;
glm::vec3 position(-18.0f, 1.5f, 0.0f);
glm::vec3 forwardView(1.0f, 0.0f, 0.0f);

// Variable que controla cual constelaciones observamos
int constelacion = 0;

// Shaders
Shader *staticShader;
Shader *cubemapShader;
Shader *particlesShader;
Shader* keplerShader;
Shader* fresnelShader;
Shader *mLightsShader;

// Modelos del sistema solar
Model* sol;
Model* mercurio;
Model* venus;
Model* tierra;
Model* marte;
Model* jupiter;
Model* saturno;
Model* urano;
Model* neptuno;

// Modelos de las constaleacione
Model* acuario;
Model* aries;
Model* cancer;
Model* coronaBorealis;
Model* corvus;
Model* cygnus;
Model* gemini;
Model* leo;
Model* libra;
Model* phoenix;
Model* pyxis;
Model* virgo;

// Modelos del museo
Model* museoDomo;
Model* museoPuerta01;
Model* museoPuerta02;
Model* museoPuertaInt;
Model* museoEstructura;
Model* museoVitrina;

// Elementos del mobiliario
Model* consola;

// Skybox
Model* skybox;

// Audio
ISoundEngine *SoundEngine = createIrrKlangDevice();

// Materiales
Material material;

//Vector de luces
std::vector<Light> gLights;
Model* luz;

// Arreglos para el dibujado de los planetas
Model* planetas[] = { mercurio,	venus,	tierra,	 marte,  jupiter, saturno,	urano,   neptuno };
float ejesMayores[] = { 10.0f,	15.0f,	20.f,	 25.0f,  35.0f,	  45.0f,	55.0f,   75.0f };
float periodOrbital[] = { 176.0f,	225.0f, 365.25f, 687.0f, 833.0f,  1059.0f,	1687.0f, 1820.0f };
float excentricidad[] = { 0.205f,	0.007f, 0.017f,	 0.093f, 0.049f,  0.056f,	0.046f,  0.010f };
float escalaTiempo[] = { 87.69f,	28.5f,	36.525f, 48.7f,	 11.86f,  29.46f,	84.01f,  164.79f };

// selección de cámara
bool    activeCamera = 0; // activamos la primera cámara

// Entrada a función principal
int main()
{
	if (!Start())
		return -1;

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		if (!Update())
			break;
	}

	glfwTerminate();
	return 0;

}

bool Start() {
	// Inicialización de GLFW

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Creación de la ventana con GLFW
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Planetario Charly", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Ocultar el cursor mientras se rota la escena
	// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: Cargar todos los apuntadores
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	// Activación de buffer de profundidad
	glEnable(GL_DEPTH_TEST);

	// Compilación y enlace de shaders
	/*
	cubemapShader = new Shader("shaders/10_vertex_cubemap.vs", "shaders/10_fragment_cubemap.fs");
	particlesShader	= new Shader("shaders/13_particles.vs", "shaders/13_particles.fs");*/
	mLightsShader   = new Shader("shaders/11_PhongShaderMultLights.vs", "shaders/11_PhongShaderMultLights.fs");
	staticShader	= new Shader("shaders/10_vertex_simple.vs",			"shaders/10_fragment_simple.fs");
	keplerShader	= new Shader("shaders/kepler.vs",					"shaders/kepler.fs");
	fresnelShader	= new Shader("shaders/11_Fresnel.vs",				"shaders/11_Fresnel.fs");

	// Modelos del sistema solar
	sol		=	new Model("models/Proyecto/Planetas/sol.fbx");
	mercurio=	new Model("models/Proyecto/Planetas/mercurio.fbx");
	venus	=	new Model("models/Proyecto/Planetas/venus.fbx");
	tierra	=	new Model("models/Proyecto/Planetas/tierra.fbx");
	marte	=	new Model("models/Proyecto/Planetas/marte.fbx");
	jupiter =	new Model("models/Proyecto/Planetas/jupiter.fbx");
	saturno =	new Model("models/Proyecto/Planetas/saturno.fbx");
	urano	=	new Model("models/Proyecto/Planetas/urano.fbx");
	neptuno =	new Model("models/Proyecto/Planetas/neptuno.fbx");
	
	// Modelos del museo
	museoDomo		= new Model("models/Proyecto/Museo/museoFinal-Domo.fbx");
	museoPuerta01	= new Model("models/Proyecto/Museo/museoFinal-Door1.fbx");
	museoPuerta02	= new Model("models/Proyecto/Museo/museoFinal-Door2.fbx");
	museoPuertaInt	= new Model("models/Proyecto/Museo/museoFinal-DoorInterior.fbx");
	museoEstructura	= new Model("models/Proyecto/Museo/museoFinal-Estructura.fbx");
	museoVitrina	= new Model("models/Proyecto/Museo/museoFinal-Vitrina.fbx");

	// Modelos del mobiliario
	consola = new Model("models/Proyecto/Mobiliario/consola.fbx");

	// Modelos de las constelaciones
	acuario			= new Model("models/Proyecto/Constelaciones/acuario.fbx");
	aries			= new Model("models/Proyecto/Constelaciones/aries.fbx");
	cancer			= new Model("models/Proyecto/Constelaciones/cancer.fbx");
	coronaBorealis	= new Model("models/Proyecto/Constelaciones/coronaBorealis.fbx");
	corvus			= new Model("models/Proyecto/Constelaciones/corvus.fbx");
	cygnus			= new Model("models/Proyecto/Constelaciones/cygnus.fbx");
	gemini			= new Model("models/Proyecto/Constelaciones/gemini.fbx");
	leo				= new Model("models/Proyecto/Constelaciones/leo.fbx");
	libra			= new Model("models/Proyecto/Constelaciones/libra.fbx");
	phoenix			= new Model("models/Proyecto/Constelaciones/phoenix.fbx");
	pyxis			= new Model("models/Proyecto/Constelaciones/pyxis.fbx");
	virgo			= new Model("models/Proyecto/Constelaciones/virgo.fbx");

	// Skybox
	skybox = new Model("models/Proyecto/skybox.fbx");

	camera3rd.Position	=  position;
	camera3rd.Position.y+= 1.7f;
	camera3rd.Position	-= forwardView;
	camera3rd.Front		=  forwardView;

	SoundEngine->play2D("audios/bienvenida 1.mp3", true);

	// Configuración de luces

	luz = new Model("models/IllumModels/lightDummy.fbx");

	Light light01;
	light01.Position = glm::vec3(-12.0f, 11.0f, 12.0f);
	light01.Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	light01.Power = glm::vec4(5.0f, 5.0f, 5.0f, 1.0f);
	gLights.push_back(light01);

	Light light02;
	light02.Position = glm::vec3(-12.0f, 11.0f, -12.0f);
	light02.Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	light02.Power = glm::vec4(5.0f, 5.0f, 5.0f, 1.0f);
	gLights.push_back(light02);

	Light light03;
	light03.Position = glm::vec3(12.0f, 11.0f, -12.0f);
	light03.Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	light03.Power = glm::vec4(5.0f, 5.0f, 5.0f, 1.0f);
	gLights.push_back(light03);

	Light light04;
	light04.Position = glm::vec3(12.0f, 11.0f, 12.0f);
	light04.Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	light04.Power = glm::vec4(5.0f, 5.0f, 5.0f, 1.0f);
	gLights.push_back(light04);

	material.ambient = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	material.diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	material.specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	material.transparency = 1.0f;

	return true;
}

void SetLightUniformInt(Shader* shader, const char* propertyName, size_t lightIndex, int value) {
	std::ostringstream ss;
	ss << "allLights[" << lightIndex << "]." << propertyName;
	std::string uniformName = ss.str();

	shader->setInt(uniformName.c_str(), value);
}
void SetLightUniformFloat(Shader* shader, const char* propertyName, size_t lightIndex, float value) {
	std::ostringstream ss;
	ss << "allLights[" << lightIndex << "]." << propertyName;
	std::string uniformName = ss.str();

	shader->setFloat(uniformName.c_str(), value);
}
void SetLightUniformVec4(Shader* shader, const char* propertyName, size_t lightIndex, glm::vec4 value) {
	std::ostringstream ss;
	ss << "allLights[" << lightIndex << "]." << propertyName;
	std::string uniformName = ss.str();

	shader->setVec4(uniformName.c_str(), value);
}
void SetLightUniformVec3(Shader* shader, const char* propertyName, size_t lightIndex, glm::vec3 value) {
	std::ostringstream ss;
	ss << "allLights[" << lightIndex << "]." << propertyName;
	std::string uniformName = ss.str();

	shader->setVec3(uniformName.c_str(), value);
}

bool Update() {
	// Procesa la entrada del teclado o mouse
	processInput(window);

	// Renderizado R - G - B - A
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection;
	glm::mat4 view;

	if (activeCamera) {
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		view = camera.GetViewMatrix();
	}
	else {
		projection = glm::perspective(glm::radians(camera3rd.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		view = camera3rd.GetViewMatrix();
	}

	// Colocamos en la misma posición las variables de las camaras
	if (activeCamera)
	{
		camera3rd.Position.x = camera.Position.x;
		camera3rd.Position.y = camera.Position.y;
		camera3rd.Position.z = camera.Position.z; 
	}
	else
	{
		camera.Position.x = camera3rd.Position.x;
		camera.Position.y = camera3rd.Position.y;
		camera.Position.z = camera3rd.Position.z;
	}

	// Skymap (fondo)
	{
		staticShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		staticShader->setMat4("projection", projection);
		staticShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(100.0f, 100.0f, 1000.0f));	// it's a bit too big for our scene, so scale it down
		staticShader->setMat4("model", model);
		skybox->Draw(*staticShader);
	}
	
	glUseProgram(0);
	
	// Configuramos propiedades de fuentes de luz
	{
		mLightsShader->use();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		mLightsShader->setMat4("projection", projection);
		mLightsShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		mLightsShader->setMat4("model", model);

		mLightsShader->setInt("numLights", (int)gLights.size());

		for (size_t i = 0; i < gLights.size(); ++i)
		{
			SetLightUniformVec3(mLightsShader, "Position", i, gLights[i].Position);
			SetLightUniformVec3(mLightsShader, "Direction", i, gLights[i].Direction);
			SetLightUniformVec4(mLightsShader, "Color", i, gLights[i].Color);
			SetLightUniformVec4(mLightsShader, "Power", i, gLights[i].Power);
			SetLightUniformInt(mLightsShader, "alphaIndex", i, gLights[i].alphaIndex);
			SetLightUniformFloat(mLightsShader, "distance", i, gLights[i].distance);
		}

		mLightsShader->setVec3("eye", camera.Position);

		mLightsShader->setVec4("MaterialAmbientColor", material.ambient);
		mLightsShader->setVec4("MaterialDiffuseColor", material.diffuse);
		mLightsShader->setVec4("MaterialSpecularColor", material.specular);
		mLightsShader->setFloat("transparency", material.transparency);

		museoEstructura->Draw(*mLightsShader);

		
		/*   --    Light Dummy para realizar pruebas de luz
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(12.0f, 10.0f, 12.0f)); 
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	
		mLightsShader->setMat4("model", model);

		luz->Draw(*mLightsShader); */
	}

	glUseProgram(0);

	// Dibujado del museo
	{
		fresnelShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		fresnelShader->setMat4("projection", projection);
		fresnelShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		fresnelShader->setMat4("model", model);

		museoDomo->Draw(*fresnelShader);
		glUseProgram(0);
		
		staticShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		staticShader->setMat4("projection", projection);
		staticShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		staticShader->setMat4("model", model);
		
		//Puertas
		//museoPuerta01->Draw(*staticShader);
		//museoPuerta02->Draw(*staticShader);
		//museoPuertaInt->Draw(*staticShader);
		museoVitrina->Draw(*staticShader);
		consola->Draw(*staticShader);
	}

	glUseProgram(0);

	// Dibujado de las constelaciones
	{
		staticShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		staticShader->setMat4("projection", projection);
		staticShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -5.0f, 0.0f)); 
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	
		staticShader->setMat4("model", model);

		switch (constelacion)
		{
			case 1:
				acuario->Draw(*staticShader);
				break;
			case 2:
				aries->Draw(*staticShader);
				break;
			case 3:
				cancer->Draw(*staticShader);
				break;
			case 4:
				coronaBorealis->Draw(*staticShader);
				break;
			case 5:
				corvus->Draw(*staticShader);
				break;
			case 6:
				cygnus->Draw(*staticShader);
				break;
			case 7:
				gemini->Draw(*staticShader);
				break;
			case 8:
				leo->Draw(*staticShader);
				break;
			case 9:
				libra->Draw(*staticShader);
				break;
			case 10:
				phoenix->Draw(*staticShader);
				break;
			case 11:
				pyxis->Draw(*staticShader);
				break;
			case 12:
				virgo->Draw(*staticShader);
				break;
			default:
				dibujarSistemaSolar(projection, view);
				break;
		}
	}

	glUseProgram(0);

	// glfw: swap buffers 
	glfwSwapBuffers(window);
	glfwPollEvents();

	return true;
}

// Procesamos entradas del teclado
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);


	// camera movement
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {

		position = position + 0.010F * forwardView;
		camera3rd.Front = forwardView;
		camera3rd.ProcessKeyboard(FORWARD, deltaTime);
		camera3rd.Position = position;
		camera3rd.Position.y += 1.7f;
		camera3rd.Position -= forwardView;

	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		position = position - 0.010F * forwardView;
		camera3rd.Front = forwardView;
		camera3rd.ProcessKeyboard(BACKWARD, deltaTime);
		camera3rd.Position = position;
		camera3rd.Position.y += 1.7f;
		camera3rd.Position -= forwardView;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		rotateCharacter += 0.5f;

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(rotateCharacter), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec4 viewVector = model * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		forwardView = glm::vec3(viewVector);
		forwardView = glm::normalize(forwardView);

		camera3rd.Front = forwardView;
		camera3rd.Position = position;
		camera3rd.Position.y += 1.7f;
		camera3rd.Position -= forwardView;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		rotateCharacter -= 0.5f;

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(rotateCharacter), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec4 viewVector = model * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		forwardView = glm::vec3(viewVector);
		forwardView = glm::normalize(forwardView);

		camera3rd.Front = forwardView;
		camera3rd.Position = position;
		camera3rd.Position.y += 1.7f;
		camera3rd.Position -= forwardView;
	}

	// Cambios de la camara
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		activeCamera = 0;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		activeCamera = 1;

	// Cambio de la constelacion seleccionada
	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
	{
		constelacion = 1;
		SoundEngine->stopAllSounds();
		SoundEngine->play2D("audios/acuario.mp3", true);
	}
	if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS)
	{
		constelacion = 2;
		SoundEngine->stopAllSounds();
		SoundEngine->play2D("audios/aries.mp3", true);
	}
	if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS)
	{
		constelacion = 3;
		SoundEngine->stopAllSounds();
		SoundEngine->play2D("audios/cancer.mp3", true);
	}
	if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS)
	{
		constelacion = 4;
		SoundEngine->stopAllSounds();
		SoundEngine->play2D("audios/coronaBorealis.mp3", true);
	}
	if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS)
	{
		constelacion = 5;
		SoundEngine->stopAllSounds();
		SoundEngine->play2D("audios/corvus.mp3", true);
	}
	if (glfwGetKey(window, GLFW_KEY_F6) == GLFW_PRESS)
	{
		constelacion = 6;
		SoundEngine->stopAllSounds();
		SoundEngine->play2D("audios/cygnus.mp3", true);
	}
	if (glfwGetKey(window, GLFW_KEY_F7) == GLFW_PRESS)
	{
		constelacion = 7;
		SoundEngine->stopAllSounds();
		SoundEngine->play2D("audios/gemini.mp3", true);
	}
	if (glfwGetKey(window, GLFW_KEY_F8) == GLFW_PRESS)
	{
		constelacion = 8;
		SoundEngine->stopAllSounds();
		SoundEngine->play2D("audios/leo.mp3", true);
	}
	if (glfwGetKey(window, GLFW_KEY_F9) == GLFW_PRESS)
	{
		constelacion = 9;
		SoundEngine->stopAllSounds();
		SoundEngine->play2D("audios/libra.mp3", true);
	}
	if (glfwGetKey(window, GLFW_KEY_F10) == GLFW_PRESS)
	{
		constelacion = 10;
		SoundEngine->stopAllSounds();
		SoundEngine->play2D("audios/phoenix.mp3", true);
	}
	if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS)
	{
		constelacion = 11;
		SoundEngine->stopAllSounds();
		SoundEngine->play2D("audios/pyxis.mp3", true);
	}
	if (glfwGetKey(window, GLFW_KEY_F12) == GLFW_PRESS)
	{
		constelacion = 12;
		SoundEngine->stopAllSounds();
		SoundEngine->play2D("audios/virgo.mp3", true);
	}
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		//Abrir puertas del museo
	{

	}


	// Desactiva las constelaciones cuando se sale del domo
	if (position.x > 6.0f)
	{
		SoundEngine->stopAllSounds();
		constelacion = 0;
	}
}

// glfw: Actualizamos el puerto de vista si hay cambios del tamaño
// de la ventana
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// glfw: Callback del movimiento y eventos del mouse
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos; 

	lastX = (float)xpos;
	lastY = (float)ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: Complemento para el movimiento y eventos del mouse
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll((float)yoffset);
}

// Funcion para dibujar el modelo del sistema solar
void dibujarSistemaSolar(glm::mat4 projection, glm::mat4 view)
{	
	// Dibujamos primero al sol en el centro, aprovechando el shader
	// estatico de la funcion principal

	// Activamos para objetos transparentes
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Aplicamos transformaciones de proyección y cámara (si las hubiera)
	staticShader->setMat4("projection", projection);
	staticShader->setMat4("view", view);

	// Aplicamos transformaciones del modelo
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 4.0f, 0.0f)); // translate it down so it's at the center of the scene
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.20f, 0.20f, 0.20f));	// it's a bit too big for our scene, so scale it down
	staticShader->setMat4("model", model);
	sol->Draw(*staticShader);
	glUseProgram(0);

	// Activamos el shader de kepler
	keplerShader->use();

	// Activamos para objetos transparentes
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Aplicamos transformaciones de proyección y cámara (si las hubiera)
	keplerShader->setMat4("projection", projection);
	keplerShader->setMat4("view", view);

	// Aplicamos transformaciones del modelo
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 4.0f, 0.0f)); // translate it down so it's at the center of the scene
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));	// it's a bit too big for our scene, so scale it down
	keplerShader->setMat4("model", model);
	
	
	/* Ciclo for para optimizacion en desarrollo
	// Usa los arreglos para dibujar el modelo del sistema solar
	for (int i = 0; i < 8; i++)
	{
		float time = static_cast<float>(glfwGetTime()); // Toma el tiempo actual para el calculo del movimiento
		keplerShader->setFloat("time", time);

		// Usa los valores de los arreglos para tomar las caracteristicas de cada planeta
		keplerShader->setFloat("semiMajorAxis", ejesMayores[i]);
		keplerShader->setFloat("orbitalPeriod", periodOrbital[i]);
		keplerShader->setFloat("eccentricity", excentricidad[i]);
		keplerShader->setFloat("timeScale", escalaTiempo[i]);
		planetas[i]->Draw(*keplerShader);
	};*/

	// Para Mercurio
	float time = static_cast<float>(glfwGetTime());
	semiMajorAxis = 10.0f;        // 1.00 AU
	orbitalPeriod = 176.0f;       // Periodo en días
	eccentricity = 0.205f;       // Excentricidad
	timeScale = 87.69f;          // Escala de tiempo

	keplerShader->setFloat("time", time);
	keplerShader->setFloat("semiMajorAxis", semiMajorAxis);
	keplerShader->setFloat("orbitalPeriod", orbitalPeriod);
	keplerShader->setFloat("eccentricity", eccentricity);
	keplerShader->setFloat("timeScale", timeScale);
	mercurio->Draw(*keplerShader);

	// Para Venus
	time = static_cast<float>(glfwGetTime());
	semiMajorAxis = 15.f;             // 1.69 AU
	orbitalPeriod = 225.0f;
	eccentricity = 0.007f;
	timeScale = 28.5f;

	keplerShader->setFloat("time", time);
	keplerShader->setFloat("semiMajorAxis", semiMajorAxis);
	keplerShader->setFloat("orbitalPeriod", orbitalPeriod);
	keplerShader->setFloat("eccentricity", eccentricity);
	keplerShader->setFloat("timeScale", timeScale);
	venus->Draw(*keplerShader);

	// Para Tierra
	time = static_cast<float>(glfwGetTime());
	semiMajorAxis = 20.0f;             // 2.59 AU
	orbitalPeriod = 365.25f;
	eccentricity = 0.017f;
	timeScale = 36.525f;

	keplerShader->setFloat("time", time);
	keplerShader->setFloat("semiMajorAxis", semiMajorAxis);
	keplerShader->setFloat("orbitalPeriod", orbitalPeriod);
	keplerShader->setFloat("eccentricity", eccentricity);
	keplerShader->setFloat("timeScale", timeScale);
	tierra->Draw(*keplerShader);

	// Para Marte
	time = static_cast<float>(glfwGetTime());
	semiMajorAxis = 25.0f;             // 3.90 AU
	orbitalPeriod = 687.0f;
	eccentricity = 0.093f;
	timeScale = 48.7f;

	keplerShader->setFloat("time", time);
	keplerShader->setFloat("semiMajorAxis", semiMajorAxis);
	keplerShader->setFloat("orbitalPeriod", orbitalPeriod);
	keplerShader->setFloat("eccentricity", eccentricity);
	keplerShader->setFloat("timeScale", timeScale);
	marte->Draw(*keplerShader);

	// Para Júpiter
	time = static_cast<float>(glfwGetTime());
	semiMajorAxis = 35.0f;            // 13.33 AU
	orbitalPeriod = 833.0f; // 4333.0f
	eccentricity = 0.049f;
	timeScale = 11.86f;

	keplerShader->setFloat("time", time);
	keplerShader->setFloat("semiMajorAxis", semiMajorAxis);
	keplerShader->setFloat("orbitalPeriod", orbitalPeriod);
	keplerShader->setFloat("eccentricity", eccentricity);
	keplerShader->setFloat("timeScale", timeScale);
	jupiter->Draw(*keplerShader);

	// Para Saturno
	time = static_cast<float>(glfwGetTime());
	semiMajorAxis = 45.0f;            // 24.58 AU
	orbitalPeriod = 1059.0f;		// 10759.0f
	eccentricity = 0.056f;
	timeScale = 29.46f;

	keplerShader->setFloat("time", time);
	keplerShader->setFloat("semiMajorAxis", semiMajorAxis);
	keplerShader->setFloat("orbitalPeriod", orbitalPeriod);
	keplerShader->setFloat("eccentricity", eccentricity);
	keplerShader->setFloat("timeScale", timeScale);
	saturno->Draw(*keplerShader);

	// Para Urano
	time = static_cast<float>(glfwGetTime());
	semiMajorAxis = 55.0f;            // 49.23 AU
	orbitalPeriod = 1687.0f;		// 30687.0
	eccentricity = 0.046f;
	timeScale = 84.01f;

	keplerShader->setFloat("time", time);
	keplerShader->setFloat("semiMajorAxis", semiMajorAxis);
	keplerShader->setFloat("orbitalPeriod", orbitalPeriod);
	keplerShader->setFloat("eccentricity", eccentricity);
	keplerShader->setFloat("timeScale", timeScale);
	urano->Draw(*keplerShader);

	// Para Neptuno
	time = static_cast<float>(glfwGetTime());
	semiMajorAxis = 75.0;            // 77.10 AU
	orbitalPeriod = 1820.0f;		 // 60200.0f
	eccentricity = 0.010f;
	timeScale = 164.79f;

	keplerShader->setFloat("time", time);
	keplerShader->setFloat("semiMajorAxis", semiMajorAxis);
	keplerShader->setFloat("orbitalPeriod", orbitalPeriod);
	keplerShader->setFloat("eccentricity", eccentricity);
	keplerShader->setFloat("timeScale", timeScale);
	neptuno->Draw(*keplerShader);
	glUseProgram(0);	
}
