#include "RenderedObject.hpp"
#include "SimpleVertex.hpp"
#include <utility>
#include <d3d11.h>

namespace dx
{
    RenderedObject::RenderedObject(gsl::span<Mesh> meshes)
        : meshes_{meshes.begin(), meshes.end()}
    {
    }

    DirectX::XMMATRIX RenderedObject::ComputeWorld() const noexcept
    {
        using namespace DirectX;
        return 
            XMMatrixScaling(Scale.x, Scale.y, Scale.z) *
            XMMatrixRotationQuaternion(XMLoadFloat4(&Rotation)) *
            XMMatrixTranslation(Translation.x, Translation.y, Translation.z);
    }

    void RenderedObject::Render(ID3D11DeviceContext& deviceContext)
    {
        for (auto& mesh : meshes_)
        {
            mesh.Render(deviceContext);
        }
    }

   /* RenderedObject MakeCube(ID3D11Device & device)
    {
        SimpleVertex vertices[] = {
            { {-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f} },
            { {1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f} },
            { {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f} },
            { {-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f} },

            { {-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f} },
            { {1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f} },
            { {1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f} },
            { {-1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f} },

            { {-1.0f, -1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} },
            { {-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f} },
            { {-1.0f, 1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f} },
            { {-1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} },

            { {1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f} },
            { {1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f} },
            { {1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f} },
            { {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f} },

            { {-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f} },
            { {1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f} },
            { {1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f} },
            { {-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f} },

            { {-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} },
            { {1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} },
            { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} },
            { {-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} },
        };
    }*/
}