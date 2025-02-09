#pragma once

#ifndef OBJECTRENDER_H
#define OBJECTRENDER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>      
#include <assimp/scene.h>         
#include <assimp/postprocess.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <map>

#include "Camera.h"

#include <iostream>
#include <vector>
#include <string>

// Shaders
const char* vertex_shader_source = R"(
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;


void main() {
    gl_Position = projection * view * model * vec4(position, 1.0f);
    TexCoord = texCoord;
}
)";

const char* fragment_shader_source = R"(
#version 330 core
in vec2 TexCoord;
out vec4 fragColor;

uniform sampler2D ourTextures[32];
uniform int TextureIndex[32];

//vec4 testColor = vec4(float(TextureIndex) / 10.0, 1.0, 0.0, 1.0);

void main() {
    fragColor = texture(ourTextures[TextureIndex], TexCoord);

    //fragColor = testColor;
    
    //fragColor = mix(texture(ourTextures[0], TexCoord), texture(ourTextures[1], TexCoord), 0.5);


    /*
    fragColor = texture(ourTextures[TextureIndex + 1], TexCoord);
    fragColor = texture(ourTextures[TextureIndex + 2], TexCoord);
    fragColor = texture(ourTextures[TextureIndex + 3], TexCoord);
    */
}
)";

// Object render (Vertex edition)
struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoords;
};


std::vector<Vertex> vertices;
std::vector<unsigned int> indices;
std::vector<unsigned int> textureIds;





struct Texture {
    unsigned int id;
    std::string type;
};





// global initialize for objects` meshes
struct Mesh {
    unsigned int VAO, VBO, EBO;
    //unsigned int vao, vbo, ebo;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<unsigned int> textureIds; // <Texture>
    //unsigned int textureIds;
    //unsigned int textureId;
};
std::vector<Mesh> meshes;

unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Texture wrapping and filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);

    } else {
        std::cerr << "Failed to load texture(s)!" << std::endl;

        stbi_image_free(data);
    }

    //stbi_image_free(data);

    return textureID;
}

void loadModel(const std::string& path) {
    meshes.clear();
    vertices.clear();
    indices.clear();
    textureIds.clear();

    std::cout << std::endl;
    std::cout << "{Vertices & Indices}" << std::endl;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace | aiProcess_SortByPType | aiProcess_GenSmoothNormals);
    //const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);


    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::Assimp:: " << importer.GetErrorString() << std::endl;

        return;
    }

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        const aiMesh* mesh = scene->mMeshes[i];

        Mesh currentMesh;

        unsigned int vertexCount = vertices.size();
        unsigned int textCount = textureIds.size();
        //unsigned int indiceCount = indices.size();

        // vertices & textures render
        for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec3 indice;
            
            glm::vec2 texCoord(0.0f, 0.0f); // Position of textures

            position.x = mesh->mVertices[j].x;
            position.y = mesh->mVertices[j].y;
            position.z = mesh->mVertices[j].z;


            if (mesh->HasNormals()) {
                normal.x = mesh->mNormals[j].x;
                normal.y = mesh->mNormals[j].y;
                normal.z = mesh->mNormals[j].z;
            }

            if (mesh->mTextureCoords[0]) {
                texCoord.x = mesh->mTextureCoords[0][j].x;
                texCoord.y = mesh->mTextureCoords[0][j].y;
            }

            vertices.push_back({ position, texCoord });
            //vertices.push_back({ normal, texCoord });
        }

        // indices rendering
        for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
            const aiFace face = mesh->mFaces[j];
            if (face.mNumIndices < 3) continue;

            //assert(face.mNumIndices == 3);

            for (unsigned int k = 0; k < face.mNumIndices; k++) {
                indices.push_back(face.mIndices[k] + vertexCount);
                //indices.push_back(face.mIndices[k]); // currentMesh.indices.push_back(face.mIndices[k]);
            }
        }

        std::cout << std::endl;

        if (mesh->mMaterialIndex >= 0) {
            const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            unsigned int textureCount = material->GetTextureCount(aiTextureType_DIFFUSE);
            //unsigned int textureCount2 = material->GetTextureCount(aiTextureType_SPECULAR);

            currentMesh.textureIds.reserve(textureCount);

            for (unsigned int j = 0; j < material->GetTextureCount(aiTextureType_DIFFUSE); j++) { // j = 0 || j > 0
                aiString str;
                material->GetTexture(aiTextureType_DIFFUSE, j, &str);
                //material->GetTexture(aiTextureType_DISPLACEMENT, j, &str);



                std::string directory = path.substr(0, path.find_last_of('/'));
                std::string fullPath = directory + "/" + str.C_Str();
                

                unsigned int texturePath = loadTexture(fullPath.c_str());


                // u can load any texture here
                //textureIds.push_back(loadTexture(fullPath.c_str())); // || textCount (and textCount)
                currentMesh.textureIds.push_back(texturePath); // || textCount (and textureCount)
                //currentMesh.textureIds.push_back(loadTexture("HeatherMason/vest.png")); // ...
                //currentMesh.textureIds.push_back(loadTexture("HeatherMason/hair.png", "HeatherMason/head.png", "HeatherMason/skirt.png", "HeatherMason/vest.png"));

            }
        }

        std::cout << "Vertices: " << vertices.size() << std::endl;
        std::cout << "Indices: " << indices.size() << std::endl;
        //std::cout << "Textures: " << textureIds.size() << std::endl;

        meshes.push_back(currentMesh);
    }
}

// Setup shaders
void setupShader() {
    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);

    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}


// VAO & VBO
void setupModel() {
    for (Mesh& mesh : meshes) {
        //unsigned int vbo, ebo, vao;
        //unsigned int vbo, ebo;

        glGenBuffers(1, &mesh.VBO); // glGenBuffers(1, &vbo); (...) // mesh.VBO
        glGenVertexArrays(1, &mesh.VAO);
        glGenBuffers(1, &mesh.EBO);

        glBindVertexArray(mesh.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW); // mesh.vertices &mesh.vertices

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW); // mesh.indices &mesh.indices


        //glEnableVertexAttribArray(0); // not necessary (1)

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0); // 0, 3
        glEnableVertexAttribArray(0); // 1

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords)); // 1, 2
        glEnableVertexAttribArray(1); // 1

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords)); // not necessary (2)

        glBindVertexArray(0);

        //glGenerateMipmap(GL_TEXTURE_2D);

        //stbi_set_flip_vertically_on_load(true);

    }
}


/*
void renderMeshes() {
    for (size_t i = 0; i < meshes.textureIds.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, meshes.textureIds[i]);
        glUniform1i(glGetUniformLocation(shader_program, "TetxureIndex"), i);
        //glUniform1i(glGetUniformLocation(shader_program, ("ourTextures[" + std::to_string(i) + "]").c_str()), i);
    }
    //glBindVertexArray(0);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, meshes.indices.size(), GL_UNSIGNED_INT, 0);
}
*/



/*
// VAO, VBO, EBO setup
void setupMesh() {
    for (Mesh& mesh : meshes) {
        glGenVertexArrays(1, &mesh.VAO);
        glGenBuffers(1, &mesh.VBO);
        glGenBuffers(1, &mesh.EBO);

        glBindVertexArray(mesh.VAO);

        glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
        glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), &mesh.vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), &mesh.indices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }
}
*/




#endif // ObjectRender.h