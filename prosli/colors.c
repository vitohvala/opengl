#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
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
#define RADIANS(x) (x) * (PI/180.f)


/*GLOBALS*/
vec3 cameraPos   = {0.0f, 0.0f,  3.0f};
vec3 cameraFront = {0.0f, 0.0f, 0.0f};
vec3 cameraUp    = {0.0f, 1.0f,  0.0f};
float delta_time = 0;
float lastX = 400, lastY = 300;
bool firstMouse = true;
float yaw = -90.0f, pitch = 0.0f, fov = 45.0f;


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
    const float cameraSpeed = 2.5f * delta_time; // adjust accordingly
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
            vec3 temp;
            vec3_scale(temp, cameraFront, cameraSpeed);
            vec3_add(cameraPos, cameraPos, temp);
            //cameraPos += cameraSpeed * cameraFront;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
            vec3 temp;
            vec3_scale(temp, cameraFront, cameraSpeed);
            vec3_sub(cameraPos, cameraPos, temp);
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            vec3 temp;
            vec3 cross;
            vec3_mul_cross(cross, cameraFront, cameraUp);
            vec3_norm(temp, cross);
            vec3 temp2;
            vec3_scale(temp2, temp, cameraSpeed);
            vec3_sub(cameraPos, cameraPos, temp2);
            //cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
            vec3 temp;
            vec3 cross;
            vec3_mul_cross(cross, cameraFront, cameraUp);
            vec3_norm(temp, cross);
            vec3 temp2;
            vec3_scale(temp2, temp, cameraSpeed);
            vec3_add(cameraPos, cameraPos, temp2);
        }
        cameraPos[1] = 0.0f;

}

unsigned int 
get_shader_program(const char *vertex_filename, const char *fragment_filename) 
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


void
mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov += (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f; 
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
    glfwSetCursorPosCallback(window, mouse_callback); 
    glfwSetScrollCallback(window, scroll_callback);

    float vertices[] = {
        // positions         // colors          //textures
        -0.5f, -0.5f, -0.5f,  
         0.5f, -0.5f, -0.5f,  
         0.5f,  0.5f, -0.5f,  
         0.5f,  0.5f, -0.5f,  
        -0.5f,  0.5f, -0.5f,  
        -0.5f, -0.5f, -0.5f,  

        -0.5f, -0.5f,  0.5f,  
         0.5f, -0.5f,  0.5f,  
         0.5f,  0.5f,  0.5f,  
         0.5f,  0.5f,  0.5f,  
        -0.5f,  0.5f,  0.5f,  
        -0.5f, -0.5f,  0.5f,  

        -0.5f,  0.5f,  0.5f,  
        -0.5f,  0.5f, -0.5f,  
        -0.5f, -0.5f, -0.5f,  
        -0.5f, -0.5f, -0.5f,  
        -0.5f, -0.5f,  0.5f,  
        -0.5f,  0.5f,  0.5f,  

         0.5f,  0.5f,  0.5f,  
         0.5f,  0.5f, -0.5f,  
         0.5f, -0.5f, -0.5f,  
         0.5f, -0.5f, -0.5f,  
         0.5f, -0.5f,  0.5f,  
         0.5f,  0.5f,  0.5f,  

        -0.5f, -0.5f, -0.5f,  
         0.5f, -0.5f, -0.5f,  
         0.5f, -0.5f,  0.5f,  
         0.5f, -0.5f,  0.5f,  
        -0.5f, -0.5f,  0.5f,  
        -0.5f, -0.5f, -0.5f,  

        -0.5f,  0.5f, -0.5f,  
         0.5f,  0.5f, -0.5f,  
         0.5f,  0.5f,  0.5f,  
         0.5f,  0.5f,  0.5f,  
        -0.5f,  0.5f,  0.5f,  
        -0.5f,  0.5f, -0.5f  
    };

  /*  int indices[] = {
        0, 1, 3, 
        1, 2, 3
    };*/

    unsigned int shader2 = get_shader_program("shaders/shader.vs", "shaders/shader2.fs");
    unsigned int shaderProgram = get_shader_program("shaders/shader.vs", "shaders/shader.fs");
    
    unsigned int vao1, VBO;
    
    glGenVertexArrays(1, &vao1);
    glGenBuffers(1, &VBO);

    glBindVertexArray(vao1);


    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int  VAO;
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    int nrAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
    printf("Maximum nr of vertex attributes supported: %d\n", nrAttributes);

    
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //print_mat4x4(trans);
   
    float start_frame = glfwGetTime(), end_frame;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    while(!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.17f, 0.2f, 0.23f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        vec3 direction;
        direction[0] = cosf(RADIANS(yaw)) * cosf(RADIANS(pitch)); 
        direction[1] = sinf(RADIANS(pitch));
        direction[2] = sinf(RADIANS(yaw)) * cosf(RADIANS(pitch));

        vec3_norm(cameraFront, direction);

        //mat4x4_identity(trans);
        //mat4x4 ret;
        //mat4x4_translate(trans, 0.5f, -0.5f, 0.0f);
        //mat4x4_rotate(ret, trans, 0.0f, 0.0f, 1.0f, (float)glfwGetTime());
        //mat4x4_scale_aniso(trans, ret, 0.5f, -0.5f, 0.0f);
        mat4x4 view;
        mat4x4_identity(view); 
        mat4x4 model;
        mat4x4_identity(model); 
        vec3 add;
        vec3_add(add, cameraPos, cameraFront);
        mat4x4_look_at(view, cameraPos, add, cameraUp);
        mat4x4 projection;
        mat4x4_identity(projection); 
        mat4x4_perspective(projection, RADIANS(fov), 800.0f/600.0f, 0.01f, 100.0f);


        glUseProgram(shaderProgram);
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 1.0f, 0.5f, 0.31f);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, (GLfloat*)projection);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, (GLfloat*)view);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, (GLfloat*)model);
        glBindVertexArray(VAO); 
       // glDrawArrays(GL_TRIANGLES, 0, 3);
       //
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glDrawArrays(GL_TRIANGLES, 0, 36);



        glUseProgram(shader2);

        glUniformMatrix4fv(glGetUniformLocation(shader2, "projection"), 1, GL_FALSE, (GLfloat*)projection);
        glUniformMatrix4fv(glGetUniformLocation(shader2, "view"), 1, GL_FALSE, (GLfloat*)view);
        
        mat4x4_identity(model);
        mat4x4_translate(model, 1.2f, 1.0f, 2.0f);
        mat4x4 amodel;
        mat4x4_scale_aniso(amodel, model, 0.2f, 0.2f, 0.2f);

        glUniformMatrix4fv(glGetUniformLocation(shader2, "model"), 1, GL_FALSE, (GLfloat*)amodel);

        glBindVertexArray(vao1);
        glDrawArrays(GL_TRIANGLES, 0, 36);
      
        glfwSwapBuffers(window);
        glfwPollEvents();    

        end_frame = glfwGetTime();
        delta_time = end_frame - start_frame;
        start_frame = end_frame;
    }

    glfwTerminate();
    return 0;
}
