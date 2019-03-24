#include "Pch.hpp"
#if 1
#include "CascadedShadowMappingRenderer.hpp"

using namespace DirectX;
using namespace dx;

CascadedShadowMappingRenderer::CascadedShadowMappingRenderer(
    ID3D11Device& device3D, const CascadedShadowMapConfig& shadowMapConfig)
    : m_config{shadowMapConfig},
	m_partitions{shadowMapConfig.Intervals}
{
    CreateDepthGenerationResources(device3D);
    CreateCsmGenerationResources(device3D, shadowMapConfig.ShadowMapSize);
    CreateSssmRt(device3D, shadowMapConfig.ScreenSpaceTexSize);
}

void CascadedShadowMappingRenderer::GenerateShadowMap(const dx::GlobalGraphicsContext& gfxContext,
                                                      const dx::Camera& camera,
                                                      gsl::span<const dx::Light> lights,
                                                      gsl::span<const RenderNode> renderNodes,
                                                      const dx::GlobalShaderContext& shaderContext)
{
    ID3D11DeviceContext& context3D = gfxContext.Context3D();

    // first pass: collect depth in view space
	// 为了从 screen space 还原到 world space。
	std::array<ID3D11RenderTargetView* const, 1> nullView = {};
    context3D.OMSetRenderTargets(1, nullView.data(), m_worldDepthView.Get());
    RunShadowCaster(renderNodes, shaderContext, context3D);
	BoundingBox boundingBox;
	BoundingBox::CreateFromPoints(boundingBox, g_XMNegInfinity, g_XMInfinity);
	for (const RenderNode& renderNode : renderNodes)
	{
		BoundingBox::CreateMerged(boundingBox, boundingBox, renderNode.mesh.GetBoundingBox());
	}

    // m_viewSpaceDepthMap contains what we want
    // second pass: CSM
    BoundingFrustum frustum = camera.Frustum();
    const dx::Light& mainLight = lights[0];
    BoundingFrustum lightSpaceFrustum;
    XMMATRIX lightSpaceTransformation;
    switch (mainLight.index())
    {
    case dx::kDirectionalLight:
        auto& directionalLight = std::get<DirectionalLight>(mainLight);
        lightSpaceTransformation = XMMatrixLookToLH(
            XMVectorZero(), Load(directionalLight.Direction), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
    }
    frustum.Transform(lightSpaceFrustum,
                      XMMatrixInverse({}, camera.GetView()) * lightSpaceTransformation);

    const std::uint32_t partitionCount = kCascadedCount;
    dx::GlobalShaderContext shaderContextForShadowMapping = shaderContext;
    shaderContextForShadowMapping.ViewMatrix = lightSpaceTransformation;
	const float nearZ = boundingBox.Center.z - boundingBox.Extents.z;
	const float farZ = boundingBox.Center.z + boundingBox.Extents.z;
	const float nearFarDistance = farZ - nearZ;

    for (std::uint32_t i = 0; i < partitionCount; ++i)
    {
        const float low = nearZ + m_partitions[i] * nearFarDistance;
        const float high = nearZ + m_partitions[i + 1] * nearFarDistance;
        XMMATRIX projMatrix = CalcLightProjMatrix(mainLight, lightSpaceFrustum, low, high);
        shaderContextForShadowMapping.ProjMatrix = projMatrix;
        shaderContextForShadowMapping.ViewProjMatrix =
            shaderContextForShadowMapping.ViewMatrix * shaderContextForShadowMapping.ProjMatrix;
        m_lightViewProjs[i] = lightSpaceTransformation * projMatrix;
        ID3D11RenderTargetView* rt = m_shadowMapRtViews[i].Get();
        context3D.OMSetRenderTargets(1, &rt, m_shadowMapRtDepthStencil.View());
		std::array<float, 4> color = {};
		context3D.ClearRenderTargetView(rt, color.data());
		m_shadowMapRtDepthStencil.ClearBoth(context3D);
        ShaderInputs inputs;
        for (const RenderNode& renderNode : renderNodes)
        {
			//FIXME：如何避免上一个对象设置的 buffer 遗留的问题？
			FillUpShaders(context3D, renderNode.material.shadowCasterPass, renderNode.World, nullptr, shaderContextForShadowMapping);
			DrawMesh(context3D, renderNode.mesh, *renderNode.material.shadowCasterPass.pass);
        }
    }

    // m_depthTexArray has been filled.

    // last pass: generate screen space shadowmap

    //auto invViewProj = XMMatrixInverse({}, XMMatrixTranspose(shaderContext.ViewProjMatrix));

    //dx::ShaderInputs additionalInputs;
    //// viewspace
    //additionalInputs.SetField("InvViewProj", invViewProj);
    //additionalInputs.SetField("LightViewProjs", m_lightViewProjs);
    //additionalInputs.Bind("DepthTex", m_shadowMapTexArraySrv);
    //additionalInputs.Bind("DepthTexSampler", m_nearestPointSampler);
    //DrawMesh(context3D, *m_quad, m_collectPass);
}

wrl::ComPtr<ID3D11ShaderResourceView> CascadedShadowMappingRenderer::GetCsmTexArray() const
{
	return m_shadowMapTexArraySrv;
}

void CascadedShadowMappingRenderer::RunShadowCaster(
    gsl::span<const dx::RenderNode>& renderNodes,
    const dx::GlobalShaderContext& shaderContextForShadowMapping, ID3D11DeviceContext& context3D)
{
    for (const RenderNode& renderNode : renderNodes)
    {
		const Material& material = renderNode.material;
		if (material.shadowCasterPass.pass)
		{
			FillUpShaders(context3D, material.shadowCasterPass, renderNode.World, nullptr, shaderContextForShadowMapping);
			DrawMesh(context3D, renderNode.mesh, *material.shadowCasterPass.pass);
		}
    }
}

DirectX::XMMATRIX CascadedShadowMappingRenderer::CalcLightProjMatrix(
    const dx::Light& light, DirectX::BoundingFrustum& existingFrustum, float low, float high)
{
    switch (light.index())
    {
    case kDirectionalLight:
    {
        existingFrustum.Near = low;
        existingFrustum.Far = high;
        XMFLOAT3 corners[8];
        existingFrustum.GetCorners(corners);
        BoundingBox aabb;
        BoundingBox::CreateFromPoints(aabb, std::size(corners), corners, sizeof(XMFLOAT3));
		aabb.GetCorners(corners);
		const XMFLOAT3 maxCorner = corners[2];
		const XMFLOAT3 minCorner = corners[4];
        return XMMatrixOrthographicLH(2.0f * aabb.Extents.x, 2.0f* aabb.Extents.y, minCorner.z, maxCorner.z);
    }
    default:
        break;
    }
}

void CascadedShadowMappingRenderer::CreateCsmGenerationResources(ID3D11Device& device3D,
                                                                 dx::Size size)
{
    D3D11_TEXTURE2D_DESC depthTexArrayDesc{};
    depthTexArrayDesc.ArraySize = kCascadedCount;
    depthTexArrayDesc.Width = size.Width;
    depthTexArrayDesc.Height = size.Height;
    depthTexArrayDesc.Usage = D3D11_USAGE_DEFAULT;
    depthTexArrayDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    depthTexArrayDesc.Format = DXGI_FORMAT_R32_FLOAT;
    depthTexArrayDesc.MipLevels = 1;
	depthTexArrayDesc.SampleDesc.Count = 1;
    TryHR(device3D.CreateTexture2D(&depthTexArrayDesc, nullptr, m_depthTexArray.GetAddressOf()));

    D3D11_SHADER_RESOURCE_VIEW_DESC rsvDesc{};
    rsvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    rsvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2DARRAY;
    D3D11_TEX2D_ARRAY_SRV& tex2DArray = rsvDesc.Texture2DArray;
    tex2DArray.ArraySize = kCascadedCount;
    tex2DArray.MipLevels = 1;
    tex2DArray.MostDetailedMip = 0;
    tex2DArray.FirstArraySlice = 0;
    TryHR(device3D.CreateShaderResourceView(m_depthTexArray.Get(), &rsvDesc,
                                            m_shadowMapTexArraySrv.GetAddressOf()));
	D3D11_RENDER_TARGET_VIEW_DESC rtViewDesc{};
	rtViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	rtViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	D3D11_TEX2D_ARRAY_RTV& rtTexArray = rtViewDesc.Texture2DArray;
	rtTexArray.MipSlice = 0;
	rtTexArray.ArraySize = 1;
	for (unsigned i = 0; i < kCascadedCount; ++i)
	{
		rtViewDesc.Texture2DArray.FirstArraySlice = D3D11CalcSubresource(0, i, 1);
		TryHR(device3D.CreateRenderTargetView(m_depthTexArray.Get(), &rtViewDesc, m_shadowMapRtViews[i].GetAddressOf()));
	}
	m_shadowMapRtDepthStencil = DepthStencil{
		device3D, size
	};
}

void CascadedShadowMappingRenderer::CreateSssmRt(ID3D11Device& device3D, dx::Size size)
{
    D3D11_TEXTURE2D_DESC texDesc{};
    texDesc.Width = size.Width;
    texDesc.Height = size.Height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	texDesc.SampleDesc.Count = 1;
    TryHR(device3D.CreateTexture2D(&texDesc, nullptr, m_screenSpaceShadowMap.GetAddressOf()));
    D3D11_RENDER_TARGET_VIEW_DESC rtDesc{};
    rtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    TryHR(device3D.CreateRenderTargetView(m_screenSpaceShadowMap.Get(), &rtDesc,
                                          m_sssmRt.GetAddressOf()));
   /* const ShortIndex quadIndices[] = {0, 1, 2, 1, 2, 3};
    const PositionType quadPositions[] = {
        MakePosition(-1.0f, 1.0f, 1.0f),
        MakePosition(1.0f, 1.0f, 1.0f),
        MakePosition(-1.0f, -1.0f, 1.0f),
        MakePosition(-1.0f, 1.0f, 1.0f),
    };
    const TexCoordType quadTexCoords[] = {MakeTexCoord(0.0f, 0.0f), MakeTexCoord(1.0f, 0.0f),
                                          MakeTexCoord(0.0f, 1.0f), MakeTexCoord(1.0f, 1.0f)};
    m_quad = Mesh::CreateImmutable(device3D, InputLayoutAllocator::Query(dx::PosTexDesc),
                                   gsl::span(quadIndices), gsl::span(quadPositions),
                                   gsl::span(quadTexCoords));*/
}

void CascadedShadowMappingRenderer::CreateDepthGenerationResources(ID3D11Device& device3D)
{
    // first pass: collect depth
    CD3D11_TEXTURE2D_DESC depthTexDesc{DXGI_FORMAT_R32G8X24_TYPELESS,
                                       m_config.ScreenSpaceTexSize.Width,
                                       m_config.ScreenSpaceTexSize.Height};
    depthTexDesc.BindFlags |= D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;
    TryHR(device3D.CreateTexture2D(&depthTexDesc, nullptr, m_worldSpaceDepthMap.GetAddressOf()));
    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{D3D11_DSV_DIMENSION_TEXTURE2D,
                                                        DXGI_FORMAT_D32_FLOAT_S8X24_UINT};
    TryHR(device3D.CreateDepthStencilView(m_worldSpaceDepthMap.Get(), &depthStencilViewDesc,
                                          m_worldDepthView.GetAddressOf()));
    CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{D3D11_SRV_DIMENSION_TEXTURE2D,
                                             DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS};
    TryHR(device3D.CreateShaderResourceView(m_worldSpaceDepthMap.Get(), &srvDesc,
                                            m_depthSrv.GetAddressOf()));
}

dx::Pass CascadedShadowMappingRenderer::MakeCollectionPass(ID3D11Device& device3D)
{
	throw std::runtime_error{"fuck"};
    /*using namespace dx;
    return Pass{ShaderCollection{
        Shader::FromCompiledCso(device3D, fs::current_path() / "SecondPassVS.hlsl"),
        Shader::FromCompiledCso(device3D, fs::current_path() / "SecondPassPS.hlsl")}};*/
}
#endif