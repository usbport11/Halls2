#include "stdafx.h"
#include "classes/system/Shader.h"
#include "classes/system/Scene.h"
#include "classes/buffers/StaticBuffer.h"
#include "classes/level/Cave.h"
#include "classes/level/Dungeon.h"

using namespace std;

double DeltaTime = 0;
bool Pause;
bool keys[1024] = {0};
int WindowWidth = 800, WindowHeight = 600;
bool EnableVsync = 1;
GLFWwindow* window = NULL;
GLFWmonitor* primaryMonitor = NULL;

int TilesCount[2] = {60, 60};//{30, 30}
glm::vec2 Edge(1, 1);//{2, 2}
glm::vec2 TileSize(10, 10);//(20, 20)
glm::vec2 MouseSceneCoord;

MShader Shader;
MScene Scene;

//MCave Cave = MCave(TilesCount[0], TilesCount[1], 35, 1, 4, 30, 30);
MDungeon Cave = MDungeon(TilesCount[0], TilesCount[1], 6, 20, 3);
MStaticBuffer RoomsBuffer;
MStaticBuffer LeafsBuffer;

bool GenerateLevel() {
	RoomsBuffer.Clear();
	
	glm::vec3 Color = glm::vec3(1, 1, 1);
	if(!Cave.Generate()) {
		LogFile<<"Level generate failed"<<endl; 
		return false;
	}
	for(int i=0; i<TilesCount[0]; i++) {
		for(int j=0; j<TilesCount[1]; j++) {
			if(!Cave.GetValue(i, j)) continue;
			RoomsBuffer.AddQuad(glm::vec2(i * TileSize.x + Edge.x, j * TileSize.y + Edge.y), glm::vec2((i+1) * TileSize.x - Edge.x, (j+1) * TileSize.y - Edge.y), Color);
		}
	}
	
	if(!RoomsBuffer.Dispose()) return false;
	
	return true;
}

static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void mousepos_callback(GLFWwindow* window, double x, double y) {
	MouseSceneCoord = Scene.WindowPosToWorldPos(x, y);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key >= sizeof(keys)) return;
	if(action == GLFW_PRESS) {
		keys[key] = 1;
	}
	else if(action == GLFW_RELEASE) {
		keys[key] = 0;
	}
}

bool InitApp() {
	LogFile<<"Starting application"<<endl;    
    glfwSetErrorCallback(error_callback);
    
    if(!glfwInit()) return false;
    window = glfwCreateWindow(WindowWidth, WindowHeight, "TestApp", NULL, NULL);
    if(!window) {
		LogFile << "Window: Can't create" << endl;
        glfwTerminate();
        return false;
    }
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mousepos_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwMakeContextCurrent(window);
    if(glfwExtensionSupported("WGL_EXT_swap_control")) {
    	LogFile<<"Window: V-Sync supported. V-Sync: "<<EnableVsync<<endl;
		glfwSwapInterval(EnableVsync);//0 - disable, 1 - enable
	}
	else LogFile<<"Window: V-Sync not supported"<<endl;
    LogFile<<"Window created: width: "<<WindowWidth<<" height: "<<WindowHeight<<endl;
	primaryMonitor = glfwGetPrimaryMonitor();
	if (!primaryMonitor) {
		LogFile << "Window: Can't get current monitor" << endl;
		glfwTerminate();
		return false;
	}

	//glew
	GLenum Error = glewInit();
	if(GLEW_OK != Error) {
		LogFile<<"Window: GLEW Loader error: "<<glewGetErrorString(Error)<<endl;
		return false;
	}
	LogFile<<"GLEW initialized"<<endl;
	
	if(!CheckOpenglSupport()) return false;

	//shaders
	if(!Shader.CreateShaderProgram("shaders/main.vertexshader.glsl", "shaders/main.fragmentshader.glsl")) return false;
	if(!Shader.AddUnifrom("MVP", "MVP")) return false;
	LogFile<<"Shaders loaded"<<endl;

	//scene
	if(!Scene.Initialize(&WindowWidth, &WindowHeight)) return false;
	LogFile<<"Scene initialized"<<endl;

	//use static shader and MVP
	glUseProgram(Shader.ProgramId);
	glUniformMatrix4fv(Shader.Uniforms["MVP"], 1, GL_FALSE, Scene.GetDynamicMVP());

	//randomize
    srand((unsigned int)time(NULL));
    LogFile<<"Randomized"<<endl;
    
    //other initializations

    //init buffers
    if(!LeafsBuffer.Initialize()) return false;
    LeafsBuffer.SetPrimitiveType(GL_LINES);
    if(!RoomsBuffer.Initialize()) return false;
    RoomsBuffer.SetPrimitiveType(GL_QUADS);
    //generate level
	if(!GenerateLevel()) return false;
	
	//turn off pause
	Pause = false;
    
    return true;
}

void UpdateStep() {
}

void RenderStep() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//draw functions
	RoomsBuffer.Begin();
		RoomsBuffer.Draw();
	RoomsBuffer.End();
}

void KeysProcessing() {
	if(keys[GLFW_KEY_ESCAPE]) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		return;
	}
	if(keys['R']) {
		keys['R'] = GLFW_RELEASE;//clear key state to avoid multiple generations
		if(!GenerateLevel()) {
			LogFile<<"Error while generate level"<<endl;
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
		return;
	}
}

void ClearApp() {
	//clear functions
	Cave.Close();
	LeafsBuffer.Close();
	RoomsBuffer.Close();
	
	memset(keys, 0, 1024);
	Shader.Close();
	LogFile<<"Application: closed"<<endl;
}

int main(int argc, char** argv) {
	LogFile<<"Application: started"<<endl;
	if(!InitApp()) {
		ClearApp();
		glfwTerminate();
		LogFile.close();
		return 0;
	}

	const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
	const double LimitFPS = 1.0f / mode->refreshRate;
	int Frames = 0;
	int Updates = 0;
	double LastTime = 0, NowTime = 0;
	double RenderTimer = 0;
	
	while(!glfwWindowShouldClose(window)) {
		NowTime = glfwGetTime();
		DeltaTime = NowTime - LastTime;
		RenderTimer += DeltaTime;
		LastTime = NowTime;
		
		if(!Pause) UpdateStep();
		if(RenderTimer >= LimitFPS) {
			RenderStep();
			RenderTimer -= LimitFPS;
		}
		
        glfwSwapBuffers(window);
        glfwPollEvents();
        KeysProcessing();
	}
	ClearApp();
    glfwTerminate();
    LogFile.close();
}
