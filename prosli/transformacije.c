#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <linmath.h>

#include "untitled_types.h"

#define ERROR_EXIT(E, ...) fprintf(stderr, __VA_ARGS__); exit(E)
#define ERROR_RETURN(R, ...) fprintf(stderr, __VA_ARGS__); return R
#define print_mat4x4(mat) \
    do { \
        for(size_t i = 0; i < 4; ++i) { \
            for(size_t j = 0; j < 4; j++) { \
                fprintf(stdout, "%.2f ", mat[i][j]); \
            } \
            puts("");\
        } \
    } while(0)

#define PI 3.141592f

typedef  struct {
    const char *vertex_shader_source;
    const char *fragment_shader_source;
} Shader;

typedef struct {
    char *data;
    size_t length;
    size_t capacity;
} String;

String 
string_init() 
{
    String t;
    t.capacity = 256;
    t.length = 0;
    t.data = (char *)malloc(t.capacity * sizeof(char));
    
    if(!t.data) {
        ERROR_EXIT(1, "Couldn't malloc\n");
    }

    return t;
}

void
string_free(String *s) 
{
    free(s->data);
    s->capacity = 0;
    s->length = 0;
}

char 
*get_file_data(const char *filename, const char *r)
{
    FILE *file = fopen(filename, r);
    if(!file) {
        ERROR_EXIT(1, "Couldn't open file %s\n", filename);
    }
    String tmp = string_init();
    char c;
    while((c = fgetc(file)) != EOF) {
        tmp.data[tmp.length++] = c;

        if(tmp.length > tmp.capacity - 1) {
            tmp.capacity *= 2;
            tmp.data = realloc(tmp.data, tmp.capacity * sizeof(char));
        }
    }
    fclose(file);
    tmp.data[tmp.length - 1] = '\0';
    return tmp.data;
}

Shader 
load_shader_source(const char *f_vertex_shader, const char * f_fragment_shader) 
{

    
    Shader ret;

    ret.vertex_shader_source = get_file_data(f_vertex_shader, "r");
    ret.fragment_shader_source = get_file_data(f_fragment_shader, "r");

    return ret;
}

unsigned int
compile_shader(const char *shader_src, GLenum shader_type) 
{
    unsigned int vf_shader = glCreateShader(shader_type);

    glShaderSource(vf_shader, 1, &shader_src, NULL);
    glCompileShader(vf_shader);

    int  success;
    char infoLog[512];
    glGetShaderiv(vf_shader, GL_COMPILE_STATUS, &success);

    if(!success) {
        glGetShaderInfoLog(vf_shader, 512, NULL, infoLog);
        ERROR_EXIT(1, "ERROR SHADER VERTEX COMPILATION_FAILED %s\n", infoLog);
    }
    return vf_shader;
}

void 
framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void 
processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

unsigned int get_shader_program(const char *vertex_filename, const char *fragment_filename) 
{
    Shader shader = load_shader_source(vertex_filename, fragment_filename);
    unsigned int vertexShader = compile_shader(shader.vertex_shader_source, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compile_shader(shader.fragment_shader_source, GL_FRAGMENT_SHADER);
    unsigned int shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int  success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        ERROR_EXIT(1,"ERROR SHADER Program COMPILATION_FAILED %s\n", infoLog);
   }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    free((char*)shader.vertex_shader_source);
    free((char*)shader.fragment_shader_source);

    fprintf(stdout, "Shader program loaded\n");
    return shaderProgram;
}

int
main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // ovo je za mac-os
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "OpenGL", 0, 0);
    if(!window) {
        fprintf(stderr, "Couldn't create window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return -1;
    }

    glViewport(0, 0, 800, 600);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); 

    float vertices[] = {
        // positions         // colors          //textures
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left  
    };

    int indices[] = {
        0, 1, 3, 
        1, 2, 3
    };

    unsigned int shaderProgram = get_shader_program("shader.vs", "shader.fs");
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    int nrAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
    printf("Maximum nr of vertex attributes supported: %d\n", nrAttributes);

    unsigned int texture1;
  
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1); 
     // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    unsigned char *data = stbi_load("img/container.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        fprintf(stdout, "Failed to load texture\n");
    }
    stbi_image_free(data);
     
    unsigned int texture2;
  
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2); 
     // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    data = stbi_load("./img/awesomeface.png", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        fprintf(stdout, "Failed to load texture\n");
    }
    stbi_image_free(data);
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture2"), 1);


    //print_mat4x4(trans);

    while(!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        mat4x4 trans;
        mat4x4_identity(trans);
        mat4x4 ret;
        mat4x4_translate(trans, 0.5f, -0.5f, 0.0f);
        mat4x4_rotate(ret, trans, 0.0f, 0.0f, 1.0f, (float)glfwGetTime());
        //mat4x4_scale_aniso(trans, ret, 0.5f, -0.5f, 0.0f);

        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1); 
        glBindTexture(GL_TEXTURE_2D, texture2);
        glUseProgram(shaderProgram);

        unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, (GLfloat*)ret);

        //float timeValue = glfwGetTime();
        //float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
        //float redvalue = (sin(timeValue) / 2.0f) + 0.5f;
        //int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
        //glUniform4f(vertexColorLocation, redvalue, greenValue, 0.0f, 1.0f);
        
        glBindVertexArray(VAO); 
       // glDrawArrays(GL_TRIANGLES, 0, 3);
       //
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        mat4x4 trans1;
        mat4x4_identity(trans1);
        mat4x4 ret1;
        mat4x4_translate(trans1, -0.5f, 0.5f, 0.0f);
        float am = sinf((float)glfwGetTime());
        mat4x4_scale_aniso(ret1, trans1, am, am, 0.0f);
        //mat4x4_rotate(ret, trans, 0.0f, 0.0f, 1.0f, (float)glfwGetTime());

        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, (GLfloat*)ret1);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();    
    }

    glfwTerminate();
    return 0;
}
