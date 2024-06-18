/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** StaticMesh
*/

#include "Models/StaticMesh.hpp"

#include "Renderer/ShaderProgram.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"
#include "assimp/postprocess.h"

StaticMesh::StaticMesh(const std::string& modelPath) {
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(modelPath,
        aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_GenNormals |
        aiProcess_ValidateDataStructure | aiProcess_GlobalScale
    );

    if ((scene == nullptr) || ((scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0) || (scene->mRootNode == nullptr))
        throw std::runtime_error("Failed to load model: " + std::string(importer.GetErrorString()));

    loadMaterials(scene, modelPath);
    processNode(scene->mRootNode, scene);
}

StaticMesh::~StaticMesh() {
}

void StaticMesh::draw(const std::shared_ptr<ShaderProgram>& shaderProgram, const glm::mat4& transform) const noexcept {
    glEnable(GL_DEPTH_TEST);

    for (const auto& submesh : m_submeshes) {
        if (m_materials[submesh.materialIndex].albedoMap != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_materials[submesh.materialIndex].albedoMap);
            shaderProgram->setInt("albedoMap", 0);
            shaderProgram->setBool("useAlbedoMap", true);
        } else {
            shaderProgram->setBool("useAlbedoMap", false);
        }

        if (m_materials[submesh.materialIndex].metallicRoughnessMap != 0) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_materials[submesh.materialIndex].metallicRoughnessMap);
            shaderProgram->setInt("metallicRoughnessMap", 1);
            shaderProgram->setBool("useMetallicRoughnessMap", true);
        } else {
            shaderProgram->setBool("useMetallicRoughnessMap", false);
        }

        if (m_materials[submesh.materialIndex].normalMap != 0) {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, m_materials[submesh.materialIndex].normalMap);
            shaderProgram->setInt("normalMap", 2);
            shaderProgram->setBool("useNormalMap", true);
        } else {
            shaderProgram->setBool("useNormalMap", false);
        }

        shaderProgram->setMat4("model", submesh.transform * transform);
        glBindVertexArray(submesh.vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(submesh.indexCount), GL_UNSIGNED_INT, nullptr);
    }

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
}

void StaticMesh::loadTexture(const std::string& texturePath, const std::string& modelPath, uint32_t& texture) {
    std::string filePath = modelPath.substr(0, modelPath.find_last_of('/')) + "/" + texturePath;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    int width = 0;
    int height = 0;
    int channels = 0;
    stbi_set_flip_vertically_on_load(static_cast<int>(true));
    unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
    if (data == nullptr)
        throw std::runtime_error("Failed to load texture: " + filePath);

    GLenum format = GL_RGB;
    if (channels == 4)
        format = GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLsizei>(format), width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    {   // Anisotropic filtering
        GLfloat value = 0;
        constexpr GLfloat max_anisotropy = 8.0;

        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &value);
        value = (value > max_anisotropy) ? max_anisotropy : value;
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, value);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, value);
    }

    stbi_image_free(data);
}

void StaticMesh::loadMaterials(const aiScene *scene, const std::string& modelPath) {
    for (std::size_t i = 0; i < scene->mNumMaterials; ++i) {
        const aiMaterial *assimpMat = scene->mMaterials[i]; // NOLINT
        Material material = {};

        aiString texturePath;
        if (assimpMat->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
            loadTexture(texturePath.C_Str(), modelPath, material.albedoMap);

        if (assimpMat->GetTexture(aiTextureType_METALNESS, 0, &texturePath) == AI_SUCCESS)
            loadTexture(texturePath.C_Str(), modelPath, material.metallicRoughnessMap);

        if (assimpMat->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS)
            loadTexture(texturePath.C_Str(), modelPath, material.normalMap);

        m_materials.push_back(material);
    }
}

void StaticMesh::processNode(const aiNode *node, const aiScene *scene) {
    for (std::size_t i = 0; i < node->mNumMeshes; ++i) {
        const aiMatrix4x4 transform = node->mTransformation;
        const aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];  // NOLINT
        processMesh(mesh, glm::make_mat4(&transform.a1));
    }

    for (std::size_t i = 0; i < node->mNumChildren; ++i)
        processNode(node->mChildren[i], scene); // NOLINT
}

void StaticMesh::processMesh(const aiMesh *mesh, const glm::mat4& transform) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex = {
            .position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z },   // NOLINT
            .normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z },        // NOLINT
            .texCoords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y }         // NOLINT
        };

        vertices.push_back(vertex);
    }

    for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
        const aiFace& face = mesh->mFaces[i];       // NOLINT
        for (uint32_t j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);    // NOLINT
    }

    Submesh submesh = {
        .vao = 0,
        .vbo = 0,
        .ibo = 0,
        .indexCount = static_cast<uint32_t>(indices.size()),
        .materialIndex = mesh->mMaterialIndex,
        .transform = transform
    };

    glGenVertexArrays(1, &submesh.vao);
    glBindVertexArray(submesh.vao);

    glGenBuffers(1, &submesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, submesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizei>(vertices.size() * sizeof(Vertex)), vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &submesh.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, submesh.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizei>(indices.size() * sizeof(uint32_t)), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void* )(offsetof(Vertex, position)));  // NOLINT

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void* )(offsetof(Vertex, normal)));    // NOLINT

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void* )(offsetof(Vertex, texCoords))); // NOLINT

    glBindVertexArray(0);
    m_submeshes.push_back(submesh);
}