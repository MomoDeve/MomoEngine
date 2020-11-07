// Copyright(c) 2019 - 2020, #Momo
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
// 
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and /or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "ResourceEditor.h"
#include "ComponentEditor.h"
#include "Utilities/ImGui/ImGuiUtils.h"
#include "Core/BoundingObjects/Cylinder.h"
#include "Core/BoundingObjects/Capsule.h"
#include "Core/BoundingObjects/BoundingBox.h"
#include "Utilities/Image/ImageManager.h"
#include "Library/Primitives/Colors.h"
#include "Library/Primitives/Primitives.h"
#include "Utilities/FileSystem/FileManager.h"

namespace MxEngine::GUI
{
    void DrawTextureList(const char* name, bool* isOpen)
    {
        ImGui::Begin(name, isOpen);

        static char filter[128] = { '\0' };
        ImGui::InputText("search filter", filter, std::size(filter));

        if (ImGui::CollapsingHeader("create new texture"))
        {
            if (ImGui::Button("load from file"))
            {
                MxString path = FileManager::OpenFileDialog("*.png *.jpg *.jpeg *.bmp *.tga *.hdr", "Image Files");
                if (!path.empty() && File::Exists(path))
                {
                    auto newTexture = AssetManager::LoadTexture(path, TextureFormat::RGBA);
                    newTexture.MakeStatic();
                }
            }

            static Vector3 color{ 0.0f };
            ImGui::ColorEdit3("", &color[0]);
            ImGui::SameLine();
            if (ImGui::Button("create texture"))
            {
                auto colorTexture = Colors::MakeTexture(color);
                colorTexture->SetPath("color.runtime");
                colorTexture.MakeStatic();
            }
        }

        ImGui::SetCursorPosX(0.5f * ImGui::GetWindowWidth() - 20.0f);
        ImGui::Text("%s", "texture list");

        auto& factory = GraphicFactory::Get<Texture>();
        for (auto& object : factory)
        {
            auto id = (int)factory.IndexOf(object);
            ImGui::PushID(id);

            auto texture = GraphicFactory::GetHandle(object);
            auto& texturePath = texture->GetPath();

            if (filter[0] == '\0' || texturePath.find(filter) != texturePath.npos)
            {
                ImGui::AlignTextToFramePadding();
                ImGui::Text("id: %-3d", id);
                ImGui::SameLine();
                DrawTextureEditor(texturePath.c_str(), texture, false);
            }

            ImGui::PopID();
        }

        ImGui::End();
    }

    void LoadFromInputId(TextureHandle& texture, int id)
    {
        auto handle = (size_t)Max(id, 0);
        auto& storage = GraphicFactory::Get<Texture>();
        if (storage.IsAllocated(handle))
        {
            texture = GraphicFactory::GetHandle(storage[handle]);
        }
    }

    bool IsInternalEngineTexture(const TextureHandle& tex)
    {
        auto& path = tex->GetPath();
        return path.find("[[") != path.npos && path.find("]]") != path.npos;
    }

    void DrawTextureEditor(const char* name, TextureHandle& texture, bool withTextureLoader)
    {
        SCOPE_TREE_NODE(name);
        ImGui::PushID((int)texture.GetHandle());

        if (texture.IsValid())
        {
            if (!IsInternalEngineTexture(texture) && ImGui::Button("delete"))
            {
                ImGui::PopID();
                GraphicFactory::Destroy(texture);
                return;
            }

            ImGui::Text("path: %s", texture->GetPath().c_str());
            ImGui::Text("width: %d", (int)texture->GetWidth());
            ImGui::Text("height: %d", (int)texture->GetHeight());
            ImGui::Text("channels: %d", (int)texture->GetChannelCount());
            ImGui::Text("samples: %d", (int)texture->GetSampleCount());
            ImGui::Text("format: %s", EnumToString(texture->GetFormat())); //-V111
            ImGui::Text("wrap type: %s", EnumToString(texture->GetWrapType())); //-V111
            ImGui::Text("native render type: %d", (int)texture->GetTextureType());
            ImGui::Text("native handle: %d", (int)texture->GetNativeHandle());
            ImGui::Text("is depth-only: %s", BOOL_STRING(texture->IsDepthOnly()));
            ImGui::Text("is multisampled: %s", BOOL_STRING(texture->IsMultisampled()));
        }
        else
        {
            ImGui::Text("empty resource");
        }

        if (withTextureLoader)
        {
            if (ImGui::Button("load from file"))
            {
                MxString path = FileManager::OpenFileDialog("*.png *.jpg *.jpeg *.bmp *.tga *.hdr", "Image Files");
                if (!path.empty() && File::Exists(path))
                {
                    texture = AssetManager::LoadTexture(path, TextureFormat::RGBA);
                }
            }
            
            static int id = 0;
            ImGui::SameLine();
            if(GUI::InputIntOnClick("", &id, "load from id"))
                LoadFromInputId(texture, id);
        }
        
        if (texture.IsValid())
        {   
            DrawImageSaver(texture);

            ImGui::SameLine();
            if (ImGui::Button("flip image")) {
                auto image = texture->GetRawTextureData();
                ImageManager::FlipImage(image);
                texture->Load(image, texture->GetFormat(), texture->GetWrapType());
            }

            auto nativeHeight = texture->GetHeight();
            auto nativeWidth = texture->GetWidth();

            static Vector3 color{ 1.0f };
            if (ImGui::ColorEdit3("border color", &color[0]))
                texture->SetBorderColor(color);

            static float scale = 1.0f;
            ImGui::DragFloat("texture preview scale", &scale, 0.01f, 0.0f, 1.0f);
            auto width = ImGui::GetWindowSize().x * 0.9f * scale;
            auto height = width * nativeHeight / nativeWidth;
            if (!texture->IsMultisampled()) // TODO: support multisampled textures
            {
                texture->GenerateMipmaps(); // without mipmaps texture can be not visible in editor if its size is too small
                ImGui::Image((void*)(uintptr_t)texture->GetNativeHandle(), ImVec2(width, height), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
            }
        }
        ImGui::PopID();
    }

    void DrawCubeMapEditor(const char* name, CubeMapHandle& cubemap)
    {
        SCOPE_TREE_NODE(name);
        ImGui::PushID((int)cubemap.GetHandle());

        if (cubemap.IsValid())
        {
            ImGui::Text("path: %s", cubemap->GetPath().c_str());
            ImGui::Text("width: %d", (int)cubemap->GetWidth());
            ImGui::Text("height: %d", (int)cubemap->GetHeight());
            ImGui::Text("channels: %d", (int)cubemap->GetChannelCount());
            ImGui::Text("native handle: %d", (int)cubemap->GetNativeHandle());
        }
        else
        {
            ImGui::Text("empty resource");
        }
        
        if (ImGui::Button("load from file"))
        {
            MxString path = FileManager::OpenFileDialog("*.png *.jpg *.jpeg *.bmp *.tga *.hdr", "Image Files");
            if (!path.empty() && File::Exists(path))
            {
                cubemap = AssetManager::LoadCubeMap(path);
            }
        }
        // TODO: support cubemap preview
        ImGui::PopID();
    }

    void DrawMaterialEditor(MaterialHandle& material)
    {
        if (!material.IsValid())
        {
            GUI::Indent _(5.0f);
            ImGui::Text("empty material");
            return;
        }
        SCOPE_TREE_NODE(material->Name.c_str());

        DrawTextureEditor("albedo map", material->AlbedoMap, true);
        DrawTextureEditor("roughness map", material->RoughnessMap, true);
        DrawTextureEditor("metallic map", material->MetallicMap, true);
        DrawTextureEditor("emmisive map", material->EmmisiveMap, true);
        DrawTextureEditor("normal map", material->NormalMap, true);
        DrawTextureEditor("height map", material->HeightMap, true);
        DrawTextureEditor("ambient occlusion map", material->AmbientOcclusionMap, true);

        ImGui::Checkbox("casts shadows", &material->CastsShadow);
        ImGui::DragFloat("roughness factor", &material->RoughnessFactor, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("metallic factor", &material->MetallicFactor, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("emmision", &material->Emmision, 0.01f, 0.0f, FLT_MAX);
        ImGui::DragFloat("displacement", &material->Displacement, 0.01f);
        ImGui::DragFloat("transparency", &material->Transparency, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat2("UV multipliers", &material->UVMultipliers[0], 0.01f);
        ImGui::ColorEdit3("base color", &material->BaseColor[0], ImGuiColorEditFlags_HDR);
    }

    void DrawAABBEditor(const char* name, AABB& aabb)
    {
        SCOPE_TREE_NODE(name);
        ImGui::DragFloat3("min", &aabb.Min[0], 0.01f);
        ImGui::DragFloat3("max", &aabb.Max[0], 0.01f);
        aabb.Min = VectorMin(aabb.Min, aabb.Max);
        aabb.Max = VectorMax(aabb.Min, aabb.Max);
    }

    void DrawBoxEditor(const char* name, BoundingBox& box)
    {
        SCOPE_TREE_NODE(name);

        ImGui::DragFloat3("center", &box.Center[0]);
        ImGui::DragFloat3("min", &box.Min[0], 0.01f);
        ImGui::DragFloat3("max", &box.Max[0], 0.01f);
        
        auto rotation = DegreesVec(MakeEulerAngles(box.Rotation));
        auto newRotation = rotation;
        if (ImGui::DragFloat("rotate x", &newRotation.x))
            box.Rotation *= MakeQuaternion(Radians(newRotation.x - rotation.x), MakeVector3(1.0f, 0.0f, 0.0f));
        if (ImGui::DragFloat("rotate y", &newRotation.y))
            box.Rotation *= MakeQuaternion(Radians(newRotation.y - rotation.y), MakeVector3(0.0f, 1.0f, 0.0f));
        if (ImGui::DragFloat("rotate z", &newRotation.z))
            box.Rotation *= MakeQuaternion(Radians(newRotation.z - rotation.z), MakeVector3(0.0f, 0.0f, 1.0f));

        box.Min = VectorMin(box.Min, box.Max);
        box.Max = VectorMax(box.Min, box.Max);
    }

    void DrawSphereEditor(const char* name, BoundingSphere& sphere)
    {
        SCOPE_TREE_NODE(name);
        ImGui::DragFloat3("center", &sphere.Center[0], 0.01f);
        ImGui::DragFloat("radius", &sphere.Radius, 0.01f);
        sphere.Radius = Max(sphere.Radius, 0.0f);
    }

    void DrawCylinderEditor(const char* name, Cylinder& cylinder)
    {
        SCOPE_TREE_NODE(name);
        ImGui::DragFloat("height",   &cylinder.Height, 0.01f);
        ImGui::DragFloat("x radius", &cylinder.RadiusX, 0.01f);
        ImGui::DragFloat("z radius", &cylinder.RadiusZ, 0.01f);
        cylinder.Height  = Max(cylinder.Height, 0.0f);
        cylinder.RadiusX = Max(cylinder.RadiusX, 0.0f);
        cylinder.RadiusZ = Max(cylinder.RadiusZ, 0.0f);

        bool axisX = cylinder.Orientation == Cylinder::Axis::X;
        bool axisY = cylinder.Orientation == Cylinder::Axis::Y;
        bool axisZ = cylinder.Orientation == Cylinder::Axis::Z;

        if (ImGui::BeginCombo("orientation", axisX ? "x axis" : (axisY ? "y axis" : "z axis")))
        {
            if (ImGui::Selectable("x axis", &axisX))
            {
                cylinder.Orientation = Cylinder::Axis::X;
                ImGui::SetItemDefaultFocus();
            }
            if (ImGui::Selectable("y axis", &axisY))
            {
                cylinder.Orientation = Cylinder::Axis::Y;
                ImGui::SetItemDefaultFocus();
            }
            if (ImGui::Selectable("z axis", &axisZ))
            {
                cylinder.Orientation = Cylinder::Axis::Z;
                ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    void DrawCapsuleEditor(const char* name, Capsule& capsule)
    {
        SCOPE_TREE_NODE(name);
        ImGui::DragFloat("height", &capsule.Height, 0.01f);
        ImGui::DragFloat("radius", &capsule.Radius, 0.01f);
        capsule.Height = Max(capsule.Height, 0.0f);
        capsule.Radius = Max(capsule.Radius, 0.0f);

        bool axisX = capsule.Orientation == Capsule::Axis::X;
        bool axisY = capsule.Orientation == Capsule::Axis::Y;
        bool axisZ = capsule.Orientation == Capsule::Axis::Z;

        if (ImGui::BeginCombo("orientation", axisX ? "x axis" : (axisY ? "y axis" : "z axis")))
        {
            if (ImGui::Selectable("x axis", &axisX))
            {
                capsule.Orientation = Capsule::Axis::X;
                ImGui::SetItemDefaultFocus();
            }
            if (ImGui::Selectable("y axis", &axisY))
            {
                capsule.Orientation = Capsule::Axis::Y;
                ImGui::SetItemDefaultFocus();
            }
            if (ImGui::Selectable("z axis", &axisZ))
            {
                capsule.Orientation = Capsule::Axis::Z;
                ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    void DrawLightBaseEditor(LightBase& base)
    {
        float ambientIntensity = base.GetAmbientIntensity();
        float intensity = base.GetIntensity();
        Vector3 color = base.GetColor();

        ImGui::ColorEdit3("color", &color[0]);
        ImGui::DragFloat("intensity", &intensity, 0.1f);
        ImGui::DragFloat("ambient intensity", &ambientIntensity, 0.01f);

        base.SetAmbientIntensity(ambientIntensity);
        base.SetColor(color);
        base.SetIntensity(intensity);
    }

    void DrawVertexEditor(Vertex& vertex)
    {
        ImGui::InputFloat3("position", &vertex.Position[0]);
        ImGui::InputFloat2("texture", &vertex.TexCoord[0]);
        ImGui::InputFloat3("normal", &vertex.Normal[0]);
        ImGui::InputFloat3("tangent", &vertex.Tangent[0]);
        ImGui::InputFloat3("bitangent", &vertex.Bitangent[0]);
    }

    void DrawImageSaver(const TextureHandle& texture, const char* name)
    {
        static MxString path(128, '\0');
        ImGui::InputText("save path", path.data(), path.size());

        const char* imageTypes[] = { "PNG", "BMP", "TGA", "JPG", "HDR", };
        static int currentImageType = 0;
        if (ImGui::Button("save image to disk"))
            ImageManager::SaveTexture(path.c_str(), texture, (ImageType)currentImageType);
        ImGui::SameLine();
        ImGui::PushItemWidth(50.0f);
        ImGui::Combo(" ", &currentImageType, imageTypes, (int)std::size(imageTypes));
        ImGui::PopItemWidth();
    }

    void LoadFromPrimitive(MeshHandle& mesh)
    {
        bool optionPeeked = false;
        static int subdivisions = 1;
        ImGui::InputInt("subdivisions", &subdivisions);
        subdivisions = Max(subdivisions, 1);

        if (ImGui::BeginCombo("load primitive", "click to select"))
        {
            if (ImGui::Selectable("cube", &optionPeeked))
            {
                mesh = Primitives::CreateCube(subdivisions);
                ImGui::SetItemDefaultFocus();
            }
            if (ImGui::Selectable("sphere", &optionPeeked))
            {
                mesh = Primitives::CreateSphere(subdivisions);
                ImGui::SetItemDefaultFocus();
            }
            if (ImGui::Selectable("plane", &optionPeeked))
            {
                mesh = Primitives::CreatePlane(subdivisions);
                ImGui::SetItemDefaultFocus();
            }
            if (ImGui::Selectable("cylinder", &optionPeeked))
            {
                mesh = Primitives::CreateCylinder(subdivisions);
                ImGui::SetItemDefaultFocus();
            }
            if (ImGui::Selectable("pyramid", &optionPeeked))
            {
                mesh = Primitives::CreatePyramid();
                ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (optionPeeked) subdivisions = 1; // reset parameter
    }

    void DrawMeshEditor(const char* name, MeshHandle& mesh)
    {
        SCOPE_TREE_NODE(name);
        ImGui::PushID((int)mesh.GetHandle());

        DrawAABBEditor("bounding box", mesh->BoxBounding);
        DrawSphereEditor("bounding sphere", mesh->SphereBounding);

        if (ImGui::Button("load from file"))
        {
            MxString path = FileManager::OpenFileDialog();
            if (!path.empty() && File::Exists(path))
                mesh = AssetManager::LoadMesh(path);
        }

        ImGui::SameLine();
        if (ImGui::Button("update mesh boundings"))
            mesh->UpdateBoundingGeometry();
        
        ImGui::Indent(9.0f);
        LoadFromPrimitive(mesh);
        ImGui::Unindent(9.0f);

        static MxString submeshName;
        
        size_t vertexCount = 0, indexCount = 0;
        for (auto& submesh : mesh->Submeshes)
        {
            vertexCount += submesh.Data.GetVertecies().size();
            indexCount += submesh.Data.GetIndicies().size();
        }
        ImGui::Text("total vertex count: %d, total index count: %d", (int)vertexCount, (int)indexCount);

        for (int id = 0; id < (int)mesh->Submeshes.size(); id++)
        {
            auto& submesh = mesh->Submeshes[id];
            if (!ImGui::CollapsingHeader(submesh.Name.c_str())) continue;
            GUI::Indent _(5.0f);
            ImGui::PushID(id);

            ImGui::Text("vertex count: %d", (int)submesh.Data.GetVertecies().size());
            ImGui::Text("index count: %d", (int)submesh.Data.GetIndicies().size());
            ImGui::Text("material id: %d", (int)submesh.GetMaterialId());

            TransformEditor(*submesh.GetTransform());

            if (ImGui::Button("update submesh boundings"))
                submesh.Data.UpdateBoundingGeometry();
            ImGui::SameLine();
            if (ImGui::Button("buffer vertecies"))
                submesh.Data.BufferVertecies();
            ImGui::SameLine();
            if (ImGui::Button("delete submesh"))
            {
                mesh->Submeshes.erase(mesh->Submeshes.begin() + id);
                id--; // compensate erased mesh
                ImGui::PopID();
                continue; // skip other ui as current mesh is deleted
            }

            if (ImGui::Button("regenerate normals"))
                submesh.Data.RegenerateNormals();
            ImGui::SameLine();
            if (ImGui::Button("regenerate tangents"))
                submesh.Data.RegenerateTangentSpace();

            // TODO: maybe add indicies editor?
            {
                if (ImGui::CollapsingHeader("vertecies"))
                {
                    GUI::Indent _(5.0f);
                    int id = 0;
                    for (auto& vertex : submesh.Data.GetVertecies())
                    {
                        ImGui::PushID(id++);
                        DrawVertexEditor(vertex);
                        ImGui::Separator();
                        ImGui::PopID();
                    }
                }
            }
            ImGui::PopID();
        }
        ImGui::PopID();
    }
}