// Copyright(c) 2019 - 2020, #Momo
// All rights reserved.
// 
// Redistributionand use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
// 
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditionsand the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditionsand the following disclaimer in the documentation
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

#pragma once

#include "Library/Primitives/AbstractPrimitive.h"
#include "Utilities/Array/Array2D.h"
#include "Core/Application/Application.h"

namespace MxEngine
{
    class Surface : public AbstractPrimitive
    {
        Array2D<Vector3> surface;
        std::string name;
    public:
        Surface()
        {
            name = Format(FMT_STRING("MxSurface_{0}"), Application::Get()->GenerateResourceId());
        }

        template<typename Func>
        void SetSurface(Func&& f, float xsize, float ysize, float step)
        {
            static_assert(std::is_same<float, decltype(f(float(), float()))>::value);
            MX_ASSERT(step  > 0.0f);
            MX_ASSERT(xsize > 0.0f);
            MX_ASSERT(ysize > 0.0f);
            size_t intxsize = static_cast<size_t>(xsize / step);
            size_t intysize = static_cast<size_t>(ysize / step);
            surface.resize(intxsize, intysize);
            
            for (size_t x = 0; x < intxsize; x++)
            {
                for (size_t y = 0; y < intysize; y++)
                {
                    float fx = x * step;
                    float fy = y * step;
                    float fz = f(fx, fy);
                    surface[x][y] = MakeVector3(fx, fz, fy);
                }
            }
            this->Apply();
        }

        void SetSurface(ArrayView<Vector3> array, size_t xsize, size_t ysize)
        {
            surface.resize(xsize, ysize);
            for (size_t x = 0; x < xsize; x++)
            {
                for (size_t y = 0; y < ysize; y++)
                {
                    surface[x][y] = array[x * ysize + y];
                }
            }
            this->Apply();
        }

        void Apply()
        {
            std::vector<float> buffer;
            Array2D<Vector3> pointNormals;
            size_t xsize = surface.width();
            size_t ysize = surface.height();
            size_t triangleCount = 2 * (xsize - 1) * (ysize - 1);
            buffer.reserve(2 * triangleCount * VertexSize);
            pointNormals.resize(xsize, ysize, MakeVector3(0.0f));

            struct Vertex
            {
                Vector3 position;
                Vector2 texture;
                Vector3 normal;
            };
            using Triangle = std::array<Vertex, 3>;

            std::vector<Triangle> triangles;
            triangles.reserve(triangleCount);

            for (size_t x = 0; x < xsize - 1; x++)
            {
                for (size_t y = 0; y < ysize - 1; y++)
                {
                    triangles.emplace_back();
                    Triangle& up = triangles.back();
                    triangles.emplace_back();
                    Triangle& down = triangles.back();
                    up[0].position   = surface[x + 0][y + 0];
                    up[1].position   = surface[x + 0][y + 1];
                    up[2].position   = surface[x + 1][y + 0];
                    down[0].position = surface[x + 1][y + 1];
                    down[1].position = surface[x + 1][y + 0];
                    down[2].position = surface[x + 0][y + 1];

                    up[0].texture   = MakeVector2(0.0f, 0.0f);
                    up[1].texture   = MakeVector2(0.0f, 1.0f);
                    up[2].texture   = MakeVector2(1.0f, 0.0f);
                    down[0].texture = MakeVector2(1.0f, 1.0f);
                    down[1].texture = MakeVector2(1.0f, 0.0f);
                    down[2].texture = MakeVector2(0.0f, 1.0f);

                    Vector3 v1, v2, n;

                    v1 = up[1].position - up[0].position;
                    v2 = up[2].position - up[0].position;
                    n =  Normalize(Cross(v1, v2));

                    pointNormals[x + 0][y + 0] += n;
                    pointNormals[x + 1][y + 0] += n;
                    pointNormals[x + 0][y + 1] += n;

                    v1 = down[1].position - down[0].position;
                    v2 = down[2].position - down[0].position;
                    n =  Normalize(Cross(v1, v2));

                    pointNormals[x + 1][y + 1] += n;
                    pointNormals[x + 0][y + 1] += n;
                    pointNormals[x + 1][y + 0] += n;
                }
            }

            for (size_t x = 0; x < xsize - 1; x++)
            {
                for (size_t y = 0; y < ysize - 1; y++)
                {
                    auto normalize = [](auto& p)
                    {
                        return p / 6.0f;
                    };

                    auto& up       = triangles[2 * (x * (ysize - 1) + y) + 0];
                    auto& down     = triangles[2 * (x * (ysize - 1) + y) + 1];
                    up[0].normal   = pointNormals[x + 0][y + 0];
                    up[1].normal   = pointNormals[x + 0][y + 1];
                    up[2].normal   = pointNormals[x + 1][y + 0];
                    down[0].normal = pointNormals[x + 1][y + 1];
                    down[1].normal = pointNormals[x + 1][y + 0];
                    down[2].normal = pointNormals[x + 0][y + 1];
                }
            }

            for (const auto& triangle : triangles)
            {
                for (const auto& vertex : triangle)
                {
                    buffer.push_back(vertex.position.x);
                    buffer.push_back(vertex.position.y);
                    buffer.push_back(vertex.position.z);
                    buffer.push_back(vertex.texture.x);
                    buffer.push_back(vertex.texture.y);
                    buffer.push_back(vertex.normal.x);
                    buffer.push_back(vertex.normal.y);
                    buffer.push_back(vertex.normal.z);
                }
            }

            this->SubmitData(name, buffer);
        }
    };
}