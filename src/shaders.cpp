#define GL_GLEXT_PROTOTYPES
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <fstream>
#include <cstdlib>
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
GLuint loadShaders(const char* vertexShaderFile, const char* fragmentShaderFile) {
		
	ifstream vertexShaderStream(vertexShaderFile, ios::in);
	
	const int sizes = 8000;
	int size;
	
	char *vertexShaderSource; // = new char[sizes];
	
	if(vertexShaderStream.is_open()) {
		vertexShaderStream.seekg(0, ios::end);
		size = vertexShaderStream.tellg();
		vertexShaderStream.seekg(0);
		
		vertexShaderSource = new char[size];
		vertexShaderStream.read( vertexShaderSource, size );
		vertexShaderSource[size] = 0;
	} else {
		cout << "failed to load vertex shader" << endl;
		exit(1);
	}
	
	vertexShaderStream.close();
	
	ifstream fragmentShaderStream(fragmentShaderFile, ios::in);
	
	char *fragmentShaderSource; // = new char[sizes];
	if(fragmentShaderStream.is_open()) {
		fragmentShaderStream.seekg(0, ios::end);
		size = fragmentShaderStream.tellg();
		fragmentShaderStream.seekg(0);
		
		fragmentShaderSource = new char[size];
		fragmentShaderStream.read( fragmentShaderSource, size );
		fragmentShaderSource[size] = 0;
	} else {
		cout << "failed to load fragment shader" << endl;
		exit(1);
	}
	
	fragmentShaderStream.close();
	
	// ----
	
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, (const GLchar**)&vertexShaderSource, 0);
	
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentShaderSource, 0);

	// compiling
	GLint status;
	
	glCompileShader(vertexShader);
	
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
	
	//cout << "compilation: " << (status == GL_TRUE) << endl;
	//~ 
	if(status != GL_TRUE) {
		char buffer[512];
		glGetShaderInfoLog(vertexShader, 512, NULL, buffer);
		cout << buffer << endl;
		exit(1);
	}
	
	glCompileShader(fragmentShader);
	
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
	
	//cout << "compilation: " << (status == GL_TRUE) << endl;
	
	if(status != GL_TRUE) {
		char buffer[512];
		glGetShaderInfoLog(fragmentShader, 512, NULL, buffer);
		cout << buffer << endl;
		exit(1);
	}
	
	delete [] vertexShaderSource;
	delete [] fragmentShaderSource;
	
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	
	glLinkProgram(shaderProgram); 
	glUseProgram(shaderProgram);
	if(!shaderProgram)
		exit(1);
		
	return shaderProgram;
}
