#include "Pch.hpp"
#include "CommonDevices.hpp"

std::tuple<ID3D11Device&, ID3D11DeviceContext&> GetDevice()
{
    static dx::DeviceContext3DPair device = dx::MakeDevice3D();
    return {dx::Ref(device.first), dx::Ref(device.second)};
}