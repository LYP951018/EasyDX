#pragma once

auto GetDevice() -> std::tuple<ID3D11Device&, ID3D11DeviceContext&>;