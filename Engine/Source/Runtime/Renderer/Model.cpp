#include "xlpch.h"

#include "Runtime/Renderer/Model.h"

namespace XLEngine
{
    void Model::Draw(const glm::mat4& transform, Ref<Shader>& shader, int entityID)
    {
        for (unsigned int i = 0; i < mMeshes.size(); ++i)
            mMeshes[i].Draw(transform, shader, entityID);
    }

    void Model::LoadModel(const std::string& path)
    {
        // assimp 在解析个别模型（尤其材质属性阶段）时会抛出 C++ 异常；若放任逃逸，
        // 会击穿场景反序列化并终止引擎。这里就地捕获并退化为空模型 + 错误日志。
        try
        {
            Assimp::Importer importer;
            const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
            {
                XL_CORE_ERROR("Failed to load model {0}: {1}", path, importer.GetErrorString());
                mMeshes.clear();
                return;
            }

            mDirectory = std::filesystem::path(path).parent_path().string();

            ProcessNode(scene->mRootNode, scene);
        }
        catch (const std::exception& e)
        {
            XL_CORE_ERROR("Exception while importing model {0}: {1}", path, e.what());
            mMeshes.clear();
        }
        catch (...)
        {
            XL_CORE_ERROR("Unknown exception while importing model {0}", path);
            mMeshes.clear();
        }
    }

    void Model::ProcessNode(aiNode* node, const aiScene* scene)
    {
        for (uint32_t i = 0; i < node->mNumMeshes; ++i)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            mMeshes.push_back(ProcessMesh(mesh, scene));
        }

        for (uint32_t i = 0; i < node->mNumChildren; ++i)
        {
            ProcessNode(node->mChildren[i], scene);
        }
    }

    StaticMesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
        {
            Vertex vertex;

            // pos
            glm::vec3 vector;
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Pos = vector;

            //normal
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;

            //tangent (unused for now)
            vertex.Tangent = glm::vec3(0.0f);

            //tex coord
            if (mesh->mTextureCoords[0])
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoord = vec;
            }

            vertex.Color = glm::vec4(1.0f);
            vertex.EntityID = -1;

            vertices.push_back(vertex);
        }

        for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
        {
            aiFace face = mesh->mFaces[i];
            for (uint32_t j = 0; j < face.mNumIndices; ++j)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        return StaticMesh(vertices, indices);
    }
}
