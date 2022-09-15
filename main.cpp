#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>
#include <vector> // STL dynamic memory.
#include<list>
#include<algorithm>

// Assimp file loader

#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

// Project includes
#include "maths_funcs.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb-master/stb_image.h"


#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <glm/glm/gtc/quaternion.hpp>
#include <glm/glm/gtx/quaternion.hpp>
#include <glm/glm/gtx/euler_angles.hpp>
#include <glm/glm/gtx/norm.hpp>




/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
#define TEAPOT "Models/teapot.dae"
#define BOX "Models/box.dae"
#define BALL "Models/ball.dae"
#define TORUS "Models/torus.dae"
#define RABBIT "Models/rabbit.dae"


vec3 cameraPos = vec3(0.0f, 1.0f, 5.0f);
vec3 cameraTarget = vec3(0.0f, 0.0f, 0.0f);
vec3 cameraDirec = normalise(cameraPos - cameraTarget);
vec3 up = vec3(0.0f, 1.0f, 0.0f);
vec3 camRight = normalise(cross(up, cameraDirec));
vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);
vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f);

float yaw = -90.0f;
float pitch = 0.0f;
float fov = 45.0f;



// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


// mouse state
bool useMouse = false;
bool firstMouse = true;
float lastX = 800.0f / 2.0;
float lastY = 600.0f / 2.0;


float camX = 0.0f;


#pragma region SimpleTypes
typedef struct ModelData
{
    size_t mPointCount = 0;
    std::vector<vec3> mVertices;
    std::vector<vec3> mNormals;
    std::vector<vec2> mTextureCoords;
    std::vector<vec3> mTangents;
    std::vector<vec3> mBitangents;
} ModelData;
#pragma endregion SimpleTypes

using namespace std;

// Shader programs

GLuint shaderTransmittance;
GLuint skyboxShader;



// Fresnel Constants
float fres_bias = 0.04f;
float fres_refR = 1.0f;
float fres_refG = 1.0f;
float fres_refB = 1.0f;
float fres_ref_ind = 1.56f;



// Skybox Textures
unsigned int skyboxTexture, skyboxWater, skyboxCar, skyboxLobby, skyboxMuseum, skyboxSquare;

// Model Data for imported models
ModelData box, teapot, ball, torus, rabbit;
GLuint  skybox_vao, skybox_vbo, teapot_vao, box_vao, ball_vao, torus_vao, rabbit_vao;
float skybox;

vector<std::string> facesSkyBox;

// Skybox face images
// Museum, Lobby and Square images found at http://haxor.thelaborat.org/resources/texture/skybox/cross/
// Water and mountains obtained from the online book LearnOpenGL by Joey de Vries https://learnopengl.com/About
// Car image can be found http://www.fmwconcepts.com/imagemagick/sphericalpano2cube/index.php

// Water and Mountains
vector<std::string> faces1
{
    ("Skyboxes/skybox/right.jpg"),
    ("Skyboxes/skybox/left.jpg"),
    ("Skyboxes/skybox/top.jpg"),
    ("Skyboxes/skybox/bottom.jpg"),
    ("Skyboxes/skybox/front.jpg"),
    ("Skyboxes/skybox/back.jpg")
};

// Lobby
vector<std::string> faces2
{
    ("Skyboxes/skybox2/lobby_right.jpg"),
    ("Skyboxes/skybox2/lobby_left.jpg"),
    ("Skyboxes/skybox2/lobby_top.jpg"),
    ("Skyboxes/skybox2/lobby_bottom.jpg"),
    ("Skyboxes/skybox2/lobby_front.jpg"),
    ("Skyboxes/skybox2/lobby_back.jpg")
};
// Car and garden
vector<std::string> faces3
{
    ("Skyboxes/skyboxCar/right.jpg"),
    ("Skyboxes/skyboxCar/left.jpg"),
    ("Skyboxes/skyboxCar/top.jpg"),
    ("Skyboxes/skyboxCar/bottom.jpg"),
    ("Skyboxes/skyboxCar/front.jpg"),
    ("Skyboxes/skyboxCar/back.jpg")
};


// Museum
vector<std::string> faces4
{
    ("Skyboxes/skyboxMuseum/right.jpg"),
    ("Skyboxes/skyboxMuseum/left.jpg"),
    ("Skyboxes/skyboxMuseum/top.jpg"),
    ("Skyboxes/skyboxMuseum/bottom.jpg"),
    ("Skyboxes/skyboxMuseum/front.jpg"),
    ("Skyboxes/skyboxMuseum/back.jpg")
};


// Square
vector<std::string> faces5
{
    ("Skyboxes/skyboxSquare/right.jpg"),
    ("Skyboxes/skyboxSquare/left.jpg"),
    ("Skyboxes/skyboxSquare/top.jpg"),
    ("Skyboxes/skyboxSquare/bottom.jpg"),
    ("Skyboxes/skyboxSquare/front.jpg"),
    ("Skyboxes/skyboxSquare/back.jpg")
};





GLuint loc1, loc2, loc3, loc4, loc5;

GLfloat rotate_y = 0.0f;


// Function declarations


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

//Load Mesh Function 
ModelData load_mesh(const char* file_name);

// Shader Functions
char* readShaderSource(const char* shaderFile);
static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);
GLuint CompileShaders(const char* vertex, const char* fragment);

// VBO functions
GLuint generateObjectBufferMesh(ModelData mesh_data, GLuint shader);

// Degreees and Radian Conversion
double radians(double degree);


// Texture Function
unsigned int loadCubemap(vector<std::string> faces);

// Display Function
void displayTransmittance();


// Init and Update Function
void init();
void updateScene();

// Multiply Vector by a float
vec3 vecXfloat(float f, vec3 v1);

// Mouse Callback and Key input
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);




const unsigned int screen_width = 1000;
const unsigned int screen_height = 750;




int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(screen_width, screen_height, "Transmittance", NULL, NULL);
    if (window == NULL)
    {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    // set depth function to less than AND equal for skybox depth trick.
    glDepthFunc(GL_LEQUAL);
    // enable seamless cubemap sampling for lower mip levels in the pre-filter map.
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);


    init();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);

    int skyboxChoice = 0;


    unsigned int shaderProgramP = CompileShaders("Shader_Files/Phong_Vertex.txt", "Shader_Files/Phong_Fragment.txt");

    while (!glfwWindowShouldClose(window))
    {



        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        if (skyboxChoice == 0) {
            skyboxTexture = skyboxWater;
        }
        if (skyboxChoice == 1) {
            skyboxTexture = skyboxLobby;
        }
        if (skyboxChoice == 2) {
            skyboxTexture = skyboxCar;
        }
        if (skyboxChoice == 3) {
            skyboxTexture = skyboxMuseum;
        }
        if (skyboxChoice == 4) {
            skyboxTexture = skyboxSquare;
        }

      
        processInput(window);
         
        displayTransmittance();

        updateScene();

        ImGui::Begin("ImGui Window");
        ImGui::Text("Assignment 2: Transmittance");
        ImGui::SliderFloat("Bias", &fres_bias, 0.0f, 1.0f);
        ImGui::SliderFloat("Refractive Index", &fres_ref_ind, 1.0f, 2.5f);
        ImGui::SliderFloat("RefractR", &fres_refR, 0.5f, 1.0f);
        ImGui::SliderFloat("RefractG", &fres_refG, 0.5f, 1.0f);
        ImGui::SliderFloat("RefractB", &fres_refB, 0.5f, 1.0f);    
        ImGui::SliderInt("SkyBox Choice", &skyboxChoice, 0, 4);    
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();


    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}




void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    float cameraSpeed = 10.5f * deltaTime;


    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPos += vecXfloat(cameraSpeed, cameraFront);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPos -= vecXfloat(cameraSpeed, cameraFront);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraPos -= normalise(cross(cameraFront, cameraUp)) * cameraSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPos += normalise(cross(cameraFront, cameraUp)) * cameraSpeed;
    }



    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        useMouse = !useMouse;
    }

}

ModelData load_mesh(const char* file_name) {

    ModelData modelData;

    /* Use assimp to read the model file, forcing it to be read as    */
    /* triangles. The second flag (aiProcess_PreTransformVertices) is */
    /* relevant if there are multiple meshes in the model file that   */
    /* are offset from the origin. This is pre-transform them so      */
    /* they're in the right position.                                 */
    const aiScene* scene = aiImportFile(
        file_name,
        aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace
    );

    if (!scene) {
        fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
        return modelData;
    }

    printf("  %i materials\n", scene->mNumMaterials);
    printf("  %i meshes\n", scene->mNumMeshes);
    printf("  %i textures\n", scene->mNumTextures);

    for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
        const aiMesh* mesh = scene->mMeshes[m_i];
        printf("    %i vertices in mesh\n", mesh->mNumVertices);
        modelData.mPointCount += mesh->mNumVertices;
        for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
            if (mesh->HasPositions()) {
                const aiVector3D* vp = &(mesh->mVertices[v_i]);
                modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
            }
            if (mesh->HasNormals()) {
                const aiVector3D* vn = &(mesh->mNormals[v_i]);
                modelData.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
            }
            if (mesh->HasTextureCoords(0)) {
                const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
                modelData.mTextureCoords.push_back(vec2(vt->x, vt->y));
                //	cout << "Texture coordinates : " << modelData.mTextureCoords[v_i].v[0] << ", " << modelData.mTextureCoords[v_i].v[1] <<  endl;

            }
            if (mesh->HasTangentsAndBitangents()) {
                /* You can extract tangents and bitangents here              */
                /* Note that you might need to make Assimp generate this     */
                /* data for you. Take a look at the flags that aiImportFile  */
                /* can take.                                                 */
            //	cout << "Checking tangents and stuff\n";
                const aiVector3D* vtan = &(mesh->mTangents[v_i]);
                modelData.mTangents.push_back(vec3(vtan->x, vtan->y, vtan->z));

                const aiVector3D* vbt = &(mesh->mBitangents[v_i]);
                modelData.mBitangents.push_back(vec3(vbt->x, vbt->y, vbt->z));
                //	cout << "Tangent coordinates : " << modelData.mTangents[v_i].v[0] << ", " << modelData.mTangents[v_i].v[1] << ", " << modelData.mTangents[v_i].v[2] << endl;
                //	cout << "BiTangent coordinates : " << modelData.mBiTangents[v_i].v[0] << ", " << modelData.mBiTangents[v_i].v[1] << ", " << modelData.mBiTangents[v_i].v[2] << endl;
            }
        }
    }

    aiReleaseImport(scene);
    return modelData;
}




char* readShaderSource(const char* shaderFile) {
    FILE* fp;
    fopen_s(&fp, shaderFile, "rb");

    if (fp == NULL) { return NULL; }

    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);

    fseek(fp, 0L, SEEK_SET);
    char* buf = new char[size + 1];
    fread(buf, 1, size, fp);
    buf[size] = '\0';

    fclose(fp);

    return buf;
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
    // create a shader object
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) {
        std::cerr << "Error creating shader..." << std::endl;
        std::cerr << "Press enter/return to exit..." << std::endl;
        std::cin.get();
        exit(1);
    }
    const char* pShaderSource = readShaderSource(pShaderText);


    // Bind the source code to the shader, this happens before compilation
    glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
    // compile the shader and check for errors


    glCompileShader(ShaderObj);
    GLint success;
    // check for shader related errors using glGetShaderiv

    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024] = { '\0' };
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        std::cerr << "Error compiling "
            << (ShaderType == GL_VERTEX_SHADER ? "vertex" : "fragment")
            << " shader program: " << InfoLog << std::endl;
        std::cerr << "Press enter/return to exit..." << std::endl;
        std::cin.get();
        exit(1);
    }
    // Attach the compiled shader object to the program object
    glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders(const char* vertex, const char* fragment)
{
    //Start the process of setting up our shaders by creating a program ID
    //Note: we will link all the shaders together into this ID
    GLuint shaderProgram = glCreateProgram();
    if (shaderProgram == 0) {
        std::cerr << "Error creating shader program..." << std::endl;
        std::cerr << "Press enter/return to exit..." << std::endl;
        std::cin.get();
        exit(1);
    }


    // Create two shader objects, one for the vertex, and one for the fragment shader
    AddShader(shaderProgram, vertex, GL_VERTEX_SHADER);
    AddShader(shaderProgram, fragment, GL_FRAGMENT_SHADER);


    GLint Success = 0;
    GLchar ErrorLog[1024] = { '\0' };
    // After compiling all shader objects and attaching them to the program, we can finally link it
    glLinkProgram(shaderProgram);
    // check for program related errors using glGetProgramiv
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(shaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
        std::cerr << "Press enter/return to exit..." << std::endl;
        std::cin.get();
        exit(1);
    }

    // program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
    glValidateProgram(shaderProgram);
    // check for program related errors using glGetProgramiv
    glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(shaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
        std::cerr << "Press enter/return to exit..." << std::endl;
        std::cin.get();
        exit(1);
    }
    // Finally, use the linked shader program
    // Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
    glUseProgram(shaderProgram);
    return shaderProgram;
}


GLuint generateObjectBufferMesh(ModelData mesh_data, GLuint shader) {
    /*----------------------------------------------------------------------------
    LOAD MESH HERE AND COPY INTO BUFFERS
    ----------------------------------------------------------------------------*/

    //Note: you may get an error "vector subscript out of range" if you are using this code for a mesh that doesnt have positions and normals
    //Might be an idea to do a check for that before generating and binding the buffer.



    unsigned int vp_vbo = 0;
    loc1 = glGetAttribLocation(shader, "vertex_position");
    loc2 = glGetAttribLocation(shader, "vertex_normal");
    loc3 = glGetAttribLocation(shader, "vertex_texture");
    loc4 = glGetAttribLocation(shader, "vertex_tangent");
    loc5 = glGetAttribLocation(shader, "vertex_bitangent");

    glGenBuffers(1, &vp_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mVertices[0], GL_STATIC_DRAW);
    unsigned int vn_vbo = 0;
    glGenBuffers(1, &vn_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mNormals[0], GL_STATIC_DRAW);

    unsigned int vt_vbo = 0;
    glGenBuffers(1, &vt_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec2), &mesh_data.mTextureCoords[0], GL_STATIC_DRAW);

    unsigned int vtan_vbo = 0;
    glGenBuffers(1, &vtan_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vtan_vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mTangents[0], GL_STATIC_DRAW);

    unsigned int vbt_vbo = 0;
    glGenBuffers(1, &vbt_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbt_vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mBitangents[0], GL_STATIC_DRAW);


    unsigned int vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glEnableVertexAttribArray(loc1);
    glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
    glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(loc2);
    glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
    glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(loc3);
    glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
    glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(loc4);
    glBindBuffer(GL_ARRAY_BUFFER, vtan_vbo);
    glVertexAttribPointer(loc4, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(loc5);
    glBindBuffer(GL_ARRAY_BUFFER, vbt_vbo);
    glVertexAttribPointer(loc5, 3, GL_FLOAT, GL_FALSE, 0, NULL);


    return vao;

}



double radians(double degree) {
    double pi = 3.14159265359;
    return (degree * (pi / 180));
}


unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);



    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}


void displayTransmittance() {




    // tell GL to only draw onto a pixel if the shape is closer to the viewer
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderTransmittance);



    //Declare your uniform variables that will be used in your shader
    // Vertex Shader Uniforms
    int matrix_location = glGetUniformLocation(shaderTransmittance, "model");
    int view_mat_location = glGetUniformLocation(shaderTransmittance, "view");
    int proj_mat_location = glGetUniformLocation(shaderTransmittance, "proj");
    int fres_bias_loc = glGetUniformLocation(shaderTransmittance, "fresnelBias");
    int fres_refR_loc = glGetUniformLocation(shaderTransmittance, "RefractR");
    int fres_refG_loc = glGetUniformLocation(shaderTransmittance, "RefractG");
    int fres_refB_loc = glGetUniformLocation(shaderTransmittance, "RefractB");
    int fres_ratio_loc = glGetUniformLocation(shaderTransmittance, "Ref_Index");
    int camPos_Fres = glGetUniformLocation(shaderTransmittance, "cameraPos");


    // Root of the Hierarchy
    mat4 view = identity_mat4();
    mat4 persp_proj = perspective(fov, (float)screen_width / (float)screen_height, 0.1f, 1000.0f);
    mat4 teapot_model = identity_mat4();


    mat4 box_model = identity_mat4();
    mat4 ball_model = identity_mat4();
    mat4 torus_model = identity_mat4();
    mat4 rabbit_model = identity_mat4();

    view = look_at(cameraPos, cameraPos + cameraFront, cameraUp);
    teapot_model = rotate_y_deg(teapot_model, rotate_y);

    // update uniforms & draw
    glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
    glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);

    glUniform1f(fres_bias_loc, fres_bias);
    glUniform1f(fres_refR_loc, fres_refR);
    glUniform1f(fres_refG_loc, fres_refG);
    glUniform1f(fres_refB_loc, fres_refB);
    glUniform1f(fres_ratio_loc, fres_ref_ind);
    glUniform3fv(camPos_Fres, 1, cameraPos.v);


    // Drawe teapot model
    glUniformMatrix4fv(matrix_location, 1, GL_FALSE, teapot_model.m);

    glBindVertexArray(teapot_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    glDrawArrays(GL_TRIANGLES, 0, teapot.mPointCount);
    glActiveTexture(GL_TEXTURE0);


    // Draw Box Model
    box_model = rotate_y_deg(box_model, rotate_y * 0.6);
    box_model = translate(box_model, vec3(-7.5f, 0.0f, 0.0f));

    glUniformMatrix4fv(matrix_location, 1, GL_FALSE, box_model.m);
    glBindVertexArray(box_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    glDrawArrays(GL_TRIANGLES, 0, box.mPointCount);
    glActiveTexture(GL_TEXTURE0);


    // Draw Ball Model
    ball_model = rotate_y_deg(ball_model, rotate_y);
    ball_model = translate(ball_model, vec3(7.5f, 0.0f, 0.0f));

    glUniformMatrix4fv(matrix_location, 1, GL_FALSE, ball_model.m);
    glBindVertexArray(ball_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    glDrawArrays(GL_TRIANGLES, 0, ball.mPointCount);
    glActiveTexture(GL_TEXTURE0);

    // Draw Torus Model
    torus_model = rotate_y_deg(torus_model, rotate_y);
    torus_model = translate(torus_model, vec3(-15.0f, 0.0f, 0.0f));

    glUniformMatrix4fv(matrix_location, 1, GL_FALSE, torus_model.m);
    glBindVertexArray(torus_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    glDrawArrays(GL_TRIANGLES, 0, torus.mPointCount);
    glActiveTexture(GL_TEXTURE0);

    // Draw Rabbit Model
    rabbit_model = rotate_y_deg(rabbit_model, rotate_y);
    rabbit_model = translate(rabbit_model, vec3(15.0f, 0.0f, 0.0f));

    glUniformMatrix4fv(matrix_location, 1, GL_FALSE, rabbit_model.m);
    glBindVertexArray(rabbit_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    glDrawArrays(GL_TRIANGLES, 0, rabbit.mPointCount);
    glActiveTexture(GL_TEXTURE0);


    // Draw the skybox

    glDepthFunc(GL_LEQUAL);
    glUseProgram(skyboxShader);

    glm::vec3 camPosGLM = glm::vec3(cameraPos.v[0], cameraPos.v[1], cameraPos.v[2]);
    glm::vec3 camFrontGLM = glm::vec3(cameraFront.v[0], cameraFront.v[1], cameraFront.v[2]);
    glm::vec3 camUpGLM = glm::vec3(cameraUp.v[0], cameraUp.v[1], cameraUp.v[2]);

    glm::mat4 viewSky = glm::mat4(1.0f);
    mat4 modelSky = identity_mat4();
    viewSky = glm::mat4(glm::mat3(glm::lookAt(camPosGLM, camPosGLM + camFrontGLM, camUpGLM)));

    int matrix_location_sky = glGetUniformLocation(skyboxShader, "model");
    int view_mat_location_sky = glGetUniformLocation(skyboxShader, "view");
    int proj_mat_location_sky = glGetUniformLocation(skyboxShader, "projection");
    int sampler_cube = glGetUniformLocation(skyboxShader, "skybox");

    glUniformMatrix4fv(proj_mat_location_sky, 1, GL_FALSE, persp_proj.m);
    glUniformMatrix4fv(view_mat_location_sky, 1, GL_FALSE, &viewSky[0][0]);
    glUniformMatrix4fv(matrix_location_sky, 1, GL_FALSE, modelSky.m);
    glUniform1f(sampler_cube, 0);

    glBindVertexArray(skybox_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);

}






void updateScene() {

    static double last_time = 0;
    double curr_time = glfwGetTime();
    if (last_time == 0)
        last_time = curr_time;
    float delta = (curr_time - last_time);
    last_time = curr_time;

    float currentFrame = glfwGetTime();


    deltaTime = curr_time - lastFrame;
    lastFrame = currentFrame;

    // Rotate the model slowly around the y axis at 20 degrees per second
    rotate_y += 20.5f * deltaTime;
    rotate_y = fmodf(rotate_y, 360.0f);

 //   cout << "delta" << delta << endl;

}


void init()
{
    // Set up the shaders
    shaderTransmittance = CompileShaders("Shader_Files/Transmittance_Vertex.txt", "Shader_Files/Transmittance_Fragment.txt");


    teapot = load_mesh(TEAPOT);
    teapot_vao = generateObjectBufferMesh(teapot, shaderTransmittance);

    box = load_mesh(BOX);
    box_vao = generateObjectBufferMesh(box, shaderTransmittance);

    ball = load_mesh(BALL);
    ball_vao = generateObjectBufferMesh(ball, shaderTransmittance);

    torus = load_mesh(TORUS);
    torus_vao = generateObjectBufferMesh(torus, shaderTransmittance);

    rabbit = load_mesh(RABBIT);
    rabbit_vao = generateObjectBufferMesh(rabbit, shaderTransmittance);



        // Skybox 
    skyboxShader = CompileShaders("Shader_Files/Sky_Vertex.txt", "Shader_Files/Sky_Fragment.txt");

    float skybox[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };


    unsigned int location = glGetAttribLocation(skyboxShader, "vertex_position");

    glGenVertexArrays(1, &skybox_vao);
    glGenBuffers(1, &skybox_vbo);
    glBindVertexArray(skybox_vao);
    glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skybox), &skybox, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    
    skyboxWater = loadCubemap(faces1);
    skyboxLobby = loadCubemap(faces2);
    skyboxCar = loadCubemap(faces3);
    skyboxMuseum = loadCubemap(faces4);
    skyboxSquare = loadCubemap(faces5);
    
//    skyboxTexture = skyboxWater;


}


vec3 vecXfloat(float f, vec3 v1) {

    vec3 result = vec3((v1.v[0] * f), (v1.v[1] * f), (v1.v[2] * f));
    return result;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {


    float x = static_cast<float>(xposIn);
    float y = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = x;
        lastY = y;
        firstMouse = false;
    }

    float xoffset = x - lastX;
    float yoffset = lastY - y;
    lastX = x;
    lastY = y;

    float sensitivity = 0.5f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    vec3 front;
    front.v[0] = cos(radians(yaw)) * cos(radians(pitch));
    front.v[1] = sin(radians(pitch));
    front.v[2] = sin(radians(yaw)) * cos(radians(pitch));

    if (useMouse) {
        cameraFront = normalise(front);
    }

}